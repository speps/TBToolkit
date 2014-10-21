#pragma once
#include <TBToolkit/Platform.h>
#if TB_PLATFORM_WINDOWS

#include <TBToolkit/Common.h>
#include <TBToolkit/Shader.h>

struct ID3D10Blob;
struct ID3D11VertexShader;
struct ID3D11PixelShader;
struct ID3DInclude;

namespace TB
{
    class DirectXShader : public Shader
    {
    public:
        DirectXShader(const std::shared_ptr<class DirectXRenderer>& renderer, const DataChunk& data, const std::string& entryPoint, ShaderType type);
        DirectXShader(const std::shared_ptr<class DirectXRenderer>& renderer, const std::string& path, const std::string& entryPoint, ShaderType type);
        virtual ~DirectXShader();

        const ComPtr<ID3D10Blob>& getBlob() const { return mBlob; }

        operator ID3D11VertexShader*() const { TB::runtimeCheck(mVertexShader.isValid()); return mVertexShader; }
        operator ID3D11PixelShader*() const { TB::runtimeCheck(mPixelShader.isValid()); return mPixelShader; }

    private:
        ComPtr<ID3D10Blob> mBlob;
        ComPtr<ID3D11VertexShader> mVertexShader;
        ComPtr<ID3D11PixelShader> mPixelShader;

        void compileShader(const std::shared_ptr<class DirectXRenderer>& renderer, DataChunk data, ID3DInclude* includes, const std::string& entryPoint, ShaderType type);
    };
}

#endif
