add_library(ffmpeg INTERFACE)

set(
        FFMPEG_LIBS
        ${CMAKE_CURRENT_LIST_DIR}/lib/avcodec.lib
        ${CMAKE_CURRENT_LIST_DIR}/lib/avdevice.lib
        ${CMAKE_CURRENT_LIST_DIR}/lib/avfilter.lib
        ${CMAKE_CURRENT_LIST_DIR}/lib/avformat.lib
        ${CMAKE_CURRENT_LIST_DIR}/lib/avutil.lib
        ${CMAKE_CURRENT_LIST_DIR}/lib/postproc.lib
        ${CMAKE_CURRENT_LIST_DIR}/lib/swresample.lib
        ${CMAKE_CURRENT_LIST_DIR}/lib/swscale.lib
)

set(
        FFMPEG_DLLS
        ${CMAKE_CURRENT_LIST_DIR}/bin/avcodec-60.dll
        ${CMAKE_CURRENT_LIST_DIR}/bin/avdevice-60.dll
        ${CMAKE_CURRENT_LIST_DIR}/bin/avfilter-9.dll
        ${CMAKE_CURRENT_LIST_DIR}/bin/avformat-60.dll
        ${CMAKE_CURRENT_LIST_DIR}/bin/avutil-58.dll
        ${CMAKE_CURRENT_LIST_DIR}/bin/postproc-57.dll
        ${CMAKE_CURRENT_LIST_DIR}/bin/swresample-4.dll
        ${CMAKE_CURRENT_LIST_DIR}/bin/swscale-7.dll
)

target_include_directories(ffmpeg INTERFACE "${CMAKE_CURRENT_LIST_DIR}/include")

set_target_properties(
        ffmpeg PROPERTIES
        RUNTIME_LIBS "${FFMPEG_DLLS}"
)

target_link_libraries(
        ffmpeg INTERFACE
        ${FFMPEG_LIBS}
)