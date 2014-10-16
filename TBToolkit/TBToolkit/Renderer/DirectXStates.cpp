#include <TBToolkit/Platform.h>
#if TB_PLATFORM_WINDOWS

#include <TBToolkit/Common.h>
#include <TBToolkit/Renderer/DirectXStates.h>
#include <TBToolkit/Renderer/DirectXRenderer.h>
#include <d3d11.h>

namespace
{
    D3D11_BLEND_OP translate(TB::BlendOp op)
    {
        switch (op)
        {
        case TB::BlendOp::Add: return D3D11_BLEND_OP_ADD;
        case TB::BlendOp::Substract: return D3D11_BLEND_OP_SUBTRACT;
        case TB::BlendOp::ReverseSubstract: return D3D11_BLEND_OP_REV_SUBTRACT;
        case TB::BlendOp::Min: return D3D11_BLEND_OP_MIN;
        case TB::BlendOp::Max: return D3D11_BLEND_OP_MAX;
        }
        return (D3D11_BLEND_OP)0;
    }

    D3D11_BLEND translate(TB::BlendFactor factor)
    {
        switch (factor)
        {
        case TB::BlendFactor::Zero: return D3D11_BLEND_ZERO;
        case TB::BlendFactor::One: return D3D11_BLEND_ONE;
        case TB::BlendFactor::SrcColor: return D3D11_BLEND_SRC_COLOR;
        case TB::BlendFactor::InvSrcColor: return D3D11_BLEND_INV_SRC_COLOR;
        case TB::BlendFactor::SrcAlpha: return D3D11_BLEND_SRC_ALPHA;
        case TB::BlendFactor::InvSrcAlpha: return D3D11_BLEND_INV_SRC_ALPHA;
        case TB::BlendFactor::DestAlpha: return D3D11_BLEND_DEST_ALPHA;
        case TB::BlendFactor::InvDestAlpha: return D3D11_BLEND_INV_DEST_ALPHA;
        case TB::BlendFactor::DestColor: return D3D11_BLEND_DEST_COLOR;
        case TB::BlendFactor::InvDestColor: return D3D11_BLEND_INV_DEST_COLOR;
        default: TB::runtimeCheck(false);
        }
        return (D3D11_BLEND)0;
    }
}

namespace TB
{
    std::shared_ptr<DirectXRenderer> DirectXStateRegistry::mRenderer;
    std::map<const char*, std::map<uint32_t, ComPtr<ID3D11DeviceChild>>> DirectXStateRegistry::mRegistry;

    const char* TB::TypeTraits<DirectXBlendState>::Name = "DirectXBlendState";

    uint32_t DirectXBlendState::State::key() const
    {
        return hash((uint8_t*)this, sizeof(State));
    }

    ID3D11BlendState* DirectXBlendState::getOrCreate(const State& state)
    {
        auto key = state.key();
        auto value = DirectXStateRegistry::get<DirectXBlendState>(key);
        if (!value.isValid())
        {
            D3D11_BLEND_DESC desc = {0};
            desc.RenderTarget[0].BlendEnable = state.enable;
            desc.RenderTarget[0].BlendOp = translate(state.op);
            desc.RenderTarget[0].SrcBlend = translate(state.srcBlend);
            desc.RenderTarget[0].DestBlend = translate(state.destBlend);
            desc.RenderTarget[0].BlendOpAlpha = translate(state.opAlpha);
            desc.RenderTarget[0].SrcBlendAlpha = translate(state.srcBlendAlpha);
            desc.RenderTarget[0].DestBlendAlpha = translate(state.destBlendAlpha);
            desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

            HRESULT hr = DirectXStateRegistry::getRenderer()->getDevice()->CreateBlendState(&desc, reinterpret_cast<ID3D11BlendState**>(value.getInitRef()));
            TB::runtimeCheck(hr == S_OK);
            DirectXStateRegistry::registerState<DirectXBlendState>(key, value);
        }
        return (ID3D11BlendState*)(ID3D11DeviceChild*)value;
    }
}

#endif
