#pragma once
#include <TBToolkit/Platform.h>
#if TB_PLATFORM_WINDOWS

#include <TBToolkit/Common.h>
#include <TBToolkit/Renderer.h>
#include <map>
#include <memory>

struct ID3D11InputLayout;
struct D3D11_INPUT_ELEMENT_DESC;

namespace TB
{
    class DirectXInputLayoutRegistry
    {
    public:
        static void init(const std::shared_ptr<class DirectXRenderer>& renderer)
        {
            mRenderer = renderer;
        }

        static uint32_t create(const Vertices& vertices);
        static ComPtr<ID3D11InputLayout> get(uint32_t id);

        static void cleanup()
        {
            mRegistry.clear();
        }

    private:
        DirectXInputLayoutRegistry() {}
        ~DirectXInputLayoutRegistry() {}

        static std::shared_ptr<class DirectXRenderer> mRenderer;
        static std::map<uint32_t, ComPtr<ID3D11InputLayout>> mRegistry;
    };
}

#endif
