add_library(protobuf INTERFACE)

target_include_directories(protobuf INTERFACE "${CMAKE_CURRENT_LIST_DIR}/include")

if (WIN32)
    set(
            PROTO_LIBS_DEBUG
            "${CMAKE_CURRENT_LIST_DIR}/win32/lib/libprotobufd.lib"
            "${CMAKE_CURRENT_LIST_DIR}/win32/lib/libprotobuf-lited.lib"
            "${CMAKE_CURRENT_LIST_DIR}/win32/lib/libprotocd.lib"
    )
    set(
            PROTO_LIBS
            "${CMAKE_CURRENT_LIST_DIR}/win32/lib/libprotobuf.lib"
            "${CMAKE_CURRENT_LIST_DIR}/win32/lib/libprotobuf-lite.lib"
            "${CMAKE_CURRENT_LIST_DIR}/win32/lib/libprotoc.lib"
    )

    set(_PROTOBUF_PROTOC "${CMAKE_CURRENT_LIST_DIR}/win32/bin/protoc.exe" CACHE STRING "proto compiler")


#    file(GLOB PROTO_PROTO_OTHER_LIBS "${CMAKE_CURRENT_LIST_DIR}/win32/lib/*.lib")
#    message(${PROTO_PROTO_OTHER_LIBS})
#    list(FILTER PROTO_PROTO_OTHER_LIBS EXCLUDE REGEX ".*libprotobufd?.lib")
#    list(FILTER PROTO_PROTO_OTHER_LIBS EXCLUDE REGEX ".*libprotobuf-lited?.lib")
#    list(FILTER PROTO_PROTO_OTHER_LIBS EXCLUDE REGEX ".*libprotocd?.lib")
#    message("  ")
#    message("${PROTO_PROTO_OTHER_LIBS}")

    set(
            PROTO_DLLS
            libprotobuf
            libprotobuf-lite
            libprotoc
    )

#    set(
#            PROTO_DLLS_DEBUG
#            ${CMAKE_CURRENT_LIST_DIR}/win32/bin/libprotobufd.dll
#            ${CMAKE_CURRENT_LIST_DIR}/win32/bin/libprotobuf-lited.dll
#            ${CMAKE_CURRENT_LIST_DIR}/win32/bin/libprotocd.dll
#    )

endif()


target_link_libraries(protobuf)

target_link_libraries(
        protobuf INTERFACE
        ${PROTO_PROTO_OTHER_LIBS}
        $<IF:$<CONFIG:Debug>,${PROTO_LIBS_DEBUG},${PROTO_LIBS}>
)

set_target_properties(
        protobuf PROPERTIES
        RUNTIME_LOCATION "${CMAKE_CURRENT_LIST_DIR}/win32/bin"
        RUNTIME_LIB_NAMES "${PROTO_DLLS}"
)