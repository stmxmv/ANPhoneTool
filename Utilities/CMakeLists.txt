add_library(
        Utilities OBJECT
        Log.cpp
        Log.h
)

target_link_libraries(Utilities PUBLIC Qt${QT_VERSION_MAJOR}::Core)
target_compile_definitions(Utilities PRIVATE AN_BUILD_ADB)