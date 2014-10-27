#include <TBToolkit.h>
#include <TBToolkit/Renderer/DirectXRenderer.h>
#include <TBToolkit/Renderer/DirectXShader.h>
#include <TBToolkit/Renderer/DirectXTexture.h>
#include <TBToolkit/Renderer/DirectXModel.h>
#include <TBToolkit/Renderer/DirectXConstants.h>
#include <TBToolkit/Renderer/DirectXStates.h>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <d3d11.h>
#include <directxmath.h>

struct ScreenConstants
{
    float quadColor[4];
};

class DirectXFrame : public TB::IDirectXFrame
{
public:
    DirectXFrame()
        : mTimer(0.0f)
        , mFrameIndex(0)
    {
    }

    void init(const std::shared_ptr<TB::DirectXRenderer>& renderer) override
    {
        mRenderer = renderer;

        mModel = std::make_shared<TB::DirectXModel>(mRenderer);
        mModel->addMesh({
            { TB::VertexSemantic::Position, 0, { -0.5f, 0.0f, -0.5f, 0.5f, 0.0f, -0.5f, 0.5f, 0.0f, 0.5f }, 3 },
            { TB::VertexSemantic::Color, 0, { 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f }, 4 }
        }, { 0, 1, 2 });

        mVSBasic = mRenderer->loadShader("Content/BasicEffect.hlsl", "MainVS", TB::ShaderType::Vertex);
        mPSBasic = mRenderer->loadShader("Content/BasicEffect.hlsl", "MainPS", TB::ShaderType::Pixel);
        mVSScreen = mRenderer->loadShader("Content/Screen.hlsl", "MainVS", TB::ShaderType::Vertex);
        mPSScreen = mRenderer->loadShader("Content/Screen.hlsl", "MainPS", TB::ShaderType::Pixel);
        mSceneRT = std::make_shared<TB::DirectXTexture>(mRenderer, mRenderer->getWidth() / 4, mRenderer->getHeight() / 4, TB::TextureType::Color, TB::TextureFlags::Target | TB::TextureFlags::ShaderResource);

        {
            mEyePosition = DirectX::XMFLOAT3(0.0f, -10.0f, 0.0f);
            mLookPosition = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);

            mViewConstants.create(renderer);
        }

        {
            TB::DirectXObjectConstants obj;
            obj.localToWorld = DirectX::XMMatrixIdentity();
            mObjectConstants.create(renderer);
            mObjectConstants.update(obj);
        }

        {
            ScreenConstants screen = { 1.0f, 1.0f, 1.0f, 1.0f };
            mScreenConstants.create(renderer);
            mScreenConstants.update(screen);
        }

        // Rasterizer state
        {
            D3D11_RASTERIZER_DESC drd =
            {
                D3D11_FILL_SOLID,//D3D11_FILL_MODE FillMode;
                D3D11_CULL_FRONT,//D3D11_CULL_MODE CullMode;
                FALSE,//BOOL FrontCounterClockwise;
                0,//INT DepthBias;
                0.0,//FLOAT DepthBiasClamp;
                0.0,//FLOAT SlopeScaledDepthBias;
                FALSE,//BOOL DepthClipEnable;
                FALSE,//BOOL ScissorEnable;
                FALSE,//BOOL MultisampleEnable;
                FALSE//BOOL AntialiasedLineEnable;
            };

            HRESULT hr = mRenderer->getDevice()->CreateRasterizerState(&drd, mRSState.getInitRef());
            TB::runtimeCheck(hr == S_OK);
        }
    }

    void renderScenePass(ID3D11RenderTargetView* target, const math::float2& offset)
    {
        auto imc = mRenderer->getImmediateContext();

        auto vs = std::dynamic_pointer_cast<TB::DirectXShader>(mVSBasic);
        auto ps = std::dynamic_pointer_cast<TB::DirectXShader>(mPSBasic);

        ID3D11Buffer* constants[] = { mViewConstants, mObjectConstants };

        imc->OMSetBlendState(TB::DirectXBlendState::get(), nullptr, 0xffffffff);
        imc->OMSetDepthStencilState(TB::DirectXDepthStencilState::get(), 0);
        imc->RSSetState(mRSState);
        imc->VSSetConstantBuffers(0, 2, constants);
        imc->VSSetShader(*vs, nullptr, 0);
        imc->PSSetConstantBuffers(0, 2, constants);
        imc->PSSetShader(*ps, nullptr, 0);

        // Setup the viewport
        D3D11_VIEWPORT vp;
        vp.Width = (FLOAT)mRenderer->getWidth() / 4;
        vp.Height = (FLOAT)mRenderer->getHeight() / 4;
        vp.MinDepth = 0.0f;
        vp.MaxDepth = 1.0f;
        vp.TopLeftX = 0;
        vp.TopLeftY = 0;
        imc->RSSetViewports(1, &vp);

        mRenderer->beginEvent(L"Main");
        {
            ID3D11RenderTargetView* rtvs[] = { target };
            imc->OMSetRenderTargets(1, rtvs, mRenderer->getBackBufferDSV());
            mRenderer->clear(target, math::float4::zero);
            mRenderer->clear(mRenderer->getBackBufferDSV());

            updateViewConstants(mCurrentEyePosition, mLookPosition, offset.x, offset.y);
            mModel->render();

            imc->OMSetRenderTargets(0, nullptr, nullptr);
        }
        mRenderer->endEvent();
    }

    void renderDebug()
    {
        renderScenePass(*mSceneRT, math::float2::zero);

        auto imc = mRenderer->getImmediateContext();

        mRenderer->beginEvent(L"Debug");
        {
            ID3D11RenderTargetView* rtvs[] = { mRenderer->getBackBufferRTV() };
            ID3D11Buffer* constants[] = { mScreenConstants };
            ID3D11ShaderResourceView* srvs[] =  { *mSceneRT };
            ID3D11SamplerState* samplers[] = { TB::DirectXSamplerState::get<TB::FilterMode::MinMagMipPoint>() };

            auto vs = std::dynamic_pointer_cast<TB::DirectXShader>(mVSScreen);
            auto ps = std::dynamic_pointer_cast<TB::DirectXShader>(mPSScreen);

            imc->OMSetRenderTargets(1, rtvs, mRenderer->getBackBufferDSV());
            mRenderer->clear(mRenderer->getBackBufferRTV(), math::float4::zero);
            mRenderer->clear(mRenderer->getBackBufferDSV());

            imc->RSSetState(mRSState);
            imc->VSSetConstantBuffers(0, 0, nullptr);
            imc->VSSetShader(*vs, nullptr, 0);
            imc->PSSetConstantBuffers(0, 1, constants);
            imc->PSSetShader(*ps, nullptr, 0);
            imc->PSSetShaderResources(0, 1, srvs);
            imc->PSSetSamplers(0, 1, samplers);
            imc->OMSetBlendState(TB::DirectXBlendState::get(), nullptr, 0xffffffff);
            imc->OMSetDepthStencilState(TB::DirectXDepthStencilState::get<false>(), 0);

            // Setup the viewport
            D3D11_VIEWPORT vp;
            vp.Width = (FLOAT)mRenderer->getWidth();
            vp.Height = (FLOAT)mRenderer->getHeight();
            vp.MinDepth = 0.0f;
            vp.MaxDepth = 1.0f;
            vp.TopLeftX = 0;
            vp.TopLeftY = 0;
            imc->RSSetViewports(1, &vp);

            mRenderer->drawQuad();

            ID3D11ShaderResourceView* srvsNull[] =  { nullptr, nullptr };
            imc->PSSetShaderResources(0, 2, srvsNull);
        }
        mRenderer->endEvent();
    }

    void renderNormal()
    {
        renderScenePass(mRenderer->getBackBufferRTV(), math::float2::zero);
    }

    virtual void render() override
    {
        //renderNormal();
        renderDebug();
        mFrameIndex++;
    }

    void updateViewConstants(DirectX::XMFLOAT3 eyePos, DirectX::XMFLOAT3 lookPos, float offsetX, float offsetY)
    {
        TB::DirectXViewConstants viewConstants;

        auto up = DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f);
        DirectX::XMMATRIX viewMatrix = DirectX::XMMatrixLookAtRH(DirectX::XMLoadFloat3(&eyePos), DirectX::XMLoadFloat3(&lookPos), DirectX::XMLoadFloat3(&up));
        viewConstants.worldToView = DirectX::XMMatrixTranspose(viewMatrix);

        DirectX::XMMATRIX projMatrix = mRenderer->getProjMatrix(0.78f * 0.625f, 1280, 720, 1.0f, offsetX, -offsetY);
        viewConstants.viewToClip = DirectX::XMMatrixTranspose(projMatrix);

        mViewConstants.update(viewConstants);
    }

    virtual void update(float delta) override
    {
        mTimer += delta;
        mCurrentEyePosition = mEyePosition;
        //mCurrentEyePosition.x += math::Sin(mTimer);
    }

private:
    std::shared_ptr<TB::DirectXRenderer> mRenderer;
    std::shared_ptr<TB::Model> mModel;
    std::shared_ptr<TB::Shader> mVSBasic;
    std::shared_ptr<TB::Shader> mPSBasic;
    std::shared_ptr<TB::Shader> mVSScreen;
    std::shared_ptr<TB::Shader> mPSScreen;
    std::shared_ptr<TB::DirectXTexture> mSceneRT;

    TB::DirectXConstants<TB::DirectXViewConstants> mViewConstants;
    TB::DirectXConstants<TB::DirectXObjectConstants> mObjectConstants;
    TB::DirectXConstants<ScreenConstants> mScreenConstants;
    TB::ComPtr<ID3D11RasterizerState> mRSState;

    float mTimer;
    uint64_t mFrameIndex;
    DirectX::XMFLOAT3 mEyePosition;
    DirectX::XMFLOAT3 mCurrentEyePosition;
    DirectX::XMFLOAT3 mLookPosition;
};

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
    auto canvas = TB::CreateWindowsCanvas(hInstance);
    if (!canvas->init(L"Conservative Rasterization", 1280, 720))
    {
        return 1;
    }

    auto renderer = TB::CreateDirectXRenderer(canvas, std::make_shared<DirectXFrame>());
    if (!renderer->init(1))
    {
        return 1;
    }

    float timeStep = 1.0f / 60.0f;
    double timerAccum = 0.0f;
    double timer = TB::getElapsedTime();

    // Main message loop
    MSG msg = { 0 };
    while (msg.message != WM_QUIT)
    {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            double elapsed = TB::getElapsedTime();
            timerAccum += elapsed - timer;
            timer = elapsed;
            while (timerAccum > timeStep)
            {
                timerAccum -= timeStep;
                renderer->update(timeStep);
            }
            renderer->render();
        }
    }

    return (int)msg.wParam;
}
