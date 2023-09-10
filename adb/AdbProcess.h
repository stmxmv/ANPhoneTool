#ifndef ADBPROCESS_H
#define ADBPROCESS_H

#include <adb/typedef.h>
#include <QProcess>
#include <QVariant>

class AN_API AdbHandler : public QObject
{

    Q_OBJECT

public:

    enum AdbResult
    {
        kAdbSuccess,
        kAdbError,
        kAdbStarted,
        kAdbStartFail,
        kAdbFileNotFound
    };

    Q_ENUM(AdbResult);

private:

    QString m_Path;
    QProcess *m_Process;
    QString m_StdOut;
    QString m_StdError;

    AdbResult m_LastResult;

    QStringList parseDeviceSerials(const QString &str);
    QString parseDeivceIPAdress(const QString &str);

public:

    AdbHandler(const QString &adbPath, QObject *parent = Q_NULLPTR);

    /// async execute command
    void executeCmd(const QString &serial, const QStringList &args);

    /// handy commands
    void cmdGetDevices();
    void cmdGetDeviceIP(); // current device

    void waitUntilComplete();

    void push(const QString &serial, const QString &localPath, const QString &remotePath);

    /// \param path remote path
    void removeFile(const QString &serial, const QString &path);

    void startReverseProxy(const QString &serial, const QString &deviceSocketName, quint16 localPort);

    void stopReverseProxy(const QString &serial, const QString &deviceSocketName);

    /// blocking to get
    QStringList getDeviceSerials();

    QString getDeivceIPAdress();

    AdbResult getLastResult() const { return m_LastResult; }

    QString getStdOut() const { return m_StdOut; }
    QString getStdError() const { return m_StdError; }

signals:

    void OnResult(AdbResult result, QVariant data = {});

};


#endif // ADBPROCESS_H
