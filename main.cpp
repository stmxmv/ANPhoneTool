#include "widget.h"
#include "loginview.h"

#include <Render/Decoder.h>
#include <Network/DesktopServer.h>
#include <Core/MDNSService.h>

#include <QApplication>
#include <ojoie/Render/RenderContext.hpp>

#include "Core/UserNotificationCenter.h"

#include <fstream>

#ifdef _WIN32
#include <debugapi.h>
#endif

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
    std::ofstream file("log.txt");

    if (!IsDebuggerPresent())
    {
        ANLogSetCallback([](const char *log, size_t size, void *userdata) {
                             std::ofstream *file = (std::ofstream *)userdata;
                             (*file) << log << std::endl;
                             file->flush();
                         }, &file);
    }


#ifdef _WIN32
    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
#endif

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
    service.Register("ANPhoneTool", "_anphonetool._tcp", 13139);

    /// run the server
    GetDesktopServerThread().start();
    GetDesktopServer();

    Widget w;
    LoginView loginView;

    QObject::connect(&GetDesktopServer(), &DesktopServer::OnConnected, &loginView, &LoginView::OnLogin);
    QObject::connect(&GetDesktopServer(), &DesktopServer::OnDisConnected, &w, &Widget::OnLogout);

    loginView.SetMainWidget(&w);
    w.SetLoginView(&loginView);
    loginView.show();

    int ret = a.exec();

    GetDesktopServerThread().stopServer();
    GetDesktopServerThread().wait();

    service.UnRegister();

    Object::DestroyAllObjects();

    Decoder::Deallocate();

    DeallocRenderContext();

#ifdef _WIN32
    CoUninitialize();
#endif


    return ret;
}
