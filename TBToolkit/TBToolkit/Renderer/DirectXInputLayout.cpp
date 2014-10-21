#include <TBToolkit/Platform.h>
#if TB_PLATFORM_WINDOWS

#include <TBToolkit/Common.h>
#include <TBToolkit/Geometry.h>
#include <TBToolkit/Renderer/DirectXInputLayout.h>
#include <TBToolkit/Renderer/DirectXRenderer.h>
#include <TBToolkit/Renderer/DirectXShader.h>
#include <d3d11.h>

namespace TB
{
    std::shared_ptr<DirectXRenderer> DirectXInputLayoutRegistry::mRenderer;
    std::map<uint32_t, ComPtr<ID3D11InputLayout>> DirectXInputLayoutRegistry::mRegistry;

    uint32_t DirectXInputLayoutRegistry::create(const Vertices& vertices)
    {
        TB::runtimeCheck(vertices.size() > 0);

        const char* semantics[] = { "POSITION", "NORMAL", "TEXCOORD" };
        const DXGI_FORMAT formats[] = { DXGI_FORMAT(0), DXGI_FORMAT_R32_FLOAT, DXGI_FORMAT_R32G32_FLOAT, DXGI_FORMAT_R32G32B32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT };

        // Input layout elements
        std::vector<D3D11_INPUT_ELEMENT_DESC> elements;
        for (size_t i = 0; i < vertices.size(); i++)
        {
            const auto& stream = vertices[i];
            elements.push_back({ semantics[(int)stream.semantic], (UINT)stream.usageIndex, formats[stream.elements], (UINT)i, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 });
        }

        // Hash the memory bytes of the element
        uint32_t id = hash(reinterpret_cast<uint8_t*>(&elements[0]), elements.size() * sizeof(D3D11_INPUT_ELEMENT_DESC));
        auto item = mRegistry.find(id);
        if (item == mRegistry.end())
        {
            std::stringstream fakeVSCode;
            const char* memberTypes[] = { nullptr, "float", "float2", "float3", "float4" };

            // Create a fake vertex shader for the input layout
            fakeVSCode << "struct VSInput";
            fakeVSCode << "{";
            for (size_t i = 0; i < vertices.size(); i++)
            {
                const auto& stream = vertices[i];
                fakeVSCode << memberTypes[stream.elements] << " _" << i << " : " << semantics[(int)stream.semantic] << stream.usageIndex << ";";
            }
            fakeVSCode << "};";

            fakeVSCode << "float4 MainVS(VSInput input) : SV_POSITION { return float4(0, 0, 0, 1); }";

            DirectXShader fakeVS(mRenderer, DataChunk(fakeVSCode.str()), "MainVS", ShaderType::Vertex);

            ComPtr<ID3D11InputLayout> inputLayout;
            HRESULT hr = mRenderer->getDevice()->CreateInputLayout(&elements[0], (UINT)elements.size(), fakeVS.getBlob()->GetBufferPointer(), fakeVS.getBlob()->GetBufferSize(), inputLayout.getInitRef());
            TB::runtimeCheck(hr == S_OK);

            mRegistry.emplace(id, inputLayout);
        }

        return id;
    }

    ComPtr<ID3D11InputLayout> DirectXInputLayoutRegistry::get(uint32_t id)
    {
        return mRegistry[id];
    }
}

#endif
