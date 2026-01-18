# Python bindings using pybind11
find_package(pybind11 QUIET)

if(pybind11_FOUND)
    pybind11_add_module(touchdown python_api.cpp)
    
    target_link_libraries(touchdown PRIVATE
        touchdown_app
        touchdown-core
        lvgl
    )
    
    # Install to Python site-packages
    install(TARGETS touchdown 
        LIBRARY DESTINATION ${Python3_SITEARCH}
    )
    
    message(STATUS "Python bindings enabled")
else()
    message(WARNING "pybind11 not found - Python bindings disabled")
endif()
