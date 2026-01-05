# App framework library
add_library(touchdown_app STATIC
    app.cpp
    app_registry.cpp
    manifest.cpp
)
target_include_directories(touchdown_app PRIVATE
    ${CMAKE_SOURCE_DIR}/third_party/nlohmann
)


target_link_libraries(touchdown_app
    touchdown-core
    lvgl
)
