#ifndef DEVICESOCKET_H
#define DEVICESOCKET_H

#include "DesktopMessage.pb.h"
#include <Core/UUID.h>

#include <QTcpSocket>
#include <QWaitCondition>
#include <QMutex>

class DeviceControlSocket : public QTcpSocket
{
    Q_OBJECT

    int m_CurrentPackageSize;
    QByteArray m_Bytes;

public:

    explicit DeviceControlSocket(QObject *parent);

signals:

    void OnReadMessage(DeviceControlSocket *socket, AN::DesktopMessage message);

};

struct DataHeader {
    AN::UUID uuid;
    size_t blockSize; // block size includes the header size
};

static_assert(sizeof(DataHeader) == 24);

class DeviceDataSocket : public QTcpSocket
{
    Q_OBJECT

    uint64_t m_CurrentPackageSize;
    QByteArray m_Bytes;

public:

    explicit DeviceDataSocket(QObject *parent);

signals:

    void OnReadData(const QByteArray &data);

};

class DeviceSocket : public QTcpSocket
{
    Q_OBJECT
public:
    DeviceSocket(QObject *parent = Q_NULLPTR);
    ~DeviceSocket();

    qint32 subThreadRecvData(quint8* buf, qint32 bufSize);

protected:
    virtual bool event(QEvent *event) override;

protected slots:
    void onReadyRead();
    void quitNotify();

private:
    // 锁
    QMutex m_mutex;
    QWaitCondition m_recvDataCond;

    // 标志
    bool m_recvData = false;
    bool m_quit = false;

    // 数据缓存
    quint8* m_buffer = Q_NULLPTR;
    qint32 m_bufferSize = 0;
    qint32 m_dataSize = 0;
};

#endif // DEVICESOCKET_H
