#pragma once
#include <TBToolkit/Platform.h>
#if TB_PLATFORM_WINDOWS

#include <TBToolkit/Common.h>
#include <TBToolkit/Geometry.h>
#include <memory>

struct ID3D11Buffer;

namespace OGEX
{
    class GeometryObjectStructure;
}

namespace TB
{
    class DirectXModel : public Model
    {
    public:
        DirectXModel(const std::shared_ptr<class DirectXRenderer>& renderer, const OGEX::GeometryObjectStructure& geometryObject);
        virtual ~DirectXModel();

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
            ComPtr<ID3D11Buffer> vertexBuffer;
            ComPtr<ID3D11Buffer> indexBuffer;
            Parts parts;
            uint32_t lodLevel;
        };
        typedef std::vector<Mesh> Meshes;

        std::shared_ptr<class DirectXRenderer> mRenderer;
        Meshes mMeshes;
    };
}

#endif
