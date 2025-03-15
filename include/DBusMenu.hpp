#pragma once

#include <dbusmenu-interface_proxy.h>
#include <gtkmm-4.0/gtkmm.h>

#include <Util/GlibUtil.hpp>
namespace libdbusmenu {

class Menu : public Gio::Menu {
    MAKE_PROPERTY(Glib::RefPtr<Gio::SimpleActionGroup>, actionGroup);
    MAKE_PROPERTY(std::string, actionGroupPrefix);

private:
    Glib::RefPtr<com::canonical::dbusmenuProxy> _proxy;
    Menu(Gio::DBus::BusType busType, std::string busName, std::string objectPath);

    Glib::RefPtr<Gio::MenuItem> makeItem(gint32 id, std::map<Glib::ustring, Glib::VariantBase> properties, Gio::SimpleActionGroup* actionGroup, std::string actionGroupPrefix);
    void makeMenu(Gio::Menu* model, Gio::SimpleActionGroup* actionGroup, std::tuple<gint32, std::map<Glib::ustring, Glib::VariantBase>, std::vector<Glib::VariantBase>> layout, bool debug = true, int blockLevel = 0);

    void reloadItems(guint32 revision = 0, gint32 parent = 0);
    void onProxyCreate(const Glib::RefPtr<Gio::AsyncResult>& res);

public:
    ~Menu();

    static Glib::RefPtr<Menu> create(std::string busName, std::string objectPath, Gio::DBus::BusType busType = Gio::DBus::BusType::SESSION);

    Glib::RefPtr<Gio::DBus::Proxy> dbusProxy();
};

};  // namespace libdbusmenu