#include <TBToolkit/Platform.h>
#if TB_PLATFORM_WINDOWS

#include <TBToolkit/Renderer/DirectXTexture.h>
#include <TBToolkit/Renderer/DirectXRenderer.h>
#include <d3d11.h>
#include <DDSTextureLoader.h>

namespace TB
{
    DirectXTexture::DirectXTexture(const std::shared_ptr<class DirectXRenderer>& renderer, const std::string& path)
        : mWidth(0), mHeight(0)
    {
        auto chunk = loadData(path);
        HRESULT hr = DirectX::CreateDDSTextureFromMemory(renderer->getDevice(), chunk.data(), chunk.size(), mResource.getInitRef(), mSRV.getInitRef());
        TB::runtimeCheck(hr == S_OK);
    }

    DirectXTexture::DirectXTexture(const std::shared_ptr<class DirectXRenderer>& renderer, int32_t width, int32_t height, TextureType type, TextureFlags flags)
        : mWidth(width), mHeight(height)
    {
        D3D11_TEXTURE2D_DESC desc{};
        desc.Width = width;
        desc.Height = height;
        desc.MipLevels = 1;
        desc.ArraySize = 1;
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.CPUAccessFlags = 0;
        desc.MiscFlags = 0;

        switch (type)
        {
        case TextureType::Color: desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM; break;
        case TextureType::Depth: desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; break;
        }

        if (((int)flags & (int)TextureFlags::Target) != 0)
        {
            desc.BindFlags |= D3D11_BIND_RENDER_TARGET;
        }
        if (((int)flags & (int)TextureFlags::DepthStencil) != 0)
        {
            desc.BindFlags |= D3D11_BIND_DEPTH_STENCIL;
        }
        if (((int)flags & (int)TextureFlags::ShaderResource) != 0)
        {
            desc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;
        }

        ID3D11Texture2D* texture = nullptr;
        HRESULT hr = renderer->getDevice()->CreateTexture2D(&desc, nullptr, &texture);
        TB::runtimeCheck(hr == S_OK);
        if (texture)
        {
            mResource = texture;
            texture->Release();
        }

        if (((int)flags & (int)TextureFlags::Target) != 0)
        {
            hr = renderer->getDevice()->CreateRenderTargetView(mResource, nullptr, mRTV.getInitRef());
            TB::runtimeCheck(hr == S_OK);
        }
        if (((int)flags & (int)TextureFlags::DepthStencil) != 0)
        {
            hr = renderer->getDevice()->CreateDepthStencilView(mResource, nullptr, mDSV.getInitRef());
            TB::runtimeCheck(hr == S_OK);
        }
        if (((int)flags & (int)TextureFlags::ShaderResource) != 0)
        {
            hr = renderer->getDevice()->CreateShaderResourceView(mResource, nullptr, mSRV.getInitRef());
            TB::runtimeCheck(hr == S_OK);
        }
    }
}

#endif
