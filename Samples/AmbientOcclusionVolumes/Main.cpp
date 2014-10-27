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

class DirectXFrame : public TB::IDirectXFrame
{
public:
    DirectXFrame()
        : mTimer(0.0f)
    {
    }

    void init(const std::shared_ptr<TB::DirectXRenderer>& renderer) override
    {
        mRenderer = renderer;
        mScene = mRenderer->loadScene("Content/Scene.ogex");
        mVertexShader = mRenderer->loadShader("Content/BasicEffect.hlsl", "MainVS", TB::ShaderType::Vertex);
        mPixelShader = mRenderer->loadShader("Content/BasicEffect.hlsl", "MainPS", TB::ShaderType::Pixel);
        mTexture = mRenderer->loadTexture("Content/mosaic.dds");

        {
            auto camera = mScene->getNode("node2")->getPos();
            auto cameraTarget = mScene->getNode("node3")->getPos();

            mEyePosition = DirectX::XMFLOAT3(camera.x, camera.y, camera.z);
            mLookPosition = DirectX::XMFLOAT3(cameraTarget.x, cameraTarget.y, cameraTarget.z);

            mViewConstants.create(renderer);
            updateViewConstants(mEyePosition, mLookPosition);
        }

        {
            TB::DirectXWorldConstants worldConstants;
            auto lightDir = DirectX::XMFLOAT3(0.707f, 0.0f, 0.707f);
            worldConstants.mainLightDir = DirectX::XMLoadFloat3(&lightDir);
            mWorldConstants.create(renderer);
            mWorldConstants.update(worldConstants);
        }
    }

    virtual void render() override
    {
        auto imc = mRenderer->getImmediateContext();
        auto vs = std::dynamic_pointer_cast<TB::DirectXShader>(mVertexShader);
        auto ps = std::dynamic_pointer_cast<TB::DirectXShader>(mPixelShader);
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

        mRenderer->clear(mRenderer->getBackBufferRTV(), math::float4(0.0f, 0.0f, 0.0f, 0.0f));
        mRenderer->clear(mRenderer->getBackBufferDSV());
        mScene->render();
    }

    void updateViewConstants(DirectX::XMFLOAT3 eyePos, DirectX::XMFLOAT3 lookPos)
    {
        TB::DirectXViewConstants viewConstants;

        auto up = DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f);
        DirectX::XMMATRIX viewMatrix = DirectX::XMMatrixLookAtRH(DirectX::XMLoadFloat3(&eyePos), DirectX::XMLoadFloat3(&lookPos), DirectX::XMLoadFloat3(&up));
        viewConstants.worldToView = DirectX::XMMatrixTranspose(viewMatrix);

        DirectX::XMMATRIX projMatrix = mRenderer->getProjMatrix(0.78f * 0.625f, 1280, 720, 1.0f);
        viewConstants.viewToClip = DirectX::XMMatrixTranspose(projMatrix);

        mViewConstants.update(viewConstants);
    }

    virtual void update(float delta) override
    {
        mTimer += delta;
        float osc = math::Sin(mTimer);
        auto eyePos = mEyePosition;
        eyePos.x += osc;
        updateViewConstants(eyePos, mLookPosition);
    }

private:
    std::shared_ptr<TB::DirectXRenderer> mRenderer;
    std::shared_ptr<TB::Scene> mScene;
    std::shared_ptr<TB::Shader> mVertexShader;
    std::shared_ptr<TB::Shader> mPixelShader;
    std::shared_ptr<TB::Texture> mTexture;

    TB::DirectXConstants<TB::DirectXViewConstants> mViewConstants;
    TB::DirectXConstants<TB::DirectXWorldConstants> mWorldConstants;

    float mTimer;
    DirectX::XMFLOAT3 mEyePosition;
    DirectX::XMFLOAT3 mLookPosition;
};

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
    auto canvas = TB::CreateWindowsCanvas(hInstance);
    if (!canvas->init(L"Ambient Occlusion Volumes", 1280, 720))
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
