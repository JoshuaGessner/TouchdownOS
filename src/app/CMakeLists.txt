# App framework library
add_library(touchdown_app STATIC
    app.cpp
    app_registry.cpp
    manifest.cpp
)

target_link_libraries(touchdown_app
    touchdown_core
    lvgl
)
