#pragma once
#include <TBToolkit/Platform.h>
#if TB_PLATFORM_WINDOWS

#include <TBToolkit/Common.h>
#include <TBToolkit/Geometry.h>
#include <memory>
#include <array>

struct ID3D11Buffer;
struct ID3D11InputLayout;

namespace OGEX
{
    class GeometryObjectStructure;
}

namespace TB
{
    class DirectXModel : public Model
    {
    public:
        DirectXModel(const std::shared_ptr<class DirectXRenderer>& renderer);
        DirectXModel(const std::shared_ptr<class DirectXRenderer>& renderer, const OGEX::GeometryObjectStructure& geometryObject);
        virtual ~DirectXModel();

        virtual void addMesh(const Vertices& vertices, const Indices& indices) override;
        virtual void render() const override;

    private:
        struct Part
        {
            size_t indexStart;
            size_t indexCount;
            size_t materialIndex;
        };
        typedef std::vector<Part> Parts;

        struct Mesh
        {
            std::vector<ComPtr<ID3D11Buffer>> vertexBuffers;
            ComPtr<ID3D11Buffer> indexBuffer;
            Parts parts;
            uint32_t lodLevel;

            std::array<ID3D11Buffer*, 32> transientBuffers;
            std::array<uint32_t, 32> transientStrides;
            std::array<uint32_t, 32> transientOffsets;

            uint32_t inputLayoutID;
        };
        typedef std::vector<Mesh> Meshes;

        std::shared_ptr<class DirectXRenderer> mRenderer;
        Meshes mMeshes;

        void setupBuffers(Mesh& mesh, const Vertices& vertices, const Indices& indices);
    };
}

#endif
