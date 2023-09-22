add_library(qrencode INTERFACE)

target_include_directories(qrencode INTERFACE "${CMAKE_CURRENT_LIST_DIR}/include")

set(
        qrencode_LIBS
        "${CMAKE_CURRENT_LIST_DIR}/win32/charset.lib"
        "${CMAKE_CURRENT_LIST_DIR}/win32/iconv.lib"
        "${CMAKE_CURRENT_LIST_DIR}/win32/libpng16.lib"
        "${CMAKE_CURRENT_LIST_DIR}/win32/qrencode.lib"
        "${CMAKE_CURRENT_LIST_DIR}/win32/zlib.lib"
)

set(
        qrencode_LIBS_DEBUG
        "${CMAKE_CURRENT_LIST_DIR}/win32/debug/charset.lib"
        "${CMAKE_CURRENT_LIST_DIR}/win32/debug/iconv.lib"
        "${CMAKE_CURRENT_LIST_DIR}/win32/debug/libpng16d.lib"
        "${CMAKE_CURRENT_LIST_DIR}/win32/debug/qrencoded.lib"
        "${CMAKE_CURRENT_LIST_DIR}/win32/debug/zlibd.lib"
)

target_link_libraries(
        qrencode
        INTERFACE
        $<IF:$<CONFIG:Debug>,${qrencode_LIBS_DEBUG},${qrencode_LIBS}>
)