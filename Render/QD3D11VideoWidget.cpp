//
// Created by aojoie on 9/12/2023.
//

#include "QD3D11VideoWidget.h"

#include <ojoie/Render/CommandPool.hpp>
#include <ojoie/Render/CommandBuffer.hpp>
#include <ojoie/Render/Shader/Shader.hpp>
#include <ojoie/Render/Material.hpp>

#include <QResizeEvent>
#include <QApplication>
#include <QScreen>
#include <QTimer>
#include <QFile>

using namespace AN;

static QString s_VideoShaderPath = ":/Shaders/Render/Shaders/YUVVideo.shader";

static Shader *s_YUVVideoShader = nullptr;
static Material *s_YUVVideoMat = nullptr;


static void CreateOrResizeTexture(Texture2D * &tex, const QSize &size) {
    if (tex == nullptr)
    {
        tex = NewObject<Texture2D>();
        TextureDescriptor desc{};
        desc.width = size.width();
        desc.height = size.height();
        desc.mipmapLevel = 1;
        desc.pixelFormat = kPixelFormatR8Unorm;

        tex->init(desc);

        // this make it possible to change the texture content
        tex->setReadable(true);

        // upload to create the underlying gpu object so that we bind to commandBuffer, otherwise it is invalid
        tex->uploadToGPU();
    }
    else
    {
        tex->resize(size.width(), size.height());
    }
}

QD3D11VideoWidget::QD3D11VideoWidget(QWidget *parent)
    : QWidget(parent), m_UTex(), m_VTex(), m_YTex(), m_BufferSize()
{
    setAttribute(Qt::WA_PaintOnScreen, true);
    setAttribute(Qt::WA_NativeWindow, true);

    setFocusPolicy(Qt::WheelFocus);
    setMouseTracking(true);

    Q_ASSERT(m_Layer.init(this));

    m_ViewportSize = size();

    /// set refresh timer for test render
//    QTimer *timer = new QTimer(this);
//    connect(timer, &QTimer::timeout, this, QOverload<>::of(&QWidget::update));
//
//    QScreen *primaryScreen = QGuiApplication::primaryScreen();
//    // Get the refresh rate of the primary screen
//    double refreshRate = primaryScreen->refreshRate();
//    timer->start(1.0 / refreshRate * 1000);


    /// compile shaders

    if (s_YUVVideoShader == nullptr)
    {
        QFile file(s_VideoShaderPath);

        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QString content = file.readAll();
            file.close();

            s_YUVVideoShader = NewObject<Shader>();

            if (!s_YUVVideoShader->initWithScriptText(content.toStdString().c_str()))
            {
                AN_LOG(Error, "Compile YUVVideoShader fail");
                DestroyObject(s_YUVVideoShader);
                s_YUVVideoShader = nullptr;
            }
            else
            {
                /// make sure create the underlying gpu object
                s_YUVVideoShader->createGPUObject();

                s_YUVVideoMat = NewObject<Material>();
                Q_ASSERT(s_YUVVideoMat->init(s_YUVVideoShader, "YUVVideoMat"));
            }
        }
    }

}

QD3D11VideoWidget::~QD3D11VideoWidget()
{
    DestroyObject(m_UTex);
    DestroyObject(m_VTex);
    DestroyObject(m_YTex);
}

void QD3D11VideoWidget::paintEvent(QPaintEvent *event)
{
    m_Timer.mark();
    CommandPool &commandPool = GetCommandPool();
    commandPool.reset();

    CommandBuffer *cmd = commandPool.newCommandBuffer();

    Presentable *presentable = m_Layer.nextPresentable();

    AttachmentDescriptor attachments[1];
    attachments[0].clearColor = Vector4f(cosf(m_Timer.elapsedTime), sinf(m_Timer.elapsedTime), 0.f, 1.f);
    attachments[0].format = kRTFormatDefault;
    attachments[0].loadOp = kAttachmentLoadOpClear;
    attachments[0].storeOp = kAttachmentStoreOpStore;
    attachments[0].loadStoreTarget = presentable->getRenderTarget();

    cmd->debugLabelBegin("Render QD3D11VideoWidget", Vector4f(1.F));
    cmd->beginRenderPass(m_ViewportSize.width(), m_ViewportSize.height(), 1, attachments, -1);

    cmd->setViewport({ .originX = 0.f, .originY = 0.f,
                       .width = (float) m_ViewportSize.width(), .height = (float) m_ViewportSize.height(),
                       .znear = 0.f, .zfar = 1.f
                    });

    cmd->setScissor({ .x = 0, .y = 0,
                      .width = m_ViewportSize.width(),
                      .height = m_ViewportSize.height()
                   });

    if (m_YTex && m_UTex && m_VTex && s_YUVVideoMat)
    {
        s_YUVVideoMat->setTexture("_YTex", m_YTex);
        s_YUVVideoMat->setTexture("_UTex", m_UTex);
        s_YUVVideoMat->setTexture("_VTex", m_VTex);
        s_YUVVideoMat->applyMaterial(cmd, 0U);

        cmd->immediateBegin(kPrimitiveQuads);
        cmd->immediateTexCoordAll(0.0f, 1.f, 0.0f);
        cmd->immediateVertex (-1.0f, -1.0f, 0.1f);
        cmd->immediateTexCoordAll (1.f, 1.f, 0.0f);
        cmd->immediateVertex (1.0f, -1.0f, 0.1f);
        cmd->immediateTexCoordAll (1.f,  0.f, 0.0f);
        cmd->immediateVertex (1.0f, 1.0f, 0.1f);
        cmd->immediateTexCoordAll (0.f,  0.f, 0.0f);
        cmd->immediateVertex (-1.0f, 1.0f, 0.1f);
        cmd->immediateEnd();
    }

    cmd->endRenderPass();
    cmd->present(*presentable);
    cmd->debugLabelEnd();
    cmd->submit();
}

void QD3D11VideoWidget::resizeEvent(QResizeEvent *event)
{
    QSize size = event->size();
    m_Layer.resize({ (UInt32)size.width(), (UInt32) size.height() });
    m_ViewportSize = size;
}

QSize QD3D11VideoWidget::minimumSizeHint() const
{
    return QSize(50, 50);
}

QSize QD3D11VideoWidget::sizeHint() const
{
    return size();
}

void QD3D11VideoWidget::setFrameSize(const QSize &frameSize)
{
    if (m_FrameSize != frameSize) {
        m_FrameSize = frameSize;

        /// recreate textures
        QSize halfSize = frameSize / 2.0;
        CreateOrResizeTexture(m_UTex, halfSize);
        CreateOrResizeTexture(m_VTex, halfSize);

        CreateOrResizeTexture(m_YTex, frameSize);

        repaint();
    }
}

void QD3D11VideoWidget::updateTextures(quint8 *dataY,
                                       quint8 *dataU,
                                       quint8 *dataV,
                                       quint32 linesizeY,
                                       quint32 linesizeU,
                                       quint32 linesizeV)
{
    size_t requireSize = m_FrameSize.width() * m_FrameSize.height();
    if (buffer == nullptr || m_BufferSize < requireSize)
    {
        m_BufferSize = requireSize;
        buffer = std::make_unique<UInt8[]>(requireSize);
    }

    for (int i = 0; i < m_FrameSize.height(); ++i)
    {
        int offsetLine = i * linesizeY;
        memcpy(buffer.get() + i * m_FrameSize.width(), dataY + offsetLine, m_FrameSize.width());
    }

    m_YTex->setPixelData(buffer.get());
    m_YTex->uploadToGPU();

    QSize halfSize = m_FrameSize / 2.0;

    for (int i = 0; i < halfSize.height(); ++i)
    {
        int offsetLine = i * linesizeU;
        memcpy(buffer.get() + i * halfSize.width(), dataU + offsetLine, halfSize.width());
    }

    m_UTex->setPixelData(buffer.get());
    m_UTex->uploadToGPU();

    for (int i = 0; i < halfSize.height(); ++i)
    {
        int offsetLine = i * linesizeV;
        memcpy(buffer.get() + i * halfSize.width(), dataV + offsetLine, halfSize.width());
    }
    m_VTex->setPixelData(buffer.get());
    m_VTex->uploadToGPU();

    // update calls paintEvent
    update();
}
