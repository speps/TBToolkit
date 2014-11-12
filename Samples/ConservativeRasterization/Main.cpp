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

struct TempConstants
{
    float quadColor[4];
    float invPixel[2];
    float offset[2];
};

enum class DebugMode
{
    None,
    Wire,
    Output
};

class DirectXFrame : public TB::IDirectXFrame
{
public:
    DirectXFrame()
        : mTimer(0.0f)
        , mFrameIndex(0)
        , mDivider(32)
        , mOffset(math::float2::zero)
        , mModelIndex(0)
        , mUpdateGrid(true)
        , mShowDebugConservative(true)
        , mShowDebugWire(true)
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
        vertices.pop_back();
        vertices.pop_back();
        modifierTriangleUnique(vertices, indices);
        modifierTriangleVerts(vertices, indices);
    }

    void init(const std::shared_ptr<TB::DirectXRenderer>& renderer) override
    {
        mRenderer = renderer;

        auto triModel = std::make_shared<TB::DirectXModel>(mRenderer);
        TB::Vertices v = {
            { TB::VertexSemantic::Position, 0, { -0.7f, 0.0f, -0.42f, 0.57f, 0.0f, -0.45f, 0.8f, 0.0f, 0.2f }, 3 }
        };
        TB::Indices i = { 0, 1, 2 };
        modifierTriangleVerts(v, i);
        triModel->addMesh(v, i);

        mModels.push_back(triModel);

        auto grassModel = mRenderer->loadScene("Content/GrassScene.ogex", modifierScene);
        mModels.push_back(grassModel);

        mVSBasic = mRenderer->loadShader("Content/BasicEffect.hlsl", "MainVS", TB::ShaderType::Vertex);
        mPSBasic = mRenderer->loadShader("Content/BasicEffect.hlsl", "MainPS", TB::ShaderType::Pixel);
        mVSConservative = mRenderer->loadShader("Content/ConservativeEffect.hlsl", "MainVS", TB::ShaderType::Vertex);
        mPSConservative = mRenderer->loadShader("Content/ConservativeEffect.hlsl", "MainPS", TB::ShaderType::Pixel);
        mVSScreen = mRenderer->loadShader("Content/Screen.hlsl", "MainVS", TB::ShaderType::Vertex);
        mPSScreen = mRenderer->loadShader("Content/Screen.hlsl", "MainPS", TB::ShaderType::Pixel);
        mVSScreenColorOnly = mRenderer->loadShader("Content/ScreenColorOnly.hlsl", "MainVS", TB::ShaderType::Vertex);
        mPSScreenColorOnly = mRenderer->loadShader("Content/ScreenColorOnly.hlsl", "MainPS", TB::ShaderType::Pixel);

        {
            mEyePosition = DirectX::XMFLOAT3(0.0f, -4.0f, 0.0f);
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

        {
            TempConstants temp = { 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f };
            mTempConstants.create(renderer);
            mTempConstants.update(temp);
        }
    }

    void renderScenePass(ID3D11RenderTargetView* target, const math::float2& offset, DebugMode debug)
    {
        auto imc = mRenderer->getImmediateContext();

        std::shared_ptr<TB::DirectXShader> vs;
        std::shared_ptr<TB::DirectXShader> ps;

        ID3D11Buffer* constants[] = { mViewConstants, mObjectConstants, mTempConstants };

        int width = mRenderer->getWidth() / mDivider;
        int height = mRenderer->getHeight() / mDivider;
        float invPixelX = 2.0f / width;
        float invPixelY = 2.0f / height;

        if (debug == DebugMode::Wire)
        {
            TempConstants temp = { 0.0f, 0.0f, 1.0f, 1.0f, invPixelX, invPixelY, mOffset.x, mOffset.y };
            mTempConstants.update(temp);
            imc->RSSetState(TB::DirectXRasterizerState::get<TB::CullMode::None, TB::FillMode::Wireframe>());

            vs = std::dynamic_pointer_cast<TB::DirectXShader>(mVSBasic);
            ps = std::dynamic_pointer_cast<TB::DirectXShader>(mPSBasic);
        }
        else if (debug == DebugMode::Output)
        {
            TempConstants temp = { 0.0f, 1.0f, 0.0f, 1.0f, invPixelX, invPixelY, mOffset.x, mOffset.y };
            mTempConstants.update(temp);
            imc->RSSetState(TB::DirectXRasterizerState::get<TB::CullMode::None, TB::FillMode::Wireframe>());

            vs = std::dynamic_pointer_cast<TB::DirectXShader>(mVSConservative);
            ps = std::dynamic_pointer_cast<TB::DirectXShader>(mPSConservative);
        }
        else
        {
            TempConstants temp = { 1.0f, 1.0f, 1.0f, 1.0f, invPixelX, invPixelY, mOffset.x, mOffset.y };
            mTempConstants.update(temp);
            imc->RSSetState(TB::DirectXRasterizerState::get<TB::CullMode::None>());

            if (mShowDebugConservative)
            {
                vs = std::dynamic_pointer_cast<TB::DirectXShader>(mVSConservative);
                ps = std::dynamic_pointer_cast<TB::DirectXShader>(mPSConservative);
            }
            else
            {
                vs = std::dynamic_pointer_cast<TB::DirectXShader>(mVSBasic);
                ps = std::dynamic_pointer_cast<TB::DirectXShader>(mPSBasic);
            }
        }

        imc->OMSetBlendState(TB::DirectXBlendState::get(), nullptr, 0xffffffff);
        imc->OMSetDepthStencilState(TB::DirectXDepthStencilState::get(), 0);
        imc->VSSetConstantBuffers(0, 3, constants);
        imc->VSSetShader(*vs, nullptr, 0);
        imc->PSSetConstantBuffers(0, 3, constants);
        imc->PSSetShader(*ps, nullptr, 0);

        mRenderer->beginEvent(L"Main");
        {
            ID3D11RenderTargetView* rtvs[] = { target };
            imc->OMSetRenderTargets(1, rtvs, nullptr);
            if (debug == DebugMode::None)
            {
                mRenderer->clear(target, math::float4::zero);
            }

            updateViewConstants(mCurrentEyePosition, mLookPosition, offset.x, offset.y);
            mModels[mModelIndex]->render();

            imc->OMSetRenderTargets(0, nullptr, nullptr);
        }
        mRenderer->endEvent();
    }

    void renderDebug()
    {
        auto imc = mRenderer->getImmediateContext();

        {
            // Setup the viewport
            D3D11_VIEWPORT vp;
            vp.Width = (FLOAT)mRenderer->getWidth() / mDivider;
            vp.Height = (FLOAT)mRenderer->getHeight() / mDivider;
            vp.MinDepth = 0.0f;
            vp.MaxDepth = 1.0f;
            vp.TopLeftX = 0;
            vp.TopLeftY = 0;
            imc->RSSetViewports(1, &vp);

            renderScenePass(*mSceneRT, math::float2::zero, DebugMode::None);
        }

        mRenderer->beginEvent(L"Debug");
        {
            ScreenConstants screen = { 1.0f, 1.0f, 1.0f, 1.0f };
            mScreenConstants.update(screen);

            ID3D11RenderTargetView* rtvs[] = { mRenderer->getBackBufferRTV() };
            ID3D11Buffer* constants[] = { mScreenConstants };
            ID3D11ShaderResourceView* srvs[] =  { *mSceneRT };
            ID3D11SamplerState* samplers[] = { TB::DirectXSamplerState::get<TB::FilterMode::MinMagMipPoint>() };

            auto vs = std::dynamic_pointer_cast<TB::DirectXShader>(mVSScreen);
            auto ps = std::dynamic_pointer_cast<TB::DirectXShader>(mPSScreen);

            imc->OMSetRenderTargets(1, rtvs, mRenderer->getBackBufferDSV());
            mRenderer->clear(mRenderer->getBackBufferRTV(), math::float4::zero);
            mRenderer->clear(mRenderer->getBackBufferDSV());

            imc->RSSetState(TB::DirectXRasterizerState::get());
            imc->VSSetConstantBuffers(0, 1, constants);
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
        }
        mRenderer->endEvent();

        mRenderer->beginEvent(L"Grid");
        {
            ScreenConstants screen = { 1.0f, 0.0f, 0.0f, 1.0f };
            mScreenConstants.update(screen);

            ID3D11Buffer* constants[] = { mScreenConstants };

            auto vs = std::dynamic_pointer_cast<TB::DirectXShader>(mVSScreenColorOnly);
            auto ps = std::dynamic_pointer_cast<TB::DirectXShader>(mPSScreenColorOnly);

            imc->RSSetState(TB::DirectXRasterizerState::get());
            imc->VSSetConstantBuffers(0, 1, constants);
            imc->VSSetShader(*vs, nullptr, 0);
            imc->PSSetConstantBuffers(0, 1, constants);
            imc->PSSetShader(*ps, nullptr, 0);
            imc->PSSetShaderResources(0, 0, nullptr);
            imc->PSSetSamplers(0, 0, nullptr);
            imc->OMSetBlendState(TB::DirectXBlendState::get(), nullptr, 0xffffffff);
            imc->OMSetDepthStencilState(TB::DirectXDepthStencilState::get<false>(), 0);

            imc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
            mGridModel->render();
            imc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

            ID3D11ShaderResourceView* srvsNull[] =  { nullptr, nullptr };
            imc->PSSetShaderResources(0, 2, srvsNull);
        }
        mRenderer->endEvent();

        if (mShowDebugWire)
        {
            renderScenePass(mRenderer->getBackBufferRTV(), math::float2::zero, DebugMode::Wire);
            renderScenePass(mRenderer->getBackBufferRTV(), math::float2::zero, DebugMode::Output);
        }
    }

    void renderNormal()
    {
        renderScenePass(mRenderer->getBackBufferRTV(), math::float2::zero, DebugMode::None);
    }

    virtual void render() override
    {
        int width = mRenderer->getWidth() / mDivider;
        int height = mRenderer->getHeight() / mDivider;
        if (!mSceneRT || mSceneRT->getWidth() != width || mSceneRT->getHeight() != height)
        {
            mSceneRT = std::make_shared<TB::DirectXTexture>(mRenderer, width, height, TB::TextureType::Color, TB::TextureFlags::Target | TB::TextureFlags::ShaderResource);
        }

        updateGrid();
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

    void updateGrid()
    {
        if (!mUpdateGrid)
        {
            return;
        }

        mGridModel = std::make_shared<TB::DirectXModel>(mRenderer);
        TB::Vertices v = {
            { TB::VertexSemantic::Position, 0, {}, 3 }
        };
        TB::Indices i;

        if (mDivider > 4)
        {
            int iWidth = mRenderer->getWidth() / mDivider;
            for (int x = 1; x < iWidth; x++)
            {
                float fx = (x - iWidth / 2.0f) / (iWidth / 2.0f);
                uint32_t n = (uint32_t)v[0].data.size() / 3;
                v[0].data.push_back(fx);
                v[0].data.push_back(-1.0f);
                v[0].data.push_back(0.0f);
                v[0].data.push_back(fx);
                v[0].data.push_back(1.0f);
                v[0].data.push_back(0.0f);
                i.push_back(n);
                i.push_back(n + 1);
            }

            int iHeight = mRenderer->getHeight() / mDivider;
            for (int y = 1; y < iHeight; y++)
            {
                float fy = (y - iHeight / 2.0f) / (iHeight / 2.0f);
                uint32_t n = (uint32_t)v[0].data.size() / 3;
                v[0].data.push_back(-1.0f);
                v[0].data.push_back(fy);
                v[0].data.push_back(0.0f);
                v[0].data.push_back(1.0f);
                v[0].data.push_back(fy);
                v[0].data.push_back(0.0f);
                i.push_back(n);
                i.push_back(n + 1);
            }
        }

        mGridModel->addMesh(v, i);

        mUpdateGrid = false;
    }

    virtual void update(float delta) override
    {
        float coeff = 1.0f;
        if (TB::isKeyDown(TB::Key::LShift) || TB::isKeyDown(TB::Key::RShift))
        {
            coeff = 0.1f;
        }
        if (TB::isKeyDown(TB::Key::Home))
        {
            mTimer += delta * coeff;
        }
        if (TB::isKeyDown(TB::Key::End))
        {
            mTimer -= delta * coeff;
        }

        float s = math::Sin(mTimer);
        float c = math::Cos(mTimer);
        mCurrentEyePosition.x = s * -1.0f;
        mCurrentEyePosition.y = c * -1.0f;
        mCurrentEyePosition.z = 2.0f;

        if (TB::isKeyPressed(TB::Key::PageDown))
        {
            mDivider = TB::clamp(mDivider * 2, 1, 32);
            mUpdateGrid = true;
        }
        if (TB::isKeyPressed(TB::Key::PageUp))
        {
            mDivider = TB::clamp(mDivider / 2, 1, 32);
            mUpdateGrid = true;
        }

        if (TB::isKeyPressed(TB::Key::Left))
        {
            mOffset.x -= 1.0f;
        }
        if (TB::isKeyPressed(TB::Key::Right))
        {
            mOffset.x += 1.0f;
        }
        if (TB::isKeyPressed(TB::Key::Up))
        {
            mOffset.y += 1.0f;
        }
        if (TB::isKeyPressed(TB::Key::Down))
        {
            mOffset.y -= 1.0f;
        }

        if (TB::isKeyPressed(TB::Key::C))
        {
            mShowDebugConservative = !mShowDebugConservative;
        }
        if (TB::isKeyPressed(TB::Key::W))
        {
            mShowDebugWire = !mShowDebugWire;
        }
    }

private:
    std::shared_ptr<TB::DirectXRenderer> mRenderer;
    std::shared_ptr<TB::Model> mGridModel;
    std::shared_ptr<TB::Shader> mVSBasic;
    std::shared_ptr<TB::Shader> mPSBasic;
    std::shared_ptr<TB::Shader> mVSConservative;
    std::shared_ptr<TB::Shader> mPSConservative;
    std::shared_ptr<TB::Shader> mVSScreen;
    std::shared_ptr<TB::Shader> mPSScreen;
    std::shared_ptr<TB::Shader> mVSScreenColorOnly;
    std::shared_ptr<TB::Shader> mPSScreenColorOnly;
    std::shared_ptr<TB::DirectXTexture> mSceneRT;

    std::vector<std::shared_ptr<TB::Renderable>> mModels;
    int mModelIndex;

    bool mUpdateGrid;
    bool mShowDebugConservative;
    bool mShowDebugWire;

    TB::DirectXConstants<TB::DirectXViewConstants> mViewConstants;
    TB::DirectXConstants<TB::DirectXObjectConstants> mObjectConstants;
    TB::DirectXConstants<ScreenConstants> mScreenConstants;
    TB::DirectXConstants<TempConstants> mTempConstants;

    float mTimer;
    uint64_t mFrameIndex;
    DirectX::XMFLOAT3 mEyePosition;
    DirectX::XMFLOAT3 mCurrentEyePosition;
    DirectX::XMFLOAT3 mLookPosition;
    math::float2 mOffset;

    int mDivider;
};

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
    auto canvas = TB::CreateWindowsCanvas(hInstance);
    if (!canvas->init(L"Conservative Rasterization", 1280, 736))
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
