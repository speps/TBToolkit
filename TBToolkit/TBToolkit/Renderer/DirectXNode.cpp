#include <TBToolkit/Platform.h>
#if TB_PLATFORM_WINDOWS

#include <map>
#include <TBToolkit/Renderer/DirectXRenderer.h>
#include <TBToolkit/Renderer/DirectXNode.h>
#include <d3d11.h>
#include <directxmath.h>
#include <OpenGEX.h>

namespace TB
{
    DirectXNode::DirectXNode(const std::shared_ptr<class DirectXRenderer>& renderer, const OGEX::NodeStructure& node)
        : mRenderer(renderer)
    {
        mObjectConstants.create(renderer);

        math::float4x4 global(math::float4x4::identity), local(math::float4x4::identity);

        for (const auto* structure = node.GetFirstSubnode(); structure != nullptr; structure = structure->Next())
        {
            if (structure->GetStructureType() == OGEX::kStructureTransform)
            {
                const auto& transform = static_cast<const OGEX::TransformStructure&>(*structure);
                const float* values = transform.GetTransform();
                math::float4x4 matrix(values[0], values[1], values[2], values[3], values[4], values[5], values[6], values[7],
                    values[8], values[9], values[10], values[11], values[12], values[13], values[14], values[15]);
                matrix.Transpose();

                if (!transform.GetObjectFlag())
                {
                    global = matrix;
                }
                else
                {
                    local = matrix;
                }
            }
            else if (structure->GetStructureType() == OGEX::kStructureObjectRef)
            {
                const auto& objectRef = static_cast<const OGEX::ObjectRefStructure&>(*structure);
                mObjectRef = objectRef.GetTargetStructure()->GetStructureName();
            }
        }

        global = local * global;
        global.Decompose(mPosition, mRotation, mScale);

        updateConstants();
    }

    DirectXNode::~DirectXNode()
    {
    }

    void DirectXNode::updateConstants()
    {
        auto xmPos = DirectX::XMLoadFloat3((DirectX::XMFLOAT3*)&mPosition);
        auto xmRot = DirectX::XMLoadFloat4((DirectX::XMFLOAT4*)&mRotation);
        auto xmScale = DirectX::XMLoadFloat3((DirectX::XMFLOAT3*)&mScale);

        DirectXObjectConstants objectConstants;
        objectConstants.localToWorld = DirectX::XMMatrixTranspose(DirectX::XMMatrixAffineTransformation(xmScale, DirectX::XMVectorZero(), xmRot, xmPos));

        mObjectConstants.update(objectConstants);
    }

    void DirectXNode::set(const math::float3& pos, const math::Quat& rot, const math::float3& scale)
    {
        mPosition = pos;
        mRotation = rot;
        mScale = scale;
        updateConstants();
    }

    void DirectXNode::render(const std::shared_ptr<class Model>& model) const
    {
        ID3D11Buffer* constants[] = { mObjectConstants };

        auto imc = mRenderer->getImmediateContext();
        imc->VSSetConstantBuffers(1, 1, constants);
        imc->PSSetConstantBuffers(1, 1, constants);

        model->render();
    }
}

#endif
