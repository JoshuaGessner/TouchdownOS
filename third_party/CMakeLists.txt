# Third-party dependencies

# LVGL configuration
if(EXISTS ${CMAKE_SOURCE_DIR}/third_party/lvgl)
    set(LV_CONF_PATH ${CMAKE_SOURCE_DIR}/include/lv_conf.h CACHE STRING "LVGL config file path" FORCE)
    add_subdirectory(lvgl)
    
    # Note: We use custom drivers in src/drivers/ instead of lv_drivers submodule
endif()

# msgpack-c configuration
if(EXISTS ${CMAKE_SOURCE_DIR}/third_party/msgpack-c)
    set(MSGPACK_BUILD_EXAMPLES OFF CACHE BOOL "Build msgpack examples" FORCE)
    set(MSGPACK_BUILD_TESTS OFF CACHE BOOL "Build msgpack tests" FORCE)
    add_subdirectory(msgpack-c)
endif()
