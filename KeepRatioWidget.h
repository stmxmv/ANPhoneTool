//
// Created by aojoie on 9/12/2023.
//

#pragma once

#include <QPointer>
#include <QWidget>

class KeepRatioWidget : public QWidget
{
    Q_OBJECT
public:
    explicit KeepRatioWidget(QWidget *parent = nullptr);
    ~KeepRatioWidget();

    void setWidget(QWidget *w);
    void setWidthHeightRatio(float widthHeightRatio);
    const QSize goodSize();

protected:
    void resizeEvent(QResizeEvent *event);
    void adjustSubWidget();

private:
    float m_widthHeightRatio = -1.0f;
    QPointer<QWidget> m_subWidget;
    QSize m_goodSize;
};
