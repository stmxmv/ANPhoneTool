#include "widget.h"

#include <Render/Decoder.h>

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
    Widget w;
    w.show();

    int ret = a.exec();

    Object::DestroyAllObjects();

    Decoder::Deallocate();

    DeallocRenderContext();

    return ret;
}
