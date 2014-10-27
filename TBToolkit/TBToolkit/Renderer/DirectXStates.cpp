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

    D3D11_COMPARISON_FUNC translate(TB::CompFunc func)
    {
        switch (func)
        {
        case TB::CompFunc::Never: return D3D11_COMPARISON_NEVER;
        case TB::CompFunc::Less: return D3D11_COMPARISON_LESS;
        case TB::CompFunc::Equal: return D3D11_COMPARISON_EQUAL;
        case TB::CompFunc::LessEqual: return D3D11_COMPARISON_LESS_EQUAL;
        case TB::CompFunc::Greater: return D3D11_COMPARISON_GREATER;
        case TB::CompFunc::NotEqual: return D3D11_COMPARISON_NOT_EQUAL;
        case TB::CompFunc::GreaterEqual: return D3D11_COMPARISON_GREATER_EQUAL;
        case TB::CompFunc::Always: return D3D11_COMPARISON_ALWAYS;
        }
        return (D3D11_COMPARISON_FUNC)0;
    }

    D3D11_FILTER translate(TB::FilterMode filter)
    {
        switch(filter)
        {
        case TB::FilterMode::MinMagMipPoint: return D3D11_FILTER_MIN_MAG_MIP_POINT;
        case TB::FilterMode::MinMagPointMipLinear: return D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR;
        case TB::FilterMode::MinPointMagLinearMipPoint: return D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
        case TB::FilterMode::MinPointMagMipLinear: return D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR;
        case TB::FilterMode::MinLinearMagMipPoint: return D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT;
        case TB::FilterMode::MinLinearMagPointMipLinear: return D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
        case TB::FilterMode::MinMagLinearMipPoint: return D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
        case TB::FilterMode::MinMagMipLinear: return D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        case TB::FilterMode::Anisotropic: return D3D11_FILTER_ANISOTROPIC;
        case TB::FilterMode::ComparisonMinMagMipPoint: return D3D11_FILTER_COMPARISON_MIN_MAG_MIP_POINT;
        case TB::FilterMode::ComparisonMinMagPointMipLinear: return D3D11_FILTER_COMPARISON_MIN_MAG_POINT_MIP_LINEAR;
        case TB::FilterMode::ComparisonMinPointMagLinearMipPoint: return D3D11_FILTER_COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT;
        case TB::FilterMode::ComparisonMinPointMagMipLinear: return D3D11_FILTER_COMPARISON_MIN_POINT_MAG_MIP_LINEAR;
        case TB::FilterMode::ComparisonMinLinearMagMipPoint: return D3D11_FILTER_COMPARISON_MIN_LINEAR_MAG_MIP_POINT;
        case TB::FilterMode::ComparisonMinLinearMagPointMipLinear: return D3D11_FILTER_COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
        case TB::FilterMode::ComparisonMinMagLinearMipPoint: return D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
        case TB::FilterMode::ComparisonMinMagMipLinear: return D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
        case TB::FilterMode::ComparisonAnisotropic: return D3D11_FILTER_COMPARISON_ANISOTROPIC;
        case TB::FilterMode::MinimumMinMagMipPoint: return D3D11_FILTER_MINIMUM_MIN_MAG_MIP_POINT;
        case TB::FilterMode::MinimumMinMagPointMipLinear: return D3D11_FILTER_MINIMUM_MIN_MAG_POINT_MIP_LINEAR;
        case TB::FilterMode::MinimumMinPointMagLinearMipPoint: return D3D11_FILTER_MINIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT;
        case TB::FilterMode::MinimumMinPointMagMipLinear: return D3D11_FILTER_MINIMUM_MIN_POINT_MAG_MIP_LINEAR;
        case TB::FilterMode::MinimumMinLinearMagMipPoint: return D3D11_FILTER_MINIMUM_MIN_LINEAR_MAG_MIP_POINT;
        case TB::FilterMode::MinimumMinLinearMagPointMipLinear: return D3D11_FILTER_MINIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
        case TB::FilterMode::MinimumMinMagLinearMipPoint: return D3D11_FILTER_MINIMUM_MIN_MAG_LINEAR_MIP_POINT;
        case TB::FilterMode::MinimumMinMagMipLinear: return D3D11_FILTER_MINIMUM_MIN_MAG_MIP_LINEAR;
        case TB::FilterMode::MinimumAnisotropic: return D3D11_FILTER_MINIMUM_ANISOTROPIC;
        case TB::FilterMode::MaximumMinMagMipPoint: return D3D11_FILTER_MAXIMUM_MIN_MAG_MIP_POINT;
        case TB::FilterMode::MaximumMinMagPointMipLinear: return D3D11_FILTER_MAXIMUM_MIN_MAG_POINT_MIP_LINEAR;
        case TB::FilterMode::MaximumMinPointMagLinearMipPoint: return D3D11_FILTER_MAXIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT;
        case TB::FilterMode::MaximumMinPointMagMipLinear: return D3D11_FILTER_MAXIMUM_MIN_POINT_MAG_MIP_LINEAR;
        case TB::FilterMode::MaximumMinLinearMagMipPoint: return D3D11_FILTER_MAXIMUM_MIN_LINEAR_MAG_MIP_POINT;
        case TB::FilterMode::MaximumMinLinearMagPointMipLinear: return D3D11_FILTER_MAXIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
        case TB::FilterMode::MaximumMinMagLinearMipPoint: return D3D11_FILTER_MAXIMUM_MIN_MAG_LINEAR_MIP_POINT;
        case TB::FilterMode::MaximumMinMagMipLinear: return D3D11_FILTER_MAXIMUM_MIN_MAG_MIP_LINEAR;
        case TB::FilterMode::MaximumAnisotropic: return D3D11_FILTER_MAXIMUM_ANISOTROPIC;
        }
        return (D3D11_FILTER)0;
    }

    D3D11_TEXTURE_ADDRESS_MODE translate(TB::AddressMode addr)
    {
        switch (addr)
        {
        case TB::AddressMode::Wrap: return D3D11_TEXTURE_ADDRESS_WRAP;
        case TB::AddressMode::Mirror: return D3D11_TEXTURE_ADDRESS_MIRROR;
        case TB::AddressMode::Clamp: return D3D11_TEXTURE_ADDRESS_CLAMP;
        case TB::AddressMode::Border: return D3D11_TEXTURE_ADDRESS_BORDER;
        case TB::AddressMode::MirrorOnce: return D3D11_TEXTURE_ADDRESS_MIRROR_ONCE;
        }
        return (D3D11_TEXTURE_ADDRESS_MODE)0;
    }

    D3D11_FILL_MODE translate(TB::FillMode fill)
    {
        switch (fill)
        {
        case TB::FillMode::Wireframe: return D3D11_FILL_WIREFRAME;
        case TB::FillMode::Solid: return D3D11_FILL_SOLID;
        }
        return (D3D11_FILL_MODE)0;
    }

    D3D11_CULL_MODE translate(TB::CullMode cull)
    {
        switch (cull)
        {
        case TB::CullMode::None: return D3D11_CULL_NONE;
        case TB::CullMode::Front: return D3D11_CULL_FRONT;
        case TB::CullMode::Back: return D3D11_CULL_BACK;
        }
        return (D3D11_CULL_MODE)0;
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
            D3D11_BLEND_DESC desc{};
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

    const char* TB::TypeTraits<DirectXDepthStencilState>::Name = "DirectXDepthStencilState";

    uint32_t DirectXDepthStencilState::State::key() const
    {
        return hash((uint8_t*)this, sizeof(State));
    }

    ID3D11DepthStencilState* DirectXDepthStencilState::getOrCreate(const State& state)
    {
        auto key = state.key();
        auto value = DirectXStateRegistry::get<DirectXDepthStencilState>(key);
        if (!value.isValid())
        {
            D3D11_DEPTH_STENCIL_DESC desc{};
            desc.DepthEnable = state.depthEnable;
            desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
            desc.DepthFunc = translate(state.depthFunc);
            desc.StencilEnable = false;
            desc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
            desc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
            desc.FrontFace = { D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_COMPARISON_ALWAYS };
            desc.BackFace = { D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_COMPARISON_ALWAYS };

            HRESULT hr = DirectXStateRegistry::getRenderer()->getDevice()->CreateDepthStencilState(&desc, reinterpret_cast<ID3D11DepthStencilState**>(value.getInitRef()));
            TB::runtimeCheck(hr == S_OK);
            DirectXStateRegistry::registerState<DirectXDepthStencilState>(key, value);
        }
        return (ID3D11DepthStencilState*)(ID3D11DeviceChild*)value;
    }

    const char* TB::TypeTraits<DirectXSamplerState>::Name = "DirectXSamplerState";

    uint32_t DirectXSamplerState::State::key() const
    {
        return hash((uint8_t*)this, sizeof(State));
    }

    ID3D11SamplerState* DirectXSamplerState::getOrCreate(const State& state)
    {
        auto key = state.key();
        auto value = DirectXStateRegistry::get<DirectXSamplerState>(key);
        if (!value.isValid())
        {
            D3D11_SAMPLER_DESC desc{};
            desc.Filter = translate(state.filter);
            desc.AddressU = translate(state.addrU);
            desc.AddressV = translate(state.addrV);
            desc.AddressW = translate(state.addrW);
            desc.MinLOD = -FLT_MAX;
            desc.MaxLOD = FLT_MAX;
            desc.MipLODBias = 0.0f;
            desc.MaxAnisotropy = 1;
            desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
            desc.BorderColor[0] = 1.0f;
            desc.BorderColor[1] = 1.0f;
            desc.BorderColor[2] = 1.0f;
            desc.BorderColor[3] = 1.0f;

            HRESULT hr = DirectXStateRegistry::getRenderer()->getDevice()->CreateSamplerState(&desc, reinterpret_cast<ID3D11SamplerState**>(value.getInitRef()));
            TB::runtimeCheck(hr == S_OK);
            DirectXStateRegistry::registerState<DirectXSamplerState>(key, value);
        }
        return (ID3D11SamplerState*)(ID3D11DeviceChild*)value;
    }

    const char* TB::TypeTraits<DirectXRasterizerState>::Name = "DirectXRasterizerState";

    uint32_t DirectXRasterizerState::State::key() const
    {
        return hash((uint8_t*)this, sizeof(State));
    }

    ID3D11RasterizerState* DirectXRasterizerState::getOrCreate(const State& state)
    {
        auto key = state.key();
        auto value = DirectXStateRegistry::get<DirectXRasterizerState>(key);
        if (!value.isValid())
        {
            D3D11_RASTERIZER_DESC desc{};
            desc.FillMode = translate(state.fill);
            desc.CullMode = translate(state.cull);
            desc.FrontCounterClockwise = FALSE;
            desc.DepthBias = 0;
            desc.SlopeScaledDepthBias = 0.0f;
            desc.DepthBiasClamp = 0.0f;
            desc.DepthClipEnable = TRUE;
            desc.ScissorEnable = FALSE;
            desc.MultisampleEnable = FALSE;
            desc.AntialiasedLineEnable = FALSE;

            HRESULT hr = DirectXStateRegistry::getRenderer()->getDevice()->CreateRasterizerState(&desc, reinterpret_cast<ID3D11RasterizerState**>(value.getInitRef()));
            TB::runtimeCheck(hr == S_OK);
            DirectXStateRegistry::registerState<DirectXRasterizerState>(key, value);
        }
        return (ID3D11RasterizerState*)(ID3D11DeviceChild*)value;
    }
}

#endif
