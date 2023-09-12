//
// Created by aojoie on 9/11/2023.
//

#pragma once

#include <ojoie/Render/Texture2D.hpp>
#include <ojoie/Utility/Timer.hpp>
#include <Render/QD3D11Layer.h>
#include <QWidget>

class QD3D11VideoWidget : public QWidget
{
    Q_OBJECT

    AN::Timer   m_Timer;
    QD3D11Layer m_Layer;

    QSize m_FrameSize = {-1, -1};
    QSize m_ViewportSize;

    AN::Texture2D *m_UTex;
    AN::Texture2D *m_VTex;
    AN::Texture2D *m_YTex;

    size_t m_BufferSize;
    std::unique_ptr<UInt8[]> buffer;

public:

    QD3D11VideoWidget(QWidget *parent);

    virtual ~QD3D11VideoWidget() override;

    virtual QPaintEngine *paintEngine() const override
    {
        return Q_NULLPTR;
    }

    virtual void paintEvent(QPaintEvent *event) override;
    virtual void resizeEvent(QResizeEvent *event) override;

    virtual QSize minimumSizeHint() const override;
    virtual QSize sizeHint() const override;

    void setFrameSize(const QSize& frameSize);
    const QSize& getFrameSize() const { return m_FrameSize; }

    void updateTextures(quint8* dataY,
                        quint8* dataU,
                        quint8* dataV,
                        quint32 linesizeY,
                        quint32 linesizeU,
                        quint32 linesizeV);

};
