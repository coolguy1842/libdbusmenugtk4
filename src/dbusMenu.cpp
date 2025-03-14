#include <fmt/format.h>

#include <DBusMenu.hpp>
#include <Util/GlibUtil.hpp>
#include <Util/StringUtil.hpp>

#include "giomm/asyncresult.h"
#include "giomm/menuitem.h"
#include "glibmm/refptr.h"

// assumes has label property
Glib::RefPtr<Gio::MenuItem> libdbusmenu::Menu::makeItem(gint32 id, std::map<Glib::ustring, Glib::VariantBase> properties, Gio::SimpleActionGroup* actionGroup, std::string actionGroupPrefix) {
    using libdbusmenu::Util::String::operator""_hash;

    std::string actionName         = fmt::format("activate_{}", id);
    std::string prefixedActionName = fmt::format("{}.{}", actionGroupPrefix, actionName);

    Glib::RefPtr<Gio::MenuItem> item = Gio::MenuItem::create(properties["label"].get_dynamic<std::string>(), actionName);
    Glib::RefPtr<Gio::SimpleAction> action;

    if(properties.contains("toggle-type")) {
        bool value = properties["toggle-state"].get_dynamic<int>();
        action     = Gio::SimpleAction::create_bool(actionName, value);

        action->signal_activate().connect([id, this](const Glib::VariantBase&) {
            _proxy->Event_sync(id, "clicked", Glib::Variant<std::string>::create(""), time(NULL));
        });

        actionGroup->add_action(action);

        switch(libdbusmenu::Util::String::hashFunc(properties["toggle-type"].get_dynamic<std::string>().c_str())) {
        case "checkmark"_hash: item->set_action(prefixedActionName); break;
        case "radio"_hash:     item->set_action_and_target(prefixedActionName, Glib::Variant<bool>::create(!value)); break;
        default:               break;
        }
    }
    else {
        action = Gio::SimpleAction::create(actionName);
        action->signal_activate().connect([action, id, this](const Glib::VariantBase&) {
            _proxy->Event_sync(id, "clicked", Glib::Variant<std::string>::create(""), time(NULL));
        });

        actionGroup->add_action(action);
        item->set_action(prefixedActionName);
    }

    if(properties.contains("enabled") && action != nullptr) {
        action->set_enabled(properties["enabled"].get_dynamic<bool>());
    }

    return item;
}

void libdbusmenu::Menu::makeMenu(Gio::Menu* model, Gio::SimpleActionGroup* actionGroup, std::tuple<gint32, std::map<Glib::ustring, Glib::VariantBase>, std::vector<Glib::VariantBase>> layout, bool debug, int blockLevel) {
    using libdbusmenu::Util::String::operator""_hash;
    const int indentAmount = 2;

    auto [id, properties, children] = layout;

    if(debug) {
        printf("%*sid: %d\n", blockLevel * indentAmount, "", id);
        printf("%*s%zu properties:\n", (blockLevel + 1) * indentAmount, "", properties.size());

        for(auto property : properties) {
            printf("%*sname: %s, valueType: %s, value: ", (blockLevel + 2) * indentAmount, "", property.first.c_str(), property.second.get_type_string().c_str());

            switch(Util::String::hashFunc(property.second.get_type_string().c_str())) {
            case "s"_hash: printf("'%s'", property.second.get_dynamic<std::string>().c_str()); break;
            case "b"_hash: printf("%s", property.second.get_dynamic<bool>() ? "true" : "false"); break;
            case "i"_hash: printf("%d", property.second.get_dynamic<gint32>()); break;
            default:       printf("Couldn't read value."); break;
            }

            printf("\n");
        }
    }

    std::string actionName         = fmt::format("activate_{}", id);
    std::string prefixedActionName = fmt::format("{}.{}", get_actionGroupPrefix(), actionName);
    if(properties.contains("label") && !(properties.contains("visible") && !properties["visible"].get_dynamic<bool>())) {
        if(properties.contains("children-display")) {
            switch(libdbusmenu::Util::String::hashFunc(properties["children-display"].get_dynamic<std::string>().c_str())) {
            case "submenu"_hash: {
                Glib::RefPtr<Gio::Menu> menu = Gio::Menu::create();

                model->append_submenu(properties["label"].get_dynamic<std::string>(), menu);
                model = menu.get();

                break;
            }
            default: break;
            }
        }
        else {
            model->append_item(makeItem(id, properties, actionGroup, get_actionGroupPrefix()));
        }
    }

    if(debug) printf("%*s%zu children\n", (blockLevel + 1) * indentAmount, "", children.size());
    for(auto child : children) {
        makeMenu(model, actionGroup, child.get_dynamic<std::tuple<gint32, std::map<Glib::ustring, Glib::VariantBase>, std::vector<Glib::VariantBase>>>(), debug, blockLevel + 2);
    }
}

void libdbusmenu::Menu::reloadItems() {
    _proxy->GetLayout(0, -1, {}, [&](const Glib::RefPtr<Gio::AsyncResult>& res) {
        guint32 revision;
        std::tuple<gint32, std::map<Glib::ustring, Glib::VariantBase>, std::vector<Glib::VariantBase>> layout;

        _proxy->GetLayout_finish(revision, layout, res);

        this->remove_all();
        for(Glib::ustring action : get_actionGroup()->list_actions()) {
            get_actionGroup()->remove_action(action);
        }

        makeMenu(this, get_actionGroup().get(), layout, false);
    });
}

static uint64_t actionID = 0;
libdbusmenu::Menu::~Menu() {}
libdbusmenu::Menu::Menu(Gio::DBus::BusType busType, std::string busName, std::string objectPath)
    : Glib::ObjectBase(typeid(libdbusmenu::Menu))
    , _property_actionGroup(*this, "actionGroup", Gio::SimpleActionGroup::create())
    , _property_actionGroupPrefix(*this, "actionGroupPrefix", fmt::format("{}dbus", actionID++)) {
    com::canonical::dbusmenuProxy::createForBus(
        busType,
        Gio::DBus::ProxyFlags::NONE,
        busName,
        objectPath,
        [&](const Glib::RefPtr<Gio::AsyncResult>& res) {
            _proxy = com::canonical::dbusmenuProxy::createForBusFinish(res);

            reloadItems();
            // TODO: make sure ItemsPropertiesUpdated_signal doesn't have to be used
            // make it only update the relevant child display
            _proxy->LayoutUpdated_signal.connect([&](guint32, gint32) { reloadItems(); });
        }
    );
}

Glib::RefPtr<libdbusmenu::Menu> libdbusmenu::Menu::create(std::string busName, std::string objectPath, Gio::DBus::BusType busType) {
    return Glib::make_refptr_for_instance(new libdbusmenu::Menu(busType, busName, objectPath));
}

Glib::RefPtr<Gio::DBus::Proxy> libdbusmenu::Menu::dbusProxy() { return _proxy->dbusProxy(); }