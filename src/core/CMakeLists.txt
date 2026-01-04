# Core library - shared utilities
add_library(touchdown-core STATIC
    logger.cpp
    config.cpp
    utils.cpp
)

target_include_directories(touchdown-core PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(touchdown-core
    ${SYSTEMD_LIBRARIES}
)

install(TARGETS touchdown-core
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)
