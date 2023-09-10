

qt_add_library(
    adb SHARED
    AdbProcess.cpp
    AdbProcess.h
    typedef.h
)

target_link_libraries(adb PRIVATE Qt${QT_VERSION_MAJOR}::Core)
target_compile_definitions(adb PRIVATE AN_BUILD_ADB)

install(TARGETS adb
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
