#pragma once
#include <TBToolkit/Platform.h>
#if TB_PLATFORM_WINDOWS

#include <TBToolkit/Common.h>
#include <TBToolkit/Texture.h>

struct ID3D11Resource;
struct ID3D11RenderTargetView;
struct ID3D11DepthStencilView;
struct ID3D11ShaderResourceView;

namespace TB
{
    class DirectXTexture : public Texture
    {
    public:
        DirectXTexture(const std::shared_ptr<class DirectXRenderer>& renderer, const std::string& path);
        DirectXTexture(const std::shared_ptr<class DirectXRenderer>& renderer, int32_t width, int32_t height, TextureType type, TextureFlags flags);
        virtual ~DirectXTexture() {}

        operator ID3D11RenderTargetView*() const { TB::runtimeCheck(mRTV.isValid()); return mRTV; }
        operator ID3D11DepthStencilView*() const { TB::runtimeCheck(mDSV.isValid()); return mDSV; }
        operator ID3D11ShaderResourceView*() const { TB::runtimeCheck(mSRV.isValid());  return mSRV; }

    private:
        ComPtr<ID3D11Resource> mResource;
        ComPtr<ID3D11RenderTargetView> mRTV;
        ComPtr<ID3D11DepthStencilView> mDSV;
        ComPtr<ID3D11ShaderResourceView> mSRV;
    };
}

#endif
