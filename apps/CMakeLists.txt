# TouchdownOS Apps

add_subdirectory(system)

# Installation directories
install(FILES manifests/settings.json
    DESTINATION ${TOUCHDOWN_APP_DIR}/settings/
    RENAME manifest.json
)

install(FILES manifests/info.json
    DESTINATION ${TOUCHDOWN_APP_DIR}/info/
    RENAME manifest.json
)

install(FILES manifests/example.json
    DESTINATION ${TOUCHDOWN_APP_DIR}/com.touchdown.example/
    RENAME manifest.json
)

# Install example Python app
install(FILES ${CMAKE_SOURCE_DIR}/examples/python/example_app.py
    DESTINATION ${TOUCHDOWN_APP_DIR}/com.touchdown.example/
    RENAME main.py
)
