#include <TBToolkit/Platform.h>
#if TB_PLATFORM_WINDOWS

#include <TBToolkit/Renderer/DirectXRenderer.h>
#include <TBToolkit/Canvas/WindowsCanvas.h>
#include <TBToolkit/Renderer/DirectXModel.h>
#include <TBToolkit/Renderer/DirectXShader.h>
#include <TBToolkit/Renderer/DirectXTexture.h>
#include <OpenGEX.h>
#include <d3d11.h>

namespace TB
{
    DirectXRenderer::DirectXRenderer(const std::shared_ptr<Canvas>& canvas, const std::shared_ptr<IDirectXFrame>& frame)
        : Renderer(canvas)
        , mCanvas(std::dynamic_pointer_cast<WindowsCanvas>(canvas))
        , mDevice()
        , mSwapChain()
        , mImmediateContext()
        , mBackBufferRTV()
        , mDepthStencil()
        , mBackBufferDSV()
        , mFrame(frame)
    {
        TB::runtimeCheck(canvas->getType() == CanvasType::WindowsCanvas);
    }

    DirectXRenderer::~DirectXRenderer()
    {
        deinit();
    }

    bool DirectXRenderer::init()
    {
        HWND hwnd = mCanvas->getHwnd();

        RECT rc;
        GetClientRect(hwnd, &rc);
        UINT width = rc.right - rc.left;
        UINT height = rc.bottom - rc.top;

        UINT createDeviceFlags = 0;
    #ifdef _DEBUG
        createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
    #endif

        D3D_DRIVER_TYPE driverTypes[] =
        {
            D3D_DRIVER_TYPE_HARDWARE,
            D3D_DRIVER_TYPE_WARP,
            D3D_DRIVER_TYPE_REFERENCE,
        };
        UINT numDriverTypes = ARRAYSIZE(driverTypes);

        D3D_FEATURE_LEVEL featureLevels[] =
        {
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_10_0,
        };
        UINT numFeatureLevels = ARRAYSIZE(featureLevels);

        DXGI_SWAP_CHAIN_DESC sd;
        ZeroMemory(&sd, sizeof(sd));
        sd.BufferCount = 1;
        sd.BufferDesc.Width = width;
        sd.BufferDesc.Height = height;
        sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.BufferDesc.RefreshRate.Numerator = 60;
        sd.BufferDesc.RefreshRate.Denominator = 1;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.OutputWindow = hwnd;
        sd.SampleDesc.Count = 8;
        sd.SampleDesc.Quality = 0;
        sd.Windowed = TRUE;

        D3D_FEATURE_LEVEL featureLevel;

        HRESULT hr = S_OK;
        for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
        {
            D3D_DRIVER_TYPE driverType = driverTypes[driverTypeIndex];
            hr = D3D11CreateDeviceAndSwapChain(nullptr, driverType, nullptr, createDeviceFlags, featureLevels, numFeatureLevels,
                                                D3D11_SDK_VERSION, &sd, mSwapChain.getInitRef(), mDevice.getInitRef(), &featureLevel, mImmediateContext.getInitRef());
            if(SUCCEEDED(hr))
            {
                break;
            }
        }
        if(FAILED(hr))
        {
            return false;
        }

        // Create a render target view
        ID3D11Texture2D* pBackBuffer = nullptr;
        hr = mSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
        if (FAILED(hr))
        {
            return false;
        }

        hr = mDevice->CreateRenderTargetView(pBackBuffer, nullptr, mBackBufferRTV.getInitRef());
        pBackBuffer->Release();
        if (FAILED(hr))
        {
            return false;
        }

        // Create depth stencil texture
        D3D11_TEXTURE2D_DESC descDepth;
        ZeroMemory( &descDepth, sizeof(descDepth) );
        descDepth.Width = width;
        descDepth.Height = height;
        descDepth.MipLevels = 1;
        descDepth.ArraySize = 1;
        descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        descDepth.SampleDesc.Count = 8;
        descDepth.SampleDesc.Quality = 0;
        descDepth.Usage = D3D11_USAGE_DEFAULT;
        descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
        descDepth.CPUAccessFlags = 0;
        descDepth.MiscFlags = 0;
        hr = mDevice->CreateTexture2D(&descDepth, nullptr, mDepthStencil.getInitRef());
        if (FAILED(hr))
        {
            return false;
        }

        // Create the depth stencil view
        D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
        ZeroMemory( &descDSV, sizeof(descDSV) );
        descDSV.Format = descDepth.Format;
        descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
        descDSV.Texture2D.MipSlice = 0;
        hr = mDevice->CreateDepthStencilView(mDepthStencil, &descDSV, mBackBufferDSV.getInitRef());
        if (FAILED(hr))
        {
            return false;
        }

        mImmediateContext->OMSetRenderTargets(1, &mBackBufferRTV.getRef(), mBackBufferDSV);

        // Setup the viewport
        D3D11_VIEWPORT vp;
        vp.Width = (FLOAT)width;
        vp.Height = (FLOAT)height;
        vp.MinDepth = 0.0f;
        vp.MaxDepth = 1.0f;
        vp.TopLeftX = 0;
        vp.TopLeftY = 0;
        mImmediateContext->RSSetViewports(1, &vp);

        mImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        if (mFrame != nullptr)
        {
            mFrame->init(shared_from_this());
        }
        return true;
    }

    void DirectXRenderer::deinit()
    {
    }

    void DirectXRenderer::render()
    {
        const FLOAT clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
        mImmediateContext->ClearRenderTargetView(mBackBufferRTV, clearColor);
        mImmediateContext->ClearDepthStencilView(mBackBufferDSV, D3D11_CLEAR_DEPTH, 1.0f, 0);
        if (mFrame != nullptr)
        {
            mFrame->render();
        }
        mSwapChain->Present(0, 0);
    }

    void DirectXRenderer::update(float delta)
    {
        if (mFrame != nullptr)
        {
            mFrame->update(delta);
        }
    }

    std::shared_ptr<Scene> DirectXRenderer::loadScene(const std::string& path)
    {
        OGEX::OpenGexDataDescription dataDesc;
        ODDL::DataResult result = dataDesc.ProcessText(loadText(path).c_str());
        if (result == ODDL::kDataOkay)
        {
            return std::make_shared<Scene>(shared_from_this(), dataDesc);
        }

        return nullptr;
    }

    std::shared_ptr<Shader> DirectXRenderer::loadShader(const std::string& path, const std::string& entryPoint, ShaderType type)
    {
        return std::make_shared<DirectXShader>(shared_from_this(), path, entryPoint, type);
    }

    std::shared_ptr<Texture> DirectXRenderer::loadTexture(const std::string& path)
    {
        return std::make_shared<DirectXTexture>(shared_from_this(), path);
    }

    std::shared_ptr<Renderer> CreateDirectXRenderer(const std::shared_ptr<Canvas>& canvas, const std::shared_ptr<IDirectXFrame>& frame)
    {
        return std::make_shared<DirectXRenderer>(canvas, frame);
    }
}

#endif