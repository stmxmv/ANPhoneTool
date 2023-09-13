
#include "Utilities/Log.h"

#include <QFileInfo>
#include <QCoreApplication>

#include "AndroidDaemon.h"

#define DEVICE_SERVER_PATH "/data/local/tmp/scrcpy-server.jar"
#define SOCKET_NAME "scrcpy"
#define DEVICE_NAME_FIELD_LENGTH 64


AndroidDaemon::AndroidDaemon(const QString &adbPath, QObject *parent)
    : QObject(parent), m_AdbPath(adbPath), m_workProcess(adbPath), m_serverProcess(adbPath)
{
    connect(&m_workProcess, &AdbHandler::OnResult, this, &AndroidDaemon::onWorkProcessResult);
    connect(&m_serverProcess, &AdbHandler::OnResult, this, &AndroidDaemon::onWorkProcessResult);
    connect(&m_serverSocket, &QTcpServer::newConnection, this, [this](){
        m_deviceSocket = dynamic_cast<DeviceSocket*>(m_serverSocket.nextPendingConnection());

        QString deviceName;
        QSize size;
        // devices name, size
        if (m_deviceSocket && m_deviceSocket->isValid() && readInfo(deviceName, size)) {
            disableTunnelReverse();
            removeServer();
            emit OnConnect(true, deviceName, size);
        } else {
            stop();
            emit OnConnect(false, deviceName, size);
        }
    });

    AN_LOG(Info, "Android Damon temporary dir is %s", m_TemporaryDir.path().toStdString().c_str());
}

bool AndroidDaemon::start(const QString &serial, quint16 localPort, quint16 maxSize, quint32 bitRate)
{
    m_serial = serial;
    m_localPort = localPort;
    m_maxSize = maxSize;
    m_bitRate = bitRate;

    // start push server
    m_serverStartStep = SSS_PUSH;
    return startServerByStep();
}

void AndroidDaemon::stop()
{
    if (m_deviceSocket) {
        m_deviceSocket->close();
        /// FIXME
        //m_deviceSocket->deleteLater();
    }

    m_serverProcess.cancel();
    disableTunnelReverse();
    removeServer();
    m_serverSocket.close();
}

DeviceSocket *AndroidDaemon::getDeviceSocket()
{
    return m_deviceSocket;
}

void AndroidDaemon::onWorkProcessResult(AdbHandler::AdbResult processResult)
{
    if (sender() == &m_workProcess) {
        if (SSS_NULL != m_serverStartStep) {
            switch (m_serverStartStep) {
            case SSS_PUSH:
                if (AdbHandler::kAdbSuccess == processResult) {
                    m_serverCopiedToDevice = true;
                    m_serverStartStep = SSS_ENABLE_REVERSE;
                    startServerByStep();

                } else if (AdbHandler::kAdbStarted != processResult) {
                    qCritical("adb push failed");
                    m_serverStartStep = SSS_NULL;
                    emit OnStart(false);
                }

                break;
            case SSS_ENABLE_REVERSE:
                if (AdbHandler::kAdbSuccess == processResult) {
                    m_enableReverse = true;
                    m_serverStartStep = SSS_EXECUTE_SERVER;
                    startServerByStep();
                } else if (AdbHandler::kAdbStarted != processResult) {
                    qCritical("adb reverse failed");
                    m_serverStartStep = SSS_NULL;
                    // removeServer
                    removeServer();
                    emit OnStart(false);
                }
                break;
            default:
                break;
            }
        }
    }

    if (sender() == &m_serverProcess) {
        if (SSS_EXECUTE_SERVER == m_serverStartStep) {
            if (AdbHandler::kAdbStarted == processResult) {
                m_serverStartStep = SSS_RUNNING;
                emit OnStart(true);
            } else if (AdbHandler::kAdbStartFail == processResult) {
                // disable reverse
                disableTunnelReverse();
                qCritical("adb shell start server failed");
                m_serverStartStep = SSS_NULL;
                // removeServer
                removeServer();
                emit OnStart(false);
            }
        } else if (SSS_RUNNING == m_serverStartStep) {
            m_serverStartStep = SSS_NULL;
            emit OnStop();
        }
    }
}

bool AndroidDaemon::startServerByStep()
{
    bool stepSuccess = false;
    // push, enable reverse, execute server
    if (SSS_NULL != m_serverStartStep) {
        switch (m_serverStartStep) {
        case SSS_PUSH:
            stepSuccess = pushServer();
            break;
        case SSS_ENABLE_REVERSE:
            stepSuccess = enableTunnelReverse();
            break;
        case SSS_EXECUTE_SERVER:
            m_serverSocket.setMaxPendingConnections(1);
            if (!m_serverSocket.listen(QHostAddress::LocalHost, m_localPort)) {
                qCritical(QString("Could not listen on port %1").arg(m_localPort).toStdString().c_str());
                m_serverStartStep = SSS_NULL;
                disableTunnelReverse();
                removeServer();
                emit OnStart(false);
                return false;
            }
            stepSuccess = execute();
            break;
        default:
            break;
        }
    }

    if (!stepSuccess) {
        emit OnStart(false);
    }

    return stepSuccess;
}

bool AndroidDaemon::pushServer()
{
    m_workProcess.push(m_serial, getServerPath(), DEVICE_SERVER_PATH);
    return true;
}

bool AndroidDaemon::removeServer()
{
    if (!m_serverCopiedToDevice) {
        return true;
    }
    m_serverCopiedToDevice = false;

    AdbHandler* adb = new AdbHandler(m_AdbPath, this);
    if (!adb) {
        return false;
    }
    connect(adb, &AdbHandler::OnResult, this, [this](AdbHandler::AdbResult processResult){
        if (AdbHandler::kAdbStarted != processResult) {
            sender()->deleteLater();
        }
    });
    adb->removeFile(m_serial, DEVICE_SERVER_PATH);
    return true;
}

bool AndroidDaemon::enableTunnelReverse()
{
    m_workProcess.startReverseProxy(m_serial, SOCKET_NAME, m_localPort);
    return true;
}

bool AndroidDaemon::disableTunnelReverse()
{
    if (!m_enableReverse) {
        return true;
    }
    m_enableReverse = false;

    AdbHandler* adb = new AdbHandler(m_AdbPath, this);
    if (!adb) {
        return false;
    }
    connect(adb, &AdbHandler::OnResult, this, [this](AdbHandler::AdbResult processResult){
        if (AdbHandler::kAdbStarted != processResult) {
            sender()->deleteLater();
        }
    });
    adb->stopReverseProxy(m_serial, SOCKET_NAME);
    return true;
}

bool AndroidDaemon::execute()
{
    // adb shell CLASSPATH=/data/local/tmp/scrcpy-server.jar app_process / com.genymobile.scrcpy.Server 1080 2000000 false ""
    QStringList args;
    args << "shell";
    args << QString("CLASSPATH=%1").arg(DEVICE_SERVER_PATH);
    args << "app_process";
    args << "/";
    args << "com.genymobile.scrcpy.Server";
    args << QString::number(m_maxSize);
    args << QString::number(m_bitRate);
    args << "false";
    args << "";

    m_serverProcess.executeCmd(m_serial, args);
    return true;
}

QString AndroidDaemon::getServerPath()
{
    QFile resourceFile(":/Binary/Resources/scrcpy-server.jar");
    QString path = m_TemporaryDir.path() + "/scrcpy-server.jar";

    AN_LOG(Debug, "copy android daemon to temporary path %s", path.toStdString().c_str());

    if (resourceFile.open(QIODevice::ReadOnly)) {
        QFile tempFile(path);
        if (tempFile.open(QIODevice::WriteOnly)) {
            tempFile.write(resourceFile.readAll());
            tempFile.close();
        }
        resourceFile.close();
    }

    m_serverPath = path;

    return path;
}

bool AndroidDaemon::readInfo(QString &deviceName, QSize &size)
{
    // abk001-----------------------0x0438 0x02d0
    //               64b            2b w   2b h
    unsigned char buf[DEVICE_NAME_FIELD_LENGTH + 4];
    if (m_deviceSocket->bytesAvailable() <= (DEVICE_NAME_FIELD_LENGTH + 4)) {
        m_deviceSocket->waitForReadyRead(300);
    }

    qint64 len = m_deviceSocket->read((char*)buf, sizeof(buf));
    if (len < DEVICE_NAME_FIELD_LENGTH + 4) {
        qInfo("Could not retrieve device information");
        return false;
    }
    buf[DEVICE_NAME_FIELD_LENGTH - 1] = '\0';
    deviceName = (char*)buf;
    size.setWidth((buf[DEVICE_NAME_FIELD_LENGTH] << 8) | buf[DEVICE_NAME_FIELD_LENGTH + 1]);
    size.setHeight((buf[DEVICE_NAME_FIELD_LENGTH + 2] << 8) | buf[DEVICE_NAME_FIELD_LENGTH + 3]);
    return true;
}
