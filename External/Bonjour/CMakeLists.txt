add_library(Bonjour INTERFACE)

target_include_directories(Bonjour INTERFACE ${CMAKE_CURRENT_LIST_DIR})

if (WIN32)
    target_link_libraries(Bonjour INTERFACE ${CMAKE_CURRENT_LIST_DIR}/win32/dnssd.lib)
endif()