# TouchdownOS Shell - Main LVGL UI
add_executable(touchdown-shell
    main.cpp
    shell.cpp
    theme_engine.cpp
    home_screen.cpp
    app_launcher.cpp
    circular_layout.cpp
)

target_include_directories(touchdown-shell PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(touchdown-shell
    touchdown-core
    touchdown-drivers
    touchdown-services
    lvgl
    lv_drivers
    ${DRM_LIBRARIES}
    ${DBUS_LIBRARIES}
    ${SYSTEMD_LIBRARIES}
    pthread
)

install(TARGETS touchdown-shell
    RUNTIME DESTINATION bin
)
