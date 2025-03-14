#pragma once

#define MAKE_PROPERTY(T, name)                                                        \
protected:                                                                            \
    Glib::Property<T> _property_##name;                                               \
                                                                                      \
public:                                                                               \
    Glib::PropertyProxy<T> property_##name() { return _property_##name.get_proxy(); } \
    T get_##name() const { return _property_##name.get_value(); }                     \
    T get_##name() { return _property_##name.get_value(); }

#define MAKE_SIGNAL_RET(name, returnType, ...)                        \
public:                                                               \
    using type_signal_##name = sigc::signal<returnType(__VA_ARGS__)>; \
    type_signal_##name signal_##name() { return _signal_##name; }     \
                                                                      \
protected:                                                            \
    type_signal_##name _signal_##name;

#define MAKE_SIGNAL(name, ...) MAKE_SIGNAL_RET(name, void, __VA_ARGS__)