an_add_qt_library(
        Render STATIC
        QD3D11VideoWidget.cpp
        QD3D11VideoWidget.h
        QD3D11Layer.cpp
        QD3D11Layer.h
        Decoder.cpp
        Decoder.h
        Frames.cpp
        Frames.h
)

target_link_libraries(Render PUBLIC Qt${QT_VERSION_MAJOR}::Widgets ojoie ffmpeg Network)