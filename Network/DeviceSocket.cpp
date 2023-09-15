#include <QCoreApplication>
#include <QThread>
#include <QMutexLocker>

#include "Utilities/Log.h"

#include "DeviceSocket.h"
#include "ANPhoneEvent.h"

DeviceSocket::DeviceSocket(QObject *parent)
    : QTcpSocket(parent)
{
    connect(this, &DeviceSocket::readyRead, this, &DeviceSocket::onReadyRead);
    connect(this, &DeviceSocket::disconnected, this, &DeviceSocket::quitNotify);
    connect(this, &DeviceSocket::aboutToClose, this, &DeviceSocket::quitNotify);

}

DeviceSocket::~DeviceSocket()
{

}

qint32 DeviceSocket::subThreadRecvData(quint8 *buf, qint32 bufSize)
{
    // 保证在子线程中调用
    Q_ASSERT(QCoreApplication::instance()->thread() != QThread::currentThread());
    if (m_quit) {
        return 0;
    }

    QMutexLocker locker(&m_mutex);

    m_buffer = buf;
    m_bufferSize = bufSize;
    m_dataSize = 0;

    // 发送事件
    DeviceSocketEvent* getDataEvent = new DeviceSocketEvent();
    QCoreApplication::postEvent(this, getDataEvent);

    while (!m_recvData) {
        m_recvDataCond.wait(&m_mutex);
    }

    m_recvData = false;
    return m_dataSize;
}

bool DeviceSocket::event(QEvent *event)
{
    if (event->type() == ANPhoneEvent::DeviceSocket) {
        onReadyRead();
        return true;
    }
    return QTcpSocket::event(event);
}

void DeviceSocket::onReadyRead()
{
    QMutexLocker locker(&m_mutex);
    if (m_buffer && 0 < bytesAvailable()) {
        // 接收数据
        qint64 readSize = qMin(bytesAvailable(), (qint64)m_bufferSize);
        m_dataSize = read((char*)m_buffer, readSize);

        m_buffer = Q_NULLPTR;
        m_bufferSize = 0;
        m_recvData = true;
        m_recvDataCond.wakeOne();
    }
}

void DeviceSocket::quitNotify()
{
    m_quit = true;
    QMutexLocker locker(&m_mutex);
    if (m_buffer) {
        m_buffer = Q_NULLPTR;
        m_bufferSize = 0;
        m_recvData = true;
        m_dataSize = 0;
        m_recvDataCond.wakeOne();
    }
}

static int readRawVarint32(QByteArray &buffer, int &varintSize) {
    varintSize = 0;
    if (buffer.isEmpty()) {
        return 0;
    }
    uint8_t tmp = buffer[0];
    ++varintSize;
    if (tmp >= 0) {
        return tmp;
    } else {
        int result = tmp & 127;
        if (buffer.size() < 2) {
            return 0;
        }
        ++varintSize;
        if ((tmp = buffer[1]) >= 0) {
            result |= tmp << 7;
        } else {
            result |= (tmp & 127) << 7;
            if (buffer.size() < 3) {
                return 0;
            }
            ++varintSize;
            if ((tmp = buffer[2]) >= 0) {
                result |= tmp << 14;
            } else {
                result |= (tmp & 127) << 14;
                if (buffer.size() < 4) {
                    return 0;
                }
                ++varintSize;
                if ((tmp = buffer[3]) >= 0) {
                    result |= tmp << 21;
                } else {
                    result |= (tmp & 127) << 21;
                    if (buffer.size() < 5) {
                        return 0;
                    }
                    ++varintSize;
                    result |= (tmp = buffer[4]) << 28;
                    if (tmp < 0) {
                        return -1; /// error
                    }
                }
            }
        }
        return result;
    }
}

DeviceControlSocket::DeviceControlSocket(QObject *parent)
    : QTcpSocket(parent), m_CurrentPackageSize()
{
    connect(this, &QTcpSocket::readyRead, this, [this]()
                          {

                              do {

                                  QByteArray tempBytes = readAll();
                                  m_Bytes.append(tempBytes);

                                  if (m_CurrentPackageSize == 0)
                                  {
                                      int varSize = 0;
                                      int size = readRawVarint32(m_Bytes, varSize);
                                      if (size > 0)
                                      {
                                          m_CurrentPackageSize = size;
                                          m_Bytes.remove(0, varSize);
                                      }
                                      else if (size == -1)
                                      {
                                          qInfo() << "Invalid packet! total_size";
                                          close();
                                          deleteLater();
                                          return;
                                      }
                                  }

                                  if (m_CurrentPackageSize != 0 && m_Bytes.length() >= m_CurrentPackageSize)
                                  {
                                      AN::DesktopMessage desktopMessage;
                                      if (desktopMessage.ParseFromArray(m_Bytes.constData(), m_CurrentPackageSize))
                                      {
                                          emit OnReadMessage(this, desktopMessage);
                                      }
                                      else
                                      {
                                          AN_LOG(Error, "server message parse fail");
                                      }

                                      m_Bytes.remove(0, m_CurrentPackageSize);
                                      m_CurrentPackageSize = 0;
                                  }

                              } while (0 == m_CurrentPackageSize && m_Bytes.length() > 1);

                          });
}

DeviceDataSocket::DeviceDataSocket(QObject *parent)
    : QTcpSocket(parent), m_CurrentPackageSize()
{
    connect(this, &QTcpSocket::readyRead, this, [this]()
            {
                do {

                    QByteArray tempBytes = readAll();
                    m_Bytes.append(tempBytes);

                    if (m_CurrentPackageSize == 0)
                    {
                        if (m_Bytes.length() >= sizeof(DataHeader))
                        {
                            const DataHeader *header = (const DataHeader *)m_Bytes.constData();
                            uint64_t size = header->blockSize;
                            m_CurrentPackageSize = size + sizeof(DataHeader);
                            if (m_CurrentPackageSize < sizeof(DataHeader))
                            {
                                qInfo() << "Invalid packet! total_size:" << m_CurrentPackageSize;
                                close();
                                deleteLater();
                                return;
                            }
                        }
                    }

                    if (m_CurrentPackageSize >= sizeof(DataHeader) && m_Bytes.length() >= m_CurrentPackageSize)
                    {
                        emit OnReadData(m_Bytes);

                        m_Bytes.remove(0, m_CurrentPackageSize);
                        m_CurrentPackageSize = 0;
                    }

                } while (0 == m_CurrentPackageSize && m_Bytes.length() >= sizeof(DataHeader));

            });
}
