#include <TBToolkit/Platform.h>
#include <TBToolkit/Geometry.h>
#include <OpenGEX.h>
#include <memory>

#if TB_PLATFORM_WINDOWS
#include <TBToolkit/Renderer/DirectXRenderer.h>
#include <TBToolkit/Renderer/DirectXModel.h>
#include <TBToolkit/Renderer/DirectXNode.h>
#endif

namespace
{
    std::shared_ptr<TB::Model> createModel(const std::shared_ptr<class TB::Renderer>& renderer, const OGEX::GeometryObjectStructure& geometryObject)
    {
#if TB_PLATFORM_WINDOWS
        auto directXRenderer = std::dynamic_pointer_cast<TB::DirectXRenderer>(renderer);
        if (directXRenderer)
        {
            return std::make_shared<TB::DirectXModel>(directXRenderer, geometryObject);
        }
#endif
        return nullptr;
    }

    std::shared_ptr<TB::Node> createNode(const std::shared_ptr<class TB::Renderer>& renderer, const OGEX::NodeStructure& node)
    {
#if TB_PLATFORM_WINDOWS
        auto directXRenderer = std::dynamic_pointer_cast<TB::DirectXRenderer>(renderer);
        if (directXRenderer)
        {
            return std::make_shared<TB::DirectXNode>(directXRenderer, node);
        }
#endif
        return nullptr;
    }
}

namespace TB
{
    Scene::Scene(const std::shared_ptr<class Renderer>& renderer, const OGEX::OpenGexDataDescription& dataDesc)
    {
        for (const auto* structure = dataDesc.GetRootStructure()->GetFirstSubnode(); structure != nullptr; structure = structure->Next())
        {
            if (structure->GetStructureType() == OGEX::kStructureGeometryObject)
            {
                mModels.emplace(structure->GetStructureName(), createModel(renderer, static_cast<const OGEX::GeometryObjectStructure&>(*structure)));
            }
            if (structure->GetBaseStructureType() == OGEX::kStructureNode)
            {
                mNodes.emplace(structure->GetStructureName(), createNode(renderer, static_cast<const OGEX::NodeStructure&>(*structure)));
            }
        }
    }

    void Scene::render() const
    {
        for (const auto& node : mNodes)
        {
            auto model = mModels.find(node.second->getObjectRef());
            if (model != mModels.end())
            {
                node.second->render(model->second);
            }
        }
    }

    std::shared_ptr<Node> Scene::getNode(const std::string& name) const
    {
        auto item = mNodes.find(name);
        if (item != mNodes.end())
        {
            return item->second;
        }
        return nullptr;
    }
}
