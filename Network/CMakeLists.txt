an_add_qt_library(
        Network STATIC
        DeviceSocket.cpp
        DeviceSocket.h
        AndroidDaemon.cpp
        AndroidDaemon.h
        TcpServer.cpp
        TcpServer.h
)

target_link_libraries(Network PUBLIC Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Network Bonjour Utilities adb)