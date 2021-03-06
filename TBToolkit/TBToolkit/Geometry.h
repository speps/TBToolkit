#pragma once

#include <MathGeoLib.h>
#include <vector>
#include <memory>
#include <map>
#include <string>
#include <functional>

namespace OGEX
{
    class NodeStructure;
    class GeometryObjectStructure;
    class OpenGexDataDescription;
}

namespace TB
{
    enum class VertexSemantic
    {
        Position,
        Normal,
        TexCoord,
        Color
    };

    struct VertexStream
    {
        VertexSemantic semantic;
        int usageIndex;
        std::vector<float> data;
        int elements;
    };

    typedef std::vector<VertexStream> Vertices;
    typedef std::vector<uint32_t> Indices;
    typedef std::function<void(Vertices& vertices, Indices& indices)> MeshModifierCallback;

    class Renderable
    {
    public:
        virtual void render() const = 0;
    };

    class Model : public Renderable
    {
    public:
        Model() {}
        virtual ~Model() {}

        virtual void addMesh(const Vertices& vertices, const Indices& indices) = 0;
        virtual void render() const = 0;
    };

    class Node
    {
    public:
        Node() {}
        virtual ~Node() {}

        virtual const std::string& getObjectRef() const = 0;
        virtual void set(const math::float3& pos, const math::Quat& rot, const math::float3& scale) = 0;

        virtual const math::float3& getPos() const = 0;
        virtual const math::Quat& getRot() const = 0;
        virtual const math::float3& getScale() const = 0;

        virtual void render(const std::shared_ptr<class Model>& model) const = 0;
    };

    class Scene : public Renderable
    {
    public:
        Scene(const std::shared_ptr<class Renderer>& renderer, const OGEX::OpenGexDataDescription& dataDesc, const MeshModifierCallback& meshModifier);
        virtual ~Scene() {}

        virtual void render() const override;

        std::shared_ptr<Node> getNode(const std::string& name) const;

    private:
        typedef std::map<std::string, std::shared_ptr<Model>> Models;
        typedef std::map<std::string, std::shared_ptr<Node>> Nodes;

        Models mModels;
        Nodes mNodes;
    };
}
