# Third-party dependencies

# LVGL configuration
if(EXISTS ${CMAKE_SOURCE_DIR}/third_party/lvgl)
    set(LV_CONF_PATH ${CMAKE_SOURCE_DIR}/include/lv_conf.h CACHE STRING "LVGL config file path" FORCE)
    add_subdirectory(lvgl)
    
    # LVGL drivers
    if(EXISTS ${CMAKE_SOURCE_DIR}/third_party/lv_drivers)
        # We'll integrate lv_drivers manually for better control
        file(GLOB_RECURSE LV_DRIVER_SOURCES 
            ${CMAKE_SOURCE_DIR}/third_party/lv_drivers/display/drm.c
            ${CMAKE_SOURCE_DIR}/third_party/lv_drivers/indev/evdev.c
        )
        add_library(lv_drivers STATIC ${LV_DRIVER_SOURCES})
        target_include_directories(lv_drivers PUBLIC ${CMAKE_SOURCE_DIR}/third_party/lv_drivers)
        target_link_libraries(lv_drivers lvgl ${DRM_LIBRARIES})
    endif()
endif()

# msgpack-c configuration
if(EXISTS ${CMAKE_SOURCE_DIR}/third_party/msgpack-c)
    set(MSGPACK_BUILD_EXAMPLES OFF CACHE BOOL "Build msgpack examples" FORCE)
    set(MSGPACK_BUILD_TESTS OFF CACHE BOOL "Build msgpack tests" FORCE)
    add_subdirectory(msgpack-c)
endif()
