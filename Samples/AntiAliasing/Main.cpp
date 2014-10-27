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

    static void modifierTriangleUnique(TB::Vertices& vertices, TB::Indices& indices)
    {
        TB::Indices triNew;
        triNew.reserve(indices.size());

        TB::Vertices vertsNew;
        vertsNew.reserve(vertices.size());

        size_t numTris = indices.size() / 3;
        for (size_t sIndex = 0; sIndex < vertices.size(); sIndex++)
        {
            TB::VertexStream s = vertices[sIndex];

            std::vector<float> dataNew;
            dataNew.reserve(indices.size());

            for (size_t triIndex = 0; triIndex < numTris; triIndex++)
            {
                uint32_t i0 = indices[triIndex * 3 + 0];
                uint32_t i1 = indices[triIndex * 3 + 1];
                uint32_t i2 = indices[triIndex * 3 + 2];

                for (size_t vIndex = 0; vIndex < s.elements; vIndex++)
                {
                    dataNew.push_back(s.data[i0 * s.elements + vIndex]);
                }
                for (size_t vIndex = 0; vIndex < s.elements; vIndex++)
                {
                    dataNew.push_back(s.data[i1 * s.elements + vIndex]);
                }
                for (size_t vIndex = 0; vIndex < s.elements; vIndex++)
                {
                    dataNew.push_back(s.data[i2 * s.elements + vIndex]);
                }
            }

            TB::VertexStream sNew = s;
            sNew.data = dataNew;
            vertsNew.push_back(sNew);
        }

        for (size_t triIndex = 0; triIndex < numTris; triIndex++)
        {
            triNew.push_back(uint32_t(triIndex * 3 + 0));
            triNew.push_back(uint32_t(triIndex * 3 + 1));
            triNew.push_back(uint32_t(triIndex * 3 + 2));
        }

        vertices = vertsNew;
        indices = triNew;
    }

    static void modifierTriangleVerts(TB::Vertices& vertices, TB::Indices& indices)
    {
        const std::vector<float>& pos = vertices[0].data;

        TB::VertexStream streamPos1 = { TB::VertexSemantic::Position, 1, {}, 3 };
        streamPos1.data.resize(vertices[0].data.size());
        std::vector<float>& pos1 = streamPos1.data;

        TB::VertexStream streamPos2 = { TB::VertexSemantic::Position, 2, {}, 3 };
        streamPos2.data.resize(vertices[0].data.size());
        std::vector<float>& pos2 = streamPos2.data;

        std::vector<bool> processedVerts;
        processedVerts.resize(vertices[0].data.size() / 3);
        processedVerts.assign(processedVerts.size(), false);

        size_t numTris = indices.size() / 3;
        for (size_t triIndex = 0; triIndex < numTris; triIndex++)
        {
            uint32_t i0 = indices[triIndex * 3 + 0];
            uint32_t i1 = indices[triIndex * 3 + 1];
            uint32_t i2 = indices[triIndex * 3 + 2];

            TB::runtimeCheck(!processedVerts[i0]);
            TB::runtimeCheck(!processedVerts[i1]);
            TB::runtimeCheck(!processedVerts[i2]);

            math::float3 p0(&pos[i0 * 3]);
            math::float3 p1(&pos[i1 * 3]);
            math::float3 p2(&pos[i2 * 3]);

            // Set P1 of V0
            pos1[i0 * 3 + 0] = p1.x;
            pos1[i0 * 3 + 1] = p1.y;
            pos1[i0 * 3 + 2] = p1.z;
            // Set P2 of V0
            pos2[i0 * 3 + 0] = p2.x;
            pos2[i0 * 3 + 1] = p2.y;
            pos2[i0 * 3 + 2] = p2.z;

            // Set P1 of V1
            pos1[i1 * 3 + 0] = p2.x;
            pos1[i1 * 3 + 1] = p2.y;
            pos1[i1 * 3 + 2] = p2.z;
            // Set P2 of V1
            pos2[i1 * 3 + 0] = p0.x;
            pos2[i1 * 3 + 1] = p0.y;
            pos2[i1 * 3 + 2] = p0.z;

            // Set P1 of V2
            pos1[i2 * 3 + 0] = p0.x;
            pos1[i2 * 3 + 1] = p0.y;
            pos1[i2 * 3 + 2] = p0.z;
            // Set P2 of V2
            pos2[i2 * 3 + 0] = p1.x;
            pos2[i2 * 3 + 1] = p1.y;
            pos2[i2 * 3 + 2] = p1.z;

            processedVerts[i0] = true;
            processedVerts[i1] = true;
            processedVerts[i2] = true;
        }

        vertices.push_back(streamPos1);
        vertices.push_back(streamPos2);
    }

    static void modifierScene(TB::Vertices& vertices, TB::Indices& indices)
    {
        modifierTriangleUnique(vertices, indices);
        modifierTriangleVerts(vertices, indices);
    }

    void init(const std::shared_ptr<TB::DirectXRenderer>& renderer) override
    {
        mRenderer = renderer;
        mScene = mRenderer->loadScene("Content/Scene.ogex", modifierScene);
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
            ID3D11SamplerState* samplers[] = { TB::DirectXSamplerState::get() };

            imc->RSSetState(TB::DirectXRasterizerState::get());
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
            ID3D11SamplerState* samplers[] = { TB::DirectXSamplerState::get() };

            imc->RSSetState(TB::DirectXRasterizerState::get());
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
        ID3D11SamplerState* samplers[] = { TB::DirectXSamplerState::get() };

        imc->OMSetBlendState(TB::DirectXBlendState::get(), nullptr, 0xffffffff);
        imc->OMSetDepthStencilState(TB::DirectXDepthStencilState::get(), 0);
        imc->RSSetState(TB::DirectXRasterizerState::get());
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
            ID3D11SamplerState* samplers[] = { TB::DirectXSamplerState::get() };

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

                imc->RSSetState(TB::DirectXRasterizerState::get());
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
            ID3D11SamplerState* samplers[] = { TB::DirectXSamplerState::get() };

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

                imc->RSSetState(TB::DirectXRasterizerState::get());
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
            ID3D11SamplerState* samplers[] = { TB::DirectXSamplerState::get() };

            auto vs = std::dynamic_pointer_cast<TB::DirectXShader>(mVSFXAA);
            auto ps = std::dynamic_pointer_cast<TB::DirectXShader>(mPSFXAA);

            imc->OMSetRenderTargets(1, rtvs, mRenderer->getBackBufferDSV());
            mRenderer->clear(mRenderer->getBackBufferRTV(), math::float4::zero);
            mRenderer->clear(mRenderer->getBackBufferDSV());

            imc->RSSetState(TB::DirectXRasterizerState::get());
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
    std::shared_ptr<TB::DirectXTexture> mSceneRT;
    std::shared_ptr<TB::DirectXTexture> mOffsetRT;

    TB::DirectXConstants<TB::DirectXViewConstants> mViewConstants;
    TB::DirectXConstants<TB::DirectXWorldConstants> mWorldConstants;
    TB::DirectXConstants<SSAAConstants> mSSAAConstants;
    TB::DirectXConstants<ScreenConstants> mScreenConstants;

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
