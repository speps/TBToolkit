#include <TBToolkit/Platform.h>
#if TB_PLATFORM_WINDOWS

#include <TBToolkit/Renderer/DirectXShader.h>
#include <TBToolkit/Renderer/DirectXRenderer.h>
#include <d3d11.h>
#include <d3dcommon.h>
#include <d3dcompiler.h>

namespace
{
    const char* getTarget(TB::ShaderType type)
    {
        switch (type)
        {
        case TB::ShaderType::Vertex: return "vs_5_0";
        case TB::ShaderType::Hull: return "hs_5_0";
        case TB::ShaderType::Domain: return "ds_5_0";
        case TB::ShaderType::Geometry: return "gs_5_0";
        case TB::ShaderType::Pixel: return "ps_5_0";
        case TB::ShaderType::Compute: return "cs_5_0";
        }
        return 0;
    }
}

namespace TB
{
    DirectXShader::DirectXShader(const std::shared_ptr<DirectXRenderer>& renderer, const std::string& path, const std::string& entryPoint, ShaderType type)
        : Shader(path, entryPoint, type)
    {
        DataChunk data = loadData(path);
        ComPtr<ID3D10Blob> errors;
        HRESULT hr = D3DCompile(data.data(), data.size(), path.c_str(), NULL, NULL, entryPoint.c_str(), getTarget(type), D3DCOMPILE_DEBUG, 0, mBlob.getInitRef(), errors.getInitRef());
        if (hr != S_OK)
        {
            log("%s", errors->GetBufferPointer());
            runtimeCheck(false);
        }

        switch (type)
        {
        case ShaderType::Vertex:
            hr = renderer->getDevice()->CreateVertexShader(mBlob->GetBufferPointer(), mBlob->GetBufferSize(), nullptr, mVertexShader.getInitRef());
            TB::runtimeCheck(hr == S_OK);
            break;
        case ShaderType::Pixel:
            hr = renderer->getDevice()->CreatePixelShader(mBlob->GetBufferPointer(), mBlob->GetBufferSize(), nullptr, mPixelShader.getInitRef());
            TB::runtimeCheck(hr == S_OK);
            break;
        }
    }

    DirectXShader::~DirectXShader()
    {
    }
}

#endif