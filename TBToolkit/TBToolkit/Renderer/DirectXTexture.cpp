#include <TBToolkit/Platform.h>
#if TB_PLATFORM_WINDOWS

#include <TBToolkit/Renderer/DirectXTexture.h>
#include <TBToolkit/Renderer/DirectXRenderer.h>
#include <d3d11.h>
#include <DDSTextureLoader.h>

namespace TB
{
    DirectXTexture::DirectXTexture(const std::shared_ptr<class DirectXRenderer>& renderer, const std::string& path)
    {
        auto chunk = loadData(path);
        HRESULT hr = DirectX::CreateDDSTextureFromMemory(renderer->getDevice(), chunk.data(), chunk.size(), mResource.getInitRef(), mSRV.getInitRef());
        TB::runtimeCheck(hr == S_OK);
    }
}

#endif
