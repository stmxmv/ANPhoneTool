#include "AdbHandler.h"

#include <QRegularExpression>
#include <QDebug>

QStringList AdbHandler::parseDeviceSerials(const QString &str)
{
    static QRegularExpression s_Reg0("\r\n|\n");
    static QRegularExpression s_Reg1("\t");

    QStringList serials;
    QStringList deviceInfoList = str.split(s_Reg0, Qt::SkipEmptyParts);

    for (const QString &deviceInfo : deviceInfoList)
    {
        QStringList deviceInfos = deviceInfo.split(s_Reg1, Qt::SkipEmptyParts);
        if (deviceInfos.count() == 2 && deviceInfos[1] == "device")
        {
            serials << deviceInfos[0];
        }
    }

    return serials;
}

QString AdbHandler::parseDeivceIPAdress(const QString &str)
{
    static QRegularExpression s_Reg("inet [\\d.]*",  QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatch match = s_Reg.match(str);

    QString ip;
    if (match.hasMatch())
    {
        ip = match.captured(0);
        ip = ip.right(ip.size() - 5);
    }

    return ip;
}

AdbHandler::AdbHandler(const QString &adbPath, QObject *parent)
    : m_Path(adbPath), m_LastResult()
{

    m_Process = new QProcess(this);

    /// add QProcess signals

    connect(this, &AdbHandler::OnResult, this, [this](AdbResult result)
            {
                m_LastResult = result;
            });

    connect(m_Process, &QProcess::errorOccurred, this, [this](QProcess::ProcessError error)
            {
                qDebug() << error;
                if (error == QProcess::FailedToStart)
                {
                    emit OnResult(kAdbFileNotFound);
                }
                else
                {
                    emit OnResult(kAdbStartFail);
                }
            });

    connect(m_Process, &QProcess::finished, this, [this](int exitCode, QProcess::ExitStatus exitStatus)
            {
                qDebug() << exitCode << exitStatus;

                if (exitStatus == QProcess::NormalExit && exitCode == 0)
                {
                    emit OnResult(kAdbSuccess);
                }
                else
                {
                    emit OnResult(kAdbError);
                }
            });

    connect(m_Process, &QProcess::readyReadStandardError, this, [this]()
            {
                m_StdError = m_Process->readAllStandardError();
                m_StdError = m_StdError.trimmed();
                qDebug() << m_StdError;
            });

    connect(m_Process, &QProcess::readyReadStandardOutput, this, [this]()
            {
                m_StdOut = m_Process->readAllStandardOutput();
                m_StdOut = m_StdOut.trimmed();
                qDebug() << m_StdOut;
            });

    connect(m_Process, &QProcess::started, this, [this]()
            {
                qDebug() << "started";
                emit OnResult(kAdbStarted);
            });

    connect(m_Process, &QProcess::stateChanged, this, [](QProcess::ProcessState newState)
            {
                qDebug() << "state changed to" << newState;
            });
}

AdbHandler::~AdbHandler()
{
}

void AdbHandler::executeCmd(const QString &serial, const QStringList &args)
{
    QStringList adbArgs;
    if (!serial.isEmpty())
    {
        adbArgs << "-s" << serial;
    }
    adbArgs << args;

    qDebug() << m_Path << adbArgs.join(" ");
    m_Process->start(m_Path, adbArgs);
}

void AdbHandler::cmdGetDevices()
{
    QStringList args;
    args << "devices";
    executeCmd({}, args);
}

void AdbHandler::cmdGetDeviceIP()
{
    QStringList args;
    args << "shell";
    args << "ip";
    args << "-f";
    args << "inet";
    args << "addr";
    args << "show";
    args << "eth0";
    executeCmd({}, args);
}

void AdbHandler::waitUntilComplete()
{
    m_Process->waitForFinished();
}

void AdbHandler::cancel()
{
    m_Process->kill();
}

void AdbHandler::push(const QString &serial, const QString &localPath, const QString &remotePath)
{
    QStringList adbArgs;
    adbArgs << "push";
    adbArgs << localPath << remotePath;
    executeCmd(serial, adbArgs);
}

void AdbHandler::removeFile(const QString &serial, const QString &path)
{
    QStringList adbArgs;
    adbArgs << "shell";
    adbArgs << "rm";
    adbArgs << path;
    executeCmd(serial, adbArgs);
}

void AdbHandler::startReverseProxy(const QString &serial, const QString &deviceSocketName, quint16 localPort)
{
    QStringList adbArgs;
    adbArgs << "reverse";
    adbArgs << QString("localabstract:%1").arg(deviceSocketName);
    adbArgs << QString("tcp:%1").arg(localPort);
    executeCmd(serial, adbArgs);
}

void AdbHandler::stopReverseProxy(const QString &serial, const QString &deviceSocketName)
{
    QStringList adbArgs;
    adbArgs << "reverse";
    adbArgs << "--remove";
    adbArgs << QString("localabstract:%1").arg(deviceSocketName);
    executeCmd(serial, adbArgs);
}

QStringList AdbHandler::getDeviceSerials()
{
    m_StdOut.clear();

    cmdGetDevices();
    waitUntilComplete();

    if (getLastResult() != kAdbSuccess)
    {
        return {};
    }

    return parseDeviceSerials(m_StdOut);
}

QString AdbHandler::getDeivceIPAddress()
{
    m_StdOut.clear();

    cmdGetDeviceIP();
    waitUntilComplete();

    if (getLastResult() != kAdbSuccess)
    {
        return {};
    }

    return parseDeivceIPAdress(m_StdOut);
}
