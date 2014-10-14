#pragma once
#include <TBToolkit/Platform.h>
#if TB_PLATFORM_WINDOWS

#include <TBToolkit/Common.h>
#include <TBToolkit/Texture.h>

struct ID3D11Resource;
struct ID3D11ShaderResourceView;

namespace TB
{
    class DirectXTexture : public Texture
    {
    public:
        DirectXTexture(const std::shared_ptr<class DirectXRenderer>& renderer, const std::string& path);
        virtual ~DirectXTexture() {}

        operator ID3D11ShaderResourceView*() const { return mSRV; }

    private:
        ComPtr<ID3D11Resource> mResource;
        ComPtr<ID3D11ShaderResourceView> mSRV;
    };
}

#endif
