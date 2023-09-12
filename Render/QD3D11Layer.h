//
// Created by aojoie on 9/12/2023.
//

#pragma once

#include <QWindow>
#include <QWidget>

#include <ojoie/Core/private/win32/Window.hpp>
#include <ojoie/Object/ObjectPtr.hpp>
#include <ojoie/Render/Layer.hpp>
#include <ojoie/Render/RenderTarget.hpp>
#include <ojoie/Render/private/D3D11/Common.hpp>
#include <ojoie/Render/private/D3D11/Rendertarget.hpp>


struct QD3D11Presentable : public AN::Presentable
{

    AN::ObjectPtr<AN::RenderTarget>   _renderTarget;
    AN::D3D11::ComPtr<IDXGISwapChain> _swapChain;

    UInt32 imageIndex = 0;// this is always 0 for D3D11

    virtual UInt32 getFrameIndex() override
    {
        return imageIndex;
    }

    virtual AN::RenderTarget *getRenderTarget() override
    {
        return _renderTarget.get();
    }
};

class QD3D11Layer
{

    QWidget *m_Window;

    AN::D3D11::ComPtr<IDXGISwapChain1> _swapChain;

    AN::D3D11::RenderTarget _rendertarget;
    QD3D11Presentable _presentable;

public:

    ~QD3D11Layer();

    bool init(QWidget *window);

    QD3D11Presentable *nextPresentable()
    {
        return &_presentable;
    }

    AN::Size getSize();

    void resize(const AN::Size &size);

    void getDpiScale(float &x, float &y);

    QWidget *getWindow() const { return m_Window; }
};