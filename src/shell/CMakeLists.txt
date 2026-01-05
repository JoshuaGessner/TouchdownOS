# TouchdownOS Shell - Main LVGL UI
# Create shell library for reuse
add_library(touchdown_shell STATIC
    shell.cpp
    theme_engine.cpp
    home_screen.cpp
    app_launcher.cpp
    circular_layout.cpp
)

target_link_libraries(touchdown_shell
    touchdown-core
    touchdown-drivers
    touchdown-services
    touchdown_app
    touchdown_apps
    lvgl
    ${DRM_LIBRARIES}
    ${DBUS_LIBRARIES}
    ${SYSTEMD_LIBRARIES}
)

# Shell executable
add_executable(touchdown-shell main.cpp)

target_include_directories(touchdown-shell PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(touchdown-shell
    touchdown_shell
    pthread
)

install(TARGETS touchdown-shell
    RUNTIME DESTINATION bin
)
