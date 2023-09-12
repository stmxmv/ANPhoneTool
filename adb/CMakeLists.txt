

an_add_qt_library(
    adb SHARED
        AdbHandler.cpp
        AdbHandler.h
    typedef.h
)

target_link_libraries(adb PRIVATE Qt${QT_VERSION_MAJOR}::Core)
target_compile_definitions(adb PRIVATE AN_BUILD_ADB)

install(TARGETS adb
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
