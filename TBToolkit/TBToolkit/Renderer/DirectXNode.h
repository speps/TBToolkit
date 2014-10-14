#pragma once
#include <TBToolkit/Platform.h>
#if TB_PLATFORM_WINDOWS

#include <TBToolkit/Common.h>
#include <TBToolkit/Geometry.h>
#include <TBToolkit/Renderer/DirectXConstants.h>

namespace OGEX
{
    class NodeStructure;
}

struct ID3D11Buffer;

namespace TB
{
    class DirectXNode : public Node
    {
    public:
        DirectXNode(const std::shared_ptr<class DirectXRenderer>& renderer, const OGEX::NodeStructure& node);
        virtual ~DirectXNode();

        virtual const std::string& getObjectRef() const override { return mObjectRef; }
        virtual void set(const math::float3& pos, const math::Quat& rot, const math::float3& scale) override;

        virtual const math::float3& getPos() const override { return mPosition; }
        virtual const math::Quat& getRot() const override { return mRotation; }
        virtual const math::float3& getScale() const override { return mScale; }

        virtual void render(const std::shared_ptr<class Model>& model) const override;

    private:
        void updateConstants();

        std::shared_ptr<class DirectXRenderer> mRenderer;
        DirectXConstants<DirectXObjectConstants> mObjectConstants;

        math::float3 mPosition;
        math::Quat mRotation;
        math::float3 mScale;

        std::string mObjectRef;
    };
}

#endif
