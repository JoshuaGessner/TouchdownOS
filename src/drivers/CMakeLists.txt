# Driver implementations
add_library(touchdown-drivers STATIC
    display_driver.cpp
    touch_driver.cpp
    button_driver.cpp
)

target_include_directories(touchdown-drivers PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(touchdown-drivers
    touchdown-core
    lvgl
    ${DRM_LIBRARIES}
)

install(TARGETS touchdown-drivers
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)
