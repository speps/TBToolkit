#pragma once
#include <TBToolkit/Platform.h>
#if TB_PLATFORM_WINDOWS

#include <TBToolkit/Common.h>
#include <TBToolkit/Renderer.h>
#include <map>
#include <memory>

struct ID3D11DeviceChild;
struct ID3D11BlendState;
struct ID3D11DepthStencilState;

namespace TB
{
    class DirectXStateRegistry
    {
    public:
        static void init(const std::shared_ptr<class DirectXRenderer>& renderer)
        {
            mRenderer = renderer;
        }

        template<typename T>
        static void registerState(uint32_t key, const ComPtr<ID3D11DeviceChild>& state)
        {
            auto bucket = mRegistry.find(TypeTraits<T>::Name);
            if (bucket == mRegistry.end())
            {
                bucket = mRegistry.emplace(TypeTraits<T>::Name, std::map<uint32_t, ComPtr<ID3D11DeviceChild>>()).first;
            }
            bucket->second.emplace(key, state);
        }

        template<typename T>
        static ComPtr<ID3D11DeviceChild> get(uint32_t key)
        {
            auto bucket = mRegistry.find(TypeTraits<T>::Name);
            if (bucket == mRegistry.end())
            {
                return ComPtr<ID3D11DeviceChild>();
            }
            auto item = bucket->second.find(key);
            if (item == bucket->second.end())
            {
                return ComPtr<ID3D11DeviceChild>();
            }
            return item->second;
        }

        static const std::shared_ptr<class DirectXRenderer>& getRenderer()
        {
            return mRenderer;
        }

        static void cleanup()
        {
            mRegistry.clear();
        }

    private:
        DirectXStateRegistry() {}
        ~DirectXStateRegistry() {}

        static std::shared_ptr<class DirectXRenderer> mRenderer;
        static std::map<const char*, std::map<uint32_t, ComPtr<ID3D11DeviceChild>>> mRegistry;
    };

    class DirectXBlendState
    {
    private:
        struct State
        {
            bool enable;
            BlendOp op;
            BlendFactor srcBlend;
            BlendFactor destBlend;
            BlendOp opAlpha;
            BlendFactor srcBlendAlpha;
            BlendFactor destBlendAlpha;

            uint32_t key() const;
        };

        static ID3D11BlendState* getOrCreate(const State& state);

    public:
        template<bool enable = false, BlendOp op = BlendOp::Add, BlendFactor srcBlend = BlendFactor::One, BlendFactor destBlend = BlendFactor::Zero, BlendOp opAlpha = BlendOp::Add, BlendFactor srcBlendAlpha = BlendFactor::One, BlendFactor destBlendAlpha = BlendFactor::Zero>
        static ID3D11BlendState* get()
        {
            return getOrCreate({ enable, op, srcBlend, destBlend, opAlpha, srcBlendAlpha, destBlendAlpha });
        }
    };

    template<>
    struct TypeTraits<DirectXBlendState>
    {
        static const char* Name;
    };

    class DirectXDepthStencilState
    {
    private:
        struct State
        {
            bool depthEnable;
            CompFunc depthFunc;

            uint32_t key() const;
        };

        static ID3D11DepthStencilState* getOrCreate(const State& state);

    public:
        template<bool depthEnable = true, CompFunc depthFunc = CompFunc::Less>
        static ID3D11DepthStencilState* get()
        {
            return getOrCreate({ depthEnable, depthFunc });
        }
    };

    template<>
    struct TypeTraits<DirectXDepthStencilState>
    {
        static const char* Name;
    };
}

#endif
