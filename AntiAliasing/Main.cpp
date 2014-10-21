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

struct SSAAConstants
{
    float invTexelX, invTexelY;
    float padding[2];
};

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
        mScene = mRenderer->loadScene("Content/Scene.ogex");
        mVSBasic = mRenderer->loadShader("Content/BasicEffect.hlsl", "MainVS", TB::ShaderType::Vertex);
        mPSBasic = mRenderer->loadShader("Content/BasicEffect.hlsl", "MainPS", TB::ShaderType::Pixel);
        mVSScreen = mRenderer->loadShader("Content/Screen.hlsl", "MainVS", TB::ShaderType::Vertex);
        mPSScreen = mRenderer->loadShader("Content/Screen.hlsl", "MainPS", TB::ShaderType::Pixel);
        mVSQuincunxSSAA = mRenderer->loadShader("Content/QuincunxSSAA.hlsl", "MainVS", TB::ShaderType::Vertex);
        mPSQuincunxSSAA = mRenderer->loadShader("Content/QuincunxSSAA.hlsl", "MainPS", TB::ShaderType::Pixel);
        mVSFXAA = mRenderer->loadShader("Content/FXAA.hlsl", "MainVS", TB::ShaderType::Vertex);
        mPSFXAA = mRenderer->loadShader("Content/FXAA.hlsl", "MainPS", TB::ShaderType::Pixel);
        mTexture = mRenderer->loadTexture("Content/mosaic.dds");
        mSceneRT = std::make_shared<TB::DirectXTexture>(mRenderer, mRenderer->getWidth(), mRenderer->getHeight(), TB::TextureType::Color, TB::TextureFlags::Target | TB::TextureFlags::ShaderResource);
        mOffsetRT = std::make_shared<TB::DirectXTexture>(mRenderer, mRenderer->getWidth(), mRenderer->getHeight(), TB::TextureType::Color, TB::TextureFlags::Target | TB::TextureFlags::ShaderResource);

        {
            auto camera = mScene->getNode("node2")->getPos();
            auto cameraTarget = mScene->getNode("node3")->getPos();

            mEyePosition = DirectX::XMFLOAT3(camera.x, camera.y, camera.z);
            mLookPosition = DirectX::XMFLOAT3(cameraTarget.x, cameraTarget.y, cameraTarget.z);

            mViewConstants.create(renderer);
        }

        {
            TB::DirectXWorldConstants worldConstants;
            auto lightDir = DirectX::XMFLOAT3(0.707f, 0.0f, 0.707f);
            worldConstants.mainLightDir = DirectX::XMLoadFloat3(&lightDir);
            mWorldConstants.create(renderer);
            mWorldConstants.update(worldConstants);
        }

        {
            SSAAConstants ssaa;
            ssaa.invTexelX = 1.0f / mRenderer->getWidth();
            ssaa.invTexelY = 1.0f / mRenderer->getHeight();

            mSSAAConstants.create(renderer);
            mSSAAConstants.update(ssaa);
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

        // Sampler state
        {
            D3D11_SAMPLER_DESC desc =
            {
                D3D11_FILTER_MIN_MAG_MIP_LINEAR,//D3D11_FILTER Filter;
                D3D11_TEXTURE_ADDRESS_WRAP,//D3D11_TEXTURE_ADDRESS_MODE AddressU;
                D3D11_TEXTURE_ADDRESS_WRAP,//D3D11_TEXTURE_ADDRESS_MODE AddressV;
                D3D11_TEXTURE_ADDRESS_WRAP,//D3D11_TEXTURE_ADDRESS_MODE AddressW;
                0.0f,//FLOAT MipLODBias;
                0,//UINT MaxAnisotropy;
                D3D11_COMPARISON_NEVER,//D3D11_COMPARISON_FUNC ComparisonFunc;
                { 0.0f, 0.0f, 0.0f, 0.0f },//FLOAT BorderColor[ 4 ];
                0,//FLOAT MinLOD;
                0,//FLOAT MaxLOD;
            };

            HRESULT hr = mRenderer->getDevice()->CreateSamplerState(&desc, mSampler.getInitRef());
            TB::runtimeCheck(hr == S_OK);
        }

        // Input layout
        {
            const D3D11_INPUT_ELEMENT_DESC layout[] =
            {
                { "POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
                { "NORMAL",    0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
                { "TEXCOORD",  0, DXGI_FORMAT_R32G32_FLOAT,    2, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            };

            auto vs = std::dynamic_pointer_cast<TB::DirectXShader>(mVSBasic);

            HRESULT hr = mRenderer->getDevice()->CreateInputLayout(layout, 3, vs->getBlob()->GetBufferPointer(), vs->getBlob()->GetBufferSize(), mInputLayout.getInitRef());
            TB::runtimeCheck(hr == S_OK);
        }
    }

    void renderQuincunxSSAA()
    {
        auto imc = mRenderer->getImmediateContext();

        // Scene
        {
            auto vs = std::dynamic_pointer_cast<TB::DirectXShader>(mVSBasic);
            auto ps = std::dynamic_pointer_cast<TB::DirectXShader>(mPSBasic);
            auto tex = std::dynamic_pointer_cast<TB::DirectXTexture>(mTexture);

            ID3D11Buffer* constants[] = { mViewConstants, mWorldConstants };
            ID3D11ShaderResourceView* srvs[] =  { *tex };
            ID3D11SamplerState* samplers[] = { mSampler };

            imc->RSSetState(mRSState);
            imc->IASetInputLayout(mInputLayout);
            imc->VSSetConstantBuffers(0, 2, constants);
            imc->VSSetShader(*vs, nullptr, 0);
            imc->PSSetConstantBuffers(0, 2, constants);
            imc->PSSetShader(*ps, nullptr, 0);
            imc->PSSetShaderResources(0, 1, srvs);
            imc->PSSetSamplers(0, 1, samplers);

            mRenderer->beginEvent(L"RT");
            {
                ID3D11RenderTargetView* rtvs[] = { *mOffsetRT };
                imc->OMSetRenderTargets(1, rtvs, mRenderer->getBackBufferDSV());
                mRenderer->clear(*mOffsetRT, math::float4::zero);
                mRenderer->clear(mRenderer->getBackBufferDSV());

                updateViewConstants(mCurrentEyePosition, mLookPosition, 0.5f, 0.5f);
                mScene->render();

                imc->OMSetRenderTargets(0, nullptr, nullptr);
            }
            mRenderer->endEvent();

            mRenderer->beginEvent(L"Main");
            {
                ID3D11RenderTargetView* rtvs[] = { *mSceneRT };
                imc->OMSetRenderTargets(1, rtvs, mRenderer->getBackBufferDSV());
                mRenderer->clear(*mSceneRT, math::float4::zero);
                mRenderer->clear(mRenderer->getBackBufferDSV());

                updateViewConstants(mCurrentEyePosition, mLookPosition, 0.0f, 0.0f);
                mScene->render();

                imc->OMSetRenderTargets(0, nullptr, nullptr);
            }
            mRenderer->endEvent();
        }

        mRenderer->beginEvent(L"2X Quincunx SSAA");
        {
            ID3D11RenderTargetView* rtvs[] = { mRenderer->getBackBufferRTV() };
            imc->OMSetRenderTargets(1, rtvs, mRenderer->getBackBufferDSV());
            mRenderer->clear(mRenderer->getBackBufferRTV(), math::float4::zero);
            mRenderer->clear(mRenderer->getBackBufferDSV());

            auto vs = std::dynamic_pointer_cast<TB::DirectXShader>(mVSQuincunxSSAA);
            auto ps = std::dynamic_pointer_cast<TB::DirectXShader>(mPSQuincunxSSAA);

            ID3D11Buffer* constants[] = { mSSAAConstants };
            ID3D11ShaderResourceView* srvs[] =  { *mSceneRT, *mOffsetRT };
            ID3D11SamplerState* samplers[] = { mSampler };

            imc->RSSetState(mRSState);
            imc->IASetInputLayout(mInputLayout);
            imc->VSSetConstantBuffers(0, 0, nullptr);
            imc->VSSetShader(*vs, nullptr, 0);
            imc->PSSetConstantBuffers(0, 1, constants);
            imc->PSSetShader(*ps, nullptr, 0);
            imc->PSSetShaderResources(0, 2, srvs);
            imc->PSSetSamplers(0, 1, samplers);

            mRenderer->drawQuad();

            ID3D11ShaderResourceView* srvsNull[] =  { nullptr, nullptr };
            imc->PSSetShaderResources(0, 2, srvsNull);
        }
        mRenderer->endEvent();
    }

    void renderScenePass(ID3D11RenderTargetView* target, const math::float2& offset)
    {
        auto imc = mRenderer->getImmediateContext();

        auto vs = std::dynamic_pointer_cast<TB::DirectXShader>(mVSBasic);
        auto ps = std::dynamic_pointer_cast<TB::DirectXShader>(mPSBasic);
        auto tex = std::dynamic_pointer_cast<TB::DirectXTexture>(mTexture);

        ID3D11Buffer* constants[] = { mViewConstants, mWorldConstants };
        ID3D11ShaderResourceView* srvs[] =  { *tex };
        ID3D11SamplerState* samplers[] = { mSampler };

        imc->OMSetBlendState(TB::DirectXBlendState::get(), nullptr, 0xffffffff);
        imc->OMSetDepthStencilState(TB::DirectXDepthStencilState::get(), 0);
        imc->RSSetState(mRSState);
        imc->IASetInputLayout(mInputLayout);
        imc->VSSetConstantBuffers(0, 2, constants);
        imc->VSSetShader(*vs, nullptr, 0);
        imc->PSSetConstantBuffers(0, 2, constants);
        imc->PSSetShader(*ps, nullptr, 0);
        imc->PSSetShaderResources(0, 1, srvs);
        imc->PSSetSamplers(0, 1, samplers);

        mRenderer->beginEvent(L"Main");
        {
            ID3D11RenderTargetView* rtvs[] = { target };
            imc->OMSetRenderTargets(1, rtvs, mRenderer->getBackBufferDSV());
            mRenderer->clear(target, math::float4::zero);
            mRenderer->clear(mRenderer->getBackBufferDSV());

            updateViewConstants(mCurrentEyePosition, mLookPosition, offset.x, offset.y);
            mScene->render();

            imc->OMSetRenderTargets(0, nullptr, nullptr);
        }
        mRenderer->endEvent();
    }

    void renderRGSS()
    {
        auto imc = mRenderer->getImmediateContext();

        // Offsets for rotated grid
        // | | |3| |
        // |1| | | |
        // | | | |4|
        // | |2| | |
        static const math::float2 offsets[] = { { -0.375f, -0.125f }, { -0.125f, 0.375f }, { 0.125f, -0.375f }, { 0.375f, 0.125f } };

        mScreenConstants.update({ { 0.25f, 0.25f, 0.25f, 0.25f } });

        mRenderer->beginEvent(L"RGSS");
        {
            ID3D11RenderTargetView* rtvs[] = { mRenderer->getBackBufferRTV() };
            ID3D11Buffer* constants[] = { mScreenConstants };
            ID3D11ShaderResourceView* srvs[] =  { *mSceneRT };
            ID3D11SamplerState* samplers[] = { mSampler };

            auto vs = std::dynamic_pointer_cast<TB::DirectXShader>(mVSScreen);
            auto ps = std::dynamic_pointer_cast<TB::DirectXShader>(mPSScreen);

            for (int i = 0; i < 4; i++)
            {
                renderScenePass(*mSceneRT, offsets[i]);

                imc->OMSetRenderTargets(1, rtvs, mRenderer->getBackBufferDSV());

                if (i == 0)
                {
                    mRenderer->clear(mRenderer->getBackBufferRTV(), math::float4::zero);
                    mRenderer->clear(mRenderer->getBackBufferDSV());
                }

                imc->RSSetState(mRSState);
                imc->IASetInputLayout(mInputLayout);
                imc->VSSetConstantBuffers(0, 0, nullptr);
                imc->VSSetShader(*vs, nullptr, 0);
                imc->PSSetConstantBuffers(0, 1, constants);
                imc->PSSetShader(*ps, nullptr, 0);
                imc->PSSetShaderResources(0, 1, srvs);
                imc->PSSetSamplers(0, 1, samplers);
                imc->OMSetBlendState(TB::DirectXBlendState::get<true, TB::BlendOp::Add, TB::BlendFactor::One, TB::BlendFactor::One, TB::BlendOp::Add, TB::BlendFactor::One, TB::BlendFactor::Zero>(), nullptr, 0xffffffff);
                imc->OMSetDepthStencilState(TB::DirectXDepthStencilState::get<false>(), 0);

                mRenderer->drawQuad();

                ID3D11ShaderResourceView* srvsNull[] =  { nullptr, nullptr };
                imc->PSSetShaderResources(0, 2, srvsNull);
            }
        }
        mRenderer->endEvent();
    }

    void render44SS()
    {
        auto imc = mRenderer->getImmediateContext();

        math::float2 offsets[16];
        for (int iy = 0; iy < 4; iy++)
        {
            for (int ix = 0; ix < 4; ix++)
            {
                offsets[iy * 4 + ix] = math::float2(0.125f + ix / 4.0f, 0.125f + iy / 4.0f) - math::float2(0.5f, 0.5f);
            }
        }

        mScreenConstants.update({ { 0.0625f, 0.0625f, 0.0625f, 0.0625f } });

        mRenderer->beginEvent(L"44SS");
        {
            ID3D11RenderTargetView* rtvs[] = { mRenderer->getBackBufferRTV() };
            ID3D11Buffer* constants[] = { mScreenConstants };
            ID3D11ShaderResourceView* srvs[] =  { *mSceneRT };
            ID3D11SamplerState* samplers[] = { mSampler };

            auto vs = std::dynamic_pointer_cast<TB::DirectXShader>(mVSScreen);
            auto ps = std::dynamic_pointer_cast<TB::DirectXShader>(mPSScreen);

            for (int i = 0; i < 16; i++)
            {
                renderScenePass(*mSceneRT, offsets[i]);

                imc->OMSetRenderTargets(1, rtvs, mRenderer->getBackBufferDSV());

                if (i == 0)
                {
                    mRenderer->clear(mRenderer->getBackBufferRTV(), math::float4::zero);
                    mRenderer->clear(mRenderer->getBackBufferDSV());
                }

                imc->RSSetState(mRSState);
                imc->IASetInputLayout(mInputLayout);
                imc->VSSetConstantBuffers(0, 0, nullptr);
                imc->VSSetShader(*vs, nullptr, 0);
                imc->PSSetConstantBuffers(0, 1, constants);
                imc->PSSetShader(*ps, nullptr, 0);
                imc->PSSetShaderResources(0, 1, srvs);
                imc->PSSetSamplers(0, 1, samplers);
                imc->OMSetBlendState(TB::DirectXBlendState::get<true, TB::BlendOp::Add, TB::BlendFactor::One, TB::BlendFactor::One, TB::BlendOp::Add, TB::BlendFactor::One, TB::BlendFactor::Zero>(), nullptr, 0xffffffff);
                imc->OMSetDepthStencilState(TB::DirectXDepthStencilState::get<false>(), 0);

                mRenderer->drawQuad();

                ID3D11ShaderResourceView* srvsNull[] =  { nullptr, nullptr };
                imc->PSSetShaderResources(0, 2, srvsNull);
            }
        }
        mRenderer->endEvent();
    }

    void renderFXAA()
    {
        renderScenePass(*mSceneRT, math::float2::zero);

        auto imc = mRenderer->getImmediateContext();

        mRenderer->beginEvent(L"FXAA");
        {
            ID3D11RenderTargetView* rtvs[] = { mRenderer->getBackBufferRTV() };
            ID3D11Buffer* constants[] = { mScreenConstants };
            ID3D11ShaderResourceView* srvs[] =  { *mSceneRT };
            ID3D11SamplerState* samplers[] = { mSampler };

            auto vs = std::dynamic_pointer_cast<TB::DirectXShader>(mVSFXAA);
            auto ps = std::dynamic_pointer_cast<TB::DirectXShader>(mPSFXAA);

            imc->OMSetRenderTargets(1, rtvs, mRenderer->getBackBufferDSV());
            mRenderer->clear(mRenderer->getBackBufferRTV(), math::float4::zero);
            mRenderer->clear(mRenderer->getBackBufferDSV());

            imc->RSSetState(mRSState);
            imc->IASetInputLayout(mInputLayout);
            imc->VSSetConstantBuffers(0, 0, nullptr);
            imc->VSSetShader(*vs, nullptr, 0);
            imc->PSSetConstantBuffers(0, 1, constants);
            imc->PSSetShader(*ps, nullptr, 0);
            imc->PSSetShaderResources(0, 1, srvs);
            imc->PSSetSamplers(0, 1, samplers);
            imc->OMSetBlendState(TB::DirectXBlendState::get(), nullptr, 0xffffffff);
            imc->OMSetDepthStencilState(TB::DirectXDepthStencilState::get<false>(), 0);

            mRenderer->drawQuad();

            ID3D11ShaderResourceView* srvsNull[] =  { nullptr, nullptr };
            imc->PSSetShaderResources(0, 2, srvsNull);
        }
        mRenderer->endEvent();
    }

    void renderNoAA()
    {
        renderScenePass(mRenderer->getBackBufferRTV(), math::float2::zero);
    }

    virtual void render() override
    {
        //renderQuincunxSSAA();
        //renderRGSS();
        //render44SS();
        //renderFXAA();
        renderNoAA();
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
        mCurrentEyePosition.x += math::Sin(mTimer);
    }

private:
    std::shared_ptr<TB::DirectXRenderer> mRenderer;
    std::shared_ptr<TB::Scene> mScene;
    std::shared_ptr<TB::Shader> mVSBasic;
    std::shared_ptr<TB::Shader> mPSBasic;
    std::shared_ptr<TB::Shader> mVSScreen;
    std::shared_ptr<TB::Shader> mPSScreen;
    std::shared_ptr<TB::Shader> mVSQuincunxSSAA;
    std::shared_ptr<TB::Shader> mPSQuincunxSSAA;
    std::shared_ptr<TB::Shader> mVSFXAA;
    std::shared_ptr<TB::Shader> mPSFXAA;
    std::shared_ptr<TB::Texture> mTexture;
    TB::ComPtr<ID3D11SamplerState> mSampler;
    std::shared_ptr<TB::DirectXTexture> mSceneRT;
    std::shared_ptr<TB::DirectXTexture> mOffsetRT;

    TB::DirectXConstants<TB::DirectXViewConstants> mViewConstants;
    TB::DirectXConstants<TB::DirectXWorldConstants> mWorldConstants;
    TB::DirectXConstants<SSAAConstants> mSSAAConstants;
    TB::DirectXConstants<ScreenConstants> mScreenConstants;
    TB::ComPtr<ID3D11RasterizerState> mRSState;
    TB::ComPtr<ID3D11InputLayout> mInputLayout;

    float mTimer;
    uint64_t mFrameIndex;
    DirectX::XMFLOAT3 mEyePosition;
    DirectX::XMFLOAT3 mCurrentEyePosition;
    DirectX::XMFLOAT3 mLookPosition;
};

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
    auto canvas = TB::CreateWindowsCanvas(hInstance);
    if (!canvas->init(L"Anti Alias", 1280, 720))
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
