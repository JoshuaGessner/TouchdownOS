# System services with D-Bus interfaces
add_library(touchdown-services STATIC
    power_service.cpp
    input_service.cpp
    dbus_interface.cpp
    app_manager.cpp
)

target_include_directories(touchdown-services PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(touchdown-services
    touchdown-core
    touchdown-drivers
    touchdown_app
    ${DBUS_LIBRARIES}
    ${SYSTEMD_LIBRARIES}
)

# Service executables
add_executable(touchdown-power-service power_service_main.cpp)
target_link_libraries(touchdown-power-service touchdown-services)

add_executable(touchdown-input-service input_service_main.cpp)
target_link_libraries(touchdown-input-service touchdown-services)

# Installation
install(TARGETS touchdown-services
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(TARGETS touchdown-power-service touchdown-input-service
    RUNTIME DESTINATION bin
)
