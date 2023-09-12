#pragma once

#include <QObject>
#include <QSize>
#include <QTemporaryDir>

#include <Network/DeviceSocket.h>
#include <Network/TcpServer.h>
#include <adb/AdbHandler.h>

class AndroidDaemon : public QObject
{
    Q_OBJECT

    enum SERVER_START_STEP {
        SSS_NULL,
        SSS_PUSH,
        SSS_ENABLE_REVERSE,
        SSS_EXECUTE_SERVER,
        SSS_RUNNING,
    };

public:
    AndroidDaemon(const QString &adbPath, QObject *parent = Q_NULLPTR);

    bool start(const QString& serial, quint16 localPort, quint16 maxSize, quint32 bitRate);
    void stop();
    DeviceSocket* getDeviceSocket();

signals:
    void OnStart(bool success);
    void OnConnect(bool success, const QString& deviceName, const QSize& size);
    void OnStop();

private slots:
    void onWorkProcessResult(AdbHandler::AdbResult processResult);

private:
    bool startServerByStep();
    bool pushServer();
    bool removeServer();
    bool enableTunnelReverse();
    bool disableTunnelReverse();
    bool execute();
    QString getServerPath();
    bool readInfo(QString& deviceName, QSize& size);

private:

    QTemporaryDir m_TemporaryDir;
    QString m_AdbPath;

    QString m_serial = "";
    quint16 m_localPort = 0;
    quint16 m_maxSize = 0;
    quint32 m_bitRate = 0;

    SERVER_START_STEP m_serverStartStep = SSS_NULL;

    AdbHandler m_workProcess;
    AdbHandler m_serverProcess;

    QString m_serverPath = "";
    bool m_serverCopiedToDevice = false;
    bool m_enableReverse = false;

    TcpServer m_serverSocket;
    DeviceSocket* m_deviceSocket = Q_NULLPTR;
};