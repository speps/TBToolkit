#include <TBToolkit/Platform.h>
#if TB_PLATFORM_WINDOWS

#include <map>
#include <TBToolkit/Common.h>
#include <TBToolkit/Renderer/DirectXRenderer.h>
#include <TBToolkit/Renderer/DirectXModel.h>
#include <d3d11.h>
#include <directxmath.h>
#include <OpenGEX.h>

namespace TB
{
    DirectXModel::DirectXModel(const std::shared_ptr<DirectXRenderer>& renderer)
        : mRenderer(renderer)
    {
    }

    DirectXModel::DirectXModel(const std::shared_ptr<DirectXRenderer>& renderer, const OGEX::GeometryObjectStructure& geometryObject)
        : mRenderer(renderer)
    {
        auto& meshMap = *geometryObject.GetMeshMap();
        for (int32_t i = 0; i < meshMap.GetElementCount(); i++)
        {
            auto& meshStructure = *meshMap[i];
            TB::runtimeCheck(meshStructure.GetMeshPrimitive() == "triangles");

            Mesh mesh;
            mesh.lodLevel = meshStructure.GetMeshLevel();
            {
                std::vector<VertexData> vertexData;
                for (auto subNode = meshStructure.GetFirstSubnode(); subNode != nullptr; subNode = subNode->Next())
                {
                    if (subNode->GetStructureType() == OGEX::kStructureVertexArray)
                    {
                        auto& vertexArrayStructure = static_cast<const OGEX::VertexArrayStructure&>(*subNode);
                        // Don't support morph targets yet.
                        TB::runtimeCheck(vertexArrayStructure.GetMorphIndex() == 0);

                        auto& dataStructure = static_cast<const ODDL::DataStructure<ODDL::FloatDataType>&>(*vertexArrayStructure.GetFirstSubnode());
                        int numVertices = dataStructure.GetDataElementCount() / dataStructure.GetArraySize();
                        if (vertexData.empty())
                        {
                            vertexData.resize(numVertices);
                        }
                        else
                        {
                            TB::runtimeCheck(numVertices == vertexData.size());
                        }

                        if (vertexArrayStructure.GetArrayAttrib() == "position")
                        {
                            TB::runtimeCheck(dataStructure.GetArraySize() == 3);
                            const float* data = &dataStructure.GetDataElement(0);
                            for (int i = 0; i < numVertices; i++)
                            {
                                vertexData[i].position.x = *data++;
                                vertexData[i].position.y = *data++;
                                vertexData[i].position.z = *data++;
                            }
                        }
                        else if (vertexArrayStructure.GetArrayAttrib() == "normal")
                        {
                            TB::runtimeCheck(dataStructure.GetArraySize() == 3);
                            const float* data = &dataStructure.GetDataElement(0);
                            for (int i = 0; i < numVertices; i++)
                            {
                                vertexData[i].normal.x = *data++;
                                vertexData[i].normal.y = *data++;
                                vertexData[i].normal.z = *data++;
                            }
                        }
                        else if (vertexArrayStructure.GetArrayAttrib() == "texcoord")
                        {
                            TB::runtimeCheck(dataStructure.GetArraySize() == 2);
                            const float* data = &dataStructure.GetDataElement(0);
                            for (int i = 0; i < numVertices; i++)
                            {
                                vertexData[i].texCoord.x = *data++;
                                vertexData[i].texCoord.y = 1.0f - (*data++);
                            }
                        }
                    }
                }

                TB::runtimeCheck(vertexData.size() > 0);

                std::vector<uint32_t> indices;
                for (auto subNode = meshStructure.GetFirstSubnode(); subNode != nullptr; subNode = subNode->Next())
                {
                    if (subNode->GetStructureType() == OGEX::kStructureIndexArray)
                    {
                        auto& indexArrayStructure = static_cast<const OGEX::IndexArrayStructure&>(*subNode);
                        auto& dataStructure = static_cast<const ODDL::DataStructure<ODDL::UnsignedInt32DataType>&>(*indexArrayStructure.GetFirstSubnode());

                        Part part;
                        part.indexStart = indices.size();
                        part.indexCount = dataStructure.GetDataElementCount();
                        part.materialIndex = indexArrayStructure.GetMaterialIndex();

                        const ODDL::unsigned_int32* data = &dataStructure.GetDataElement(0);
                        indices.reserve(dataStructure.GetDataElementCount());
                        for (int i = 0; i < dataStructure.GetDataElementCount(); i++)
                        {
                            indices.push_back(*data++);
                        }

                        mesh.parts.push_back(part);
                    }
                }

                setupBuffers(mesh, vertexData, indices);
            }
            mMeshes.push_back(mesh);
        }
    }

    DirectXModel::~DirectXModel()
    {
    }

    void DirectXModel::addMesh(const std::vector<VertexData>& vertices, const std::vector<uint32_t>& indices)
    {
        Mesh mesh;
        setupBuffers(mesh, vertices, indices);
        Part part = {0};
        part.indexCount = indices.size();
        mesh.parts.push_back(part);
        mMeshes.push_back(mesh);
    }

    void DirectXModel::render() const
    {
        auto imc = mRenderer->getImmediateContext();
        for (size_t i = 0; i < mMeshes.size(); i++)
        {
            const auto& mesh = mMeshes[i];

            const UINT stride[] = { sizeof(VertexData) };
            const UINT offset[] = { 0 };
            ID3D11Buffer* buffers[] = { mesh.vertexBuffer };
            imc->IASetVertexBuffers(0, 1, buffers, stride, offset);
            imc->IASetIndexBuffer(mesh.indexBuffer, DXGI_FORMAT_R32_UINT, 0);

            for (size_t j = 0; j < mesh.parts.size(); j++)
            {
                const auto& part = mesh.parts[j];
                imc->DrawIndexed((UINT)part.indexCount, (UINT)part.indexStart, 0);
            }
        }
    }

    void DirectXModel::setupBuffers(Mesh& mesh, const std::vector<VertexData>& vertices, const std::vector<uint32_t>& indices)
    {
        // Vertex buffer
        {
            D3D11_BUFFER_DESC desc = {0};
            desc.Usage = D3D11_USAGE_DEFAULT;
            desc.ByteWidth = UINT(sizeof(VertexData) * vertices.size());
            desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

            D3D11_SUBRESOURCE_DATA initData = {0};
            initData.pSysMem = &vertices[0];

            HRESULT hr = mRenderer->getDevice()->CreateBuffer(&desc, &initData, mesh.vertexBuffer.getInitRef());
            TB::runtimeCheck(hr == S_OK);
        }

        // Index buffer
        {
            D3D11_BUFFER_DESC desc = {0};
            desc.Usage = D3D11_USAGE_DEFAULT;
            desc.ByteWidth = UINT(sizeof(uint32_t) * indices.size());
            desc.BindFlags = D3D11_BIND_INDEX_BUFFER;

            D3D11_SUBRESOURCE_DATA initData = {0};
            initData.pSysMem = &indices[0];

            HRESULT hr = mRenderer->getDevice()->CreateBuffer(&desc, &initData, mesh.indexBuffer.getInitRef());
            TB::runtimeCheck(hr == S_OK);
        }
    }
}

#endif
