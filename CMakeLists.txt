cmake_minimum_required(VERSION 3.5)

project(ANPhoneTool VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Core Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Core Network)


macro(an_add_qt_executable target)
    qt_add_executable(${target} ${ARGN})
    set_target_properties(
            ${target} PROPERTIES
            LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
            ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
            PDB_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/pdb"
    )
endmacro(an_add_qt_executable)

macro(an_add_qt_library target)
    qt_add_library(${target} ${ARGN})
    set_target_properties(
            ${target} PROPERTIES
            LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
            ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
            PDB_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/pdb"
    )
endmacro(an_add_qt_library)

get_target_property(_qmake_executable Qt${QT_VERSION_MAJOR}::qmake IMPORTED_LOCATION)
get_filename_component(_qt_bin_dir "${_qmake_executable}" DIRECTORY)

function(windeployqt target)

    if (WIN32)
        # POST_BUILD step
        # - after build, we have a bin/lib for analyzing qt dependencies
        # - we run windeployqt on target and deploy Qt libs

        add_custom_command(TARGET ${target} POST_BUILD
                COMMAND "${_qt_bin_dir}/windeployqt.exe"
                --verbose 1
                --debug
                --no-svg
#                --no-angle
                --no-opengl
                --no-opengl-sw
                --no-compiler-runtime
                --no-system-d3d-compiler
                \"$<TARGET_FILE:${target}>\"
                COMMENT "Deploying Qt libraries using windeployqt for compilation target '${target}' ..."
        )

    endif ()

endfunction()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")
include(AN)

include_directories(${CMAKE_SOURCE_DIR})

# add externals
add_subdirectory(External)

# include project libraries
add_subdirectory(Core)
add_subdirectory(Render)
add_subdirectory(Network)
add_subdirectory(Utilities)
add_subdirectory(Tools)
add_subdirectory(adb)


set(PROJECT_SOURCES
        main.cpp
        widget.cpp
        widget.h
        widget.ui
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    an_add_qt_executable(ANPhoneTool
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )

    target_sources(ANPhoneTool
            PRIVATE
            resources.qrc
            MirrorWidget.h
            MirrorWidget.cpp
            MirrorWidget.ui
            KeepRatioWidget.cpp
            KeepRatioWidget.h
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET ANPhoneTool APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(ANPhoneTool SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(ANPhoneTool
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(ANPhoneTool PRIVATE Qt${QT_VERSION_MAJOR}::Widgets adb Core Render)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.ANPhoneTool)
endif()
set_target_properties(ANPhoneTool PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS ANPhoneTool
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(ANPhoneTool)
endif()


# copy Qt runtime library to the build folder
windeployqt(ANPhoneTool)

# copy ojoie runtime
add_custom_command(
        TARGET ANPhoneTool POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different $<TARGET_FILE:ojoie> $<TARGET_FILE_DIR:ANPhoneTool>
)

# copy ffmpeg runtime
get_target_property(FFMPEG_RUNTIMES ffmpeg RUNTIME_LIBS)

foreach (LIB ${FFMPEG_RUNTIMES})
    add_custom_command(
            TARGET ANPhoneTool POST_BUILD
            COMMAND "${CMAKE_COMMAND}" -E copy_if_different ${LIB} $<TARGET_FILE_DIR:ANPhoneTool>
    )
endforeach ()



