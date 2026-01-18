# System apps (Settings and SystemInfo)
add_library(touchdown_apps STATIC
    settings_app.cpp
    system_info_app.cpp
)

target_link_libraries(touchdown_apps
    touchdown_app
    touchdown-core
    touchdown_shell
    lvgl
)
