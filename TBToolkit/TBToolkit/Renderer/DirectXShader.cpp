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

    struct D3DInclude : ID3DInclude
    {
        std::string currentFile;

        D3DInclude(const std::string& currentFile)
            : currentFile(currentFile)
        {

        }

        virtual HRESULT Open(D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID *ppData, UINT *pBytes) override
        {
            TB::DataChunk data = TB::loadData(pFileName);
            if (!data.isValid())
            {
                return S_FALSE;
            }

            *ppData = data.release();
            *pBytes = (UINT)data.size();

            return S_OK;
        }

        virtual HRESULT Close(LPCVOID pData) override
        {
            delete[] pData;
            return S_OK;
        }
    };
}

namespace TB
{
    DirectXShader::DirectXShader(const std::shared_ptr<DirectXRenderer>& renderer, const std::string& path, const std::string& entryPoint, ShaderType type)
        : Shader(path, entryPoint, type)
    {
        DataChunk data = loadData(path);
        ComPtr<ID3D10Blob> errors;
        D3DInclude include(getCurrentDir() / path);
        HRESULT hr = D3DCompile(data.data(), data.size(), path.c_str(), NULL, &include, entryPoint.c_str(), getTarget(type), D3DCOMPILE_DEBUG | D3DCOMPILE_PREFER_FLOW_CONTROL | D3DCOMPILE_SKIP_OPTIMIZATION, 0, mBlob.getInitRef(), errors.getInitRef());
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