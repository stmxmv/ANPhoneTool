#include "widget.h"

#include <Render/Decoder.h>
#include <Network/DesktopServer.h>
#include <Core/MDNSService.h>

#include <QApplication>
#include <ojoie/Render/RenderContext.hpp>

using namespace AN;

/// log qDebug() to stdout
void customMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    Q_UNUSED(context);

    switch (type) {
        case QtDebugMsg:
            std::cout << qPrintable(msg) << std::endl;
            break;
        default:
            break;
    }
}

int main(int argc, char *argv[])
{
    qInstallMessageHandler(customMessageHandler);

    InitializeRenderContext(AN::kGraphicsAPID3D11);

    Decoder::Initialize();

    QApplication a(argc, argv);


    AN::MDNSService service;

    service.SetRegisterErrorCallback(
            [](const AN::MDNSError &error, void *userInfo)
            {
                AN_LOG(Error, "MDNS Error %s", error.message.c_str());
            });
    service.Register("ANPhoneTool", "_anphonetool._tcp", 13130);

    /// run the server
    GetDesktopServer();

    Widget w;
    w.show();

    int ret = a.exec();

    service.UnRegister();

    Object::DestroyAllObjects();

    Decoder::Deallocate();

    DeallocRenderContext();

    return ret;
}
