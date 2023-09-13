an_add_qt_library(
        Core STATIC
        MDNSService.cpp
        MDNSService.h
        KeyboardHandler.cpp
        KeyboardHandler.h
)

target_link_libraries(Core PUBLIC Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Network Bonjour Utilities)