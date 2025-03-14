project('libdbusmenugtk4', [ 'cpp', 'c' ], default_options : ['c_std=c23', 'cpp_std=c++23'])

fs = import('fs')

if(fs.exists('generated') != true)
    run_command(
        meson.current_source_dir() + '/gdbus-codegen-glibmm/gdbus-codegen-glibmm.py',
        '--generate-cpp-code=' + meson.current_source_dir() + '/generated/dbusmenu-interface',
        meson.current_source_dir() + '/data/introspect/com.canonical.dbusmenu.xml',
        check: true
    )
endif

# TODO: for now its an executable for testing, make library later
inc = include_directories('generated', 'include')

libdbusmenugtk4 = shared_library(
    'libdbusmenugtk4',
    sources: [
        files(
            'generated/dbusmenu-interface_common.cpp',
            'generated/dbusmenu-interface_proxy.cpp',
            'generated/dbusmenu-interface_stub.cpp'
        ),
        files(
            'src/Util/StringUtil.cpp',
            'src/dbusMenu.cpp'
        )
    ],
    include_directories : inc,
    install : false,
    dependencies: [
        dependency('fmt'),
        dependency('gtk4'),
        dependency('gtkmm-4.0')
    ]
)

libdbusmenugtk4_dep = declare_dependency(
    include_directories : inc,
    link_with : libdbusmenugtk4,
    dependencies: [
        dependency('fmt'),
        dependency('gtk4'),
        dependency('gtkmm-4.0')
    ]
)
