an_add_qt_library(
        Threads STATIC
        Log.cpp
        Log.h
)

target_link_libraries(Threads PUBLIC Qt${QT_VERSION_MAJOR}::Core)
target_compile_definitions(Threads PRIVATE AN_BUILD_ADB)