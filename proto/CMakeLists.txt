macro(add_proto_src target)
    if(NOT _PROTOBUF_PROTOC)
        message(FATAL_ERROR "protoc not found!")
    else()
        message("Found protoc at ${_PROTOBUF_PROTOC}")
    endif()

    foreach(PROTO_SRC ${ARGN})



        get_filename_component(rg_proto ${PROTO_SRC} ABSOLUTE)
        get_filename_component(rg_proto_path "${rg_proto}" PATH)

        get_filename_component(PROTO_SRC_NAME ${PROTO_SRC} NAME_WE)

        # Generated sources
        set(rg_proto_srcs "${CMAKE_CURRENT_BINARY_DIR}/${PROTO_SRC_NAME}.pb.cc")
        set(rg_proto_hdrs "${CMAKE_CURRENT_BINARY_DIR}/${PROTO_SRC_NAME}.pb.h")

        set_property(SOURCE ${rg_proto_srcs} PROPERTY SKIP_AUTOGEN ON)
        set_property(SOURCE ${rg_proto_hdrs} PROPERTY SKIP_AUTOGEN ON)

        add_custom_command(
                OUTPUT "${rg_proto_srcs}" "${rg_proto_hdrs}"
                DEPENDS ${PROTO_SRC}
                WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
                COMMAND ${_PROTOBUF_PROTOC}
                ARGS
                --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
                -I "${rg_proto_path}"
                "${rg_proto}"
                COMMENT "Compile proto source ${PROTO_SRC_NAME} ..."
        )
        list(APPEND PROTO_SRCS ${rg_proto_srcs})
        list(APPEND PROTO_HEADS ${rg_proto_hdrs})
    endforeach()

    target_include_directories(${target} PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
    target_sources(${target} PRIVATE ${PROTO_SRCS} ${PROTO_HEADS})

endmacro()

file(GLOB PROTO_FILES *.proto)

add_library(proto OBJECT)

add_proto_src(proto ${PROTO_FILES})

target_include_directories(proto PUBLIC ${CMAKE_CURRENT_LIST_DIR} ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(proto PUBLIC protobuf)