add_an_library(
        WinToast
        STATIC
        wintoastlib.cpp
)

target_include_directories(WinToast PUBLIC ${CMAKE_CURRENT_LIST_DIR})