#pragma once
#include <TBToolkit/Platform.h>
#if TB_PLATFORM_WINDOWS

#include <TBToolkit/Renderer.h>
#include <TBToolkit/Shader.h>
#include <TBToolkit/Common.h>
#include <memory>
#include <MathGeoLib.h>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <directxmath.h>

struct ID3D11Device;
struct IDXGISwapChain;
struct ID3D11DeviceContext;
struct ID3D11RenderTargetView;
struct ID3D11Texture2D;
struct ID3D11DepthStencilView;
struct ID3DUserDefinedAnnotation;

namespace TB
{
    struct IDirectXFrame
    {
        virtual ~IDirectXFrame() {}
        virtual void init(const std::shared_ptr<class DirectXRenderer>& renderer) {}
        virtual void render() {}
        virtual void update(float delta) {}
    };

    struct DirectXViewConstants
    {
        DirectX::XMMATRIX worldToView;
        DirectX::XMMATRIX viewToClip;
    };

    struct DirectXWorldConstants
    {
        DirectX::XMVECTOR mainLightDir;
    };

    struct DirectXObjectConstants
    {
        DirectX::XMMATRIX localToWorld;
    };

    class DirectXRenderer : public Renderer, public std::enable_shared_from_this<DirectXRenderer>
    {
    public:
        DirectXRenderer(const std::shared_ptr<class Canvas>& canvas, const std::shared_ptr<struct IDirectXFrame>& frame);
        virtual ~DirectXRenderer();

        virtual RendererType getType() const override { return RendererType::DirectXRenderer; }

        virtual bool init(int multisampleLevel) override;
        virtual void deinit() override;
        virtual void render() override;
        virtual void update(float delta) override;

        virtual std::shared_ptr<class Scene> loadScene(const std::string& path) override;
        virtual std::shared_ptr<Shader> loadShader(const std::string& path, const std::string& entryPoint, ShaderType type) override;
        virtual std::shared_ptr<Texture> loadTexture(const std::string& path) override;

        void clear(ID3D11RenderTargetView* rtv, const math::float4& color);
        void clear(ID3D11DepthStencilView* dsv);
        void clear(ID3D11DepthStencilView* dsv, float depth, int stencil);

        DirectX::XMMATRIX getProjMatrix(float fovAngleY, int viewportW, int viewportH, float nearZ) const;
        DirectX::XMMATRIX getProjMatrix(float fovAngleY, int viewportW, int viewportH, float nearZ, float offsetX, float offsetY) const;

        int32_t getWidth() const { return mWidth; }
        int32_t getHeight() const { return mHeight; }
        const ComPtr<ID3D11Device>& getDevice() const { return mDevice; }
        const ComPtr<ID3D11DeviceContext>& getImmediateContext() const { return mImmediateContext; }

        const ComPtr<ID3D11RenderTargetView>& getBackBufferRTV() const { return mBackBufferRTV; }
        const ComPtr<ID3D11DepthStencilView>& getBackBufferDSV() const { return mBackBufferDSV; }

        void beginEvent(const wchar_t* name) const;
        void endEvent() const;

        void drawQuad() const;

    private:
        std::shared_ptr<class WindowsCanvas> mCanvas;
        int32_t mWidth, mHeight;

        ComPtr<ID3D11Device> mDevice;
        ComPtr<IDXGISwapChain> mSwapChain;
        ComPtr<ID3D11DeviceContext> mImmediateContext;

        ComPtr<ID3D11RenderTargetView> mBackBufferRTV;
        ComPtr<ID3D11Texture2D> mDepthStencil;
        ComPtr<ID3D11DepthStencilView> mBackBufferDSV;

        ComPtr<ID3DUserDefinedAnnotation> mUserDefinedAnnotation;

        std::shared_ptr<class DirectXModel> mUnitQuad;
        std::shared_ptr<struct IDirectXFrame> mFrame;
    };
}

#endif