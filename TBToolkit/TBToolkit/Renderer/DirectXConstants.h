#pragma once
#include <TBToolkit/Platform.h>
#if TB_PLATFORM_WINDOWS

#include <TBToolkit/Renderer/DirectXRenderer.h>
#include <d3d11.h>

namespace TB
{
    template<typename ConstantType>
    class DirectXConstants
    {
    public:
        DirectXConstants() : mCreated(false) {}
        virtual ~DirectXConstants() {}

        operator ID3D11Buffer*() const
        {
            return mBuffer;
        }

        void create(const std::shared_ptr<class DirectXRenderer>& renderer)
        {
            mRenderer = renderer;

            if (mRenderer && !mCreated)
            {
                D3D11_BUFFER_DESC desc = {0};
                desc.Usage = D3D11_USAGE_DYNAMIC;
                desc.ByteWidth = UINT(sizeof(ConstantType));
                desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
                desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

                HRESULT hr = mRenderer->getDevice()->CreateBuffer(&desc, nullptr, mBuffer.getInitRef());
                TB::runtimeCheck(hr == S_OK);

                mCreated = true;
            }
        }

        void update(const ConstantType& value)
        {
            if (mRenderer)
            {
                auto imc = mRenderer->getImmediateContext();
                D3D11_MAPPED_SUBRESOURCE resourceData;
                imc->Map(mBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &resourceData);
                memcpy(resourceData.pData, &value, sizeof(ConstantType));
                imc->Unmap(mBuffer, 0);
            }
        }

    private:
        bool mCreated;
        std::shared_ptr<DirectXRenderer> mRenderer;
        ComPtr<ID3D11Buffer> mBuffer;
    };
}

#endif
