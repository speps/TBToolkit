#include <TBToolkit/Platform.h>
#if TB_PLATFORM_WINDOWS

#include <TBToolkit/Common.h>
#include <TBToolkit/Renderer/DirectXRenderer.h>
#include <TBToolkit/Renderer/DirectXModel.h>
#include <TBToolkit/Renderer/DirectXShader.h>
#include <TBToolkit/Renderer/DirectXInputLayout.h>
#include <d3d11.h>
#include <directxmath.h>
#include <OpenGEX.h>
#include <map>
#include <sstream>

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
                Vertices vertices;
                for (auto subNode = meshStructure.GetFirstSubnode(); subNode != nullptr; subNode = subNode->Next())
                {
                    if (subNode->GetStructureType() == OGEX::kStructureVertexArray)
                    {
                        auto& vertexArrayStructure = static_cast<const OGEX::VertexArrayStructure&>(*subNode);
                        // Don't support morph targets yet.
                        TB::runtimeCheck(vertexArrayStructure.GetMorphIndex() == 0);

                        auto& dataStructure = static_cast<const ODDL::DataStructure<ODDL::FloatDataType>&>(*vertexArrayStructure.GetFirstSubnode());
                        int numVertices = dataStructure.GetDataElementCount() / dataStructure.GetArraySize();

                        if (vertexArrayStructure.GetArrayAttrib() == "position")
                        {
                            TB::runtimeCheck(dataStructure.GetArraySize() == 3);
                            const float* data = &dataStructure.GetDataElement(0);
                            VertexStream stream;
                            stream.semantic = VertexSemantic::Position;
                            stream.usageIndex = 0;
                            stream.elements = 3;
                            stream.data.resize(numVertices * stream.elements);
                            memcpy(&stream.data[0], data, stream.data.size() * sizeof(float));
                            vertices.push_back(stream);
                        }
                        else if (vertexArrayStructure.GetArrayAttrib() == "normal")
                        {
                            TB::runtimeCheck(dataStructure.GetArraySize() == 3);
                            const float* data = &dataStructure.GetDataElement(0);
                            VertexStream stream;
                            stream.semantic = VertexSemantic::Normal;
                            stream.usageIndex = 0;
                            stream.elements = 3;
                            stream.data.resize(numVertices * stream.elements);
                            memcpy(&stream.data[0], data, stream.data.size() * sizeof(float));
                            vertices.push_back(stream);
                        }
                        else if (vertexArrayStructure.GetArrayAttrib() == "texcoord")
                        {
                            TB::runtimeCheck(dataStructure.GetArraySize() == 2);
                            const float* data = &dataStructure.GetDataElement(0);
                            VertexStream stream;
                            stream.semantic = VertexSemantic::TexCoord;
                            stream.usageIndex = 0;
                            stream.elements = 2;
                            stream.data.resize(numVertices * stream.elements);
                            for (int i = 0; i < numVertices; i++)
                            {
                                stream.data[i * stream.elements + 0] = *data++;
                                stream.data[i * stream.elements + 1] = 1.0f - *data++;
                            }
                            vertices.push_back(stream);
                        }
                    }
                }

                TB::runtimeCheck(vertices.size() > 0);

                Indices indices;
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

                setupBuffers(mesh, vertices, indices);
            }
            mMeshes.push_back(mesh);
        }
    }

    DirectXModel::~DirectXModel()
    {
    }

    void DirectXModel::addMesh(const Vertices& vertices, const Indices& indices)
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

            imc->IASetInputLayout(DirectXInputLayoutRegistry::get(mesh.inputLayoutID));
            imc->IASetVertexBuffers(0, (UINT)mesh.vertexBuffers.size(), &mesh.transientBuffers[0], &mesh.transientStrides[0], &mesh.transientOffsets[0]);
            imc->IASetIndexBuffer(mesh.indexBuffer, DXGI_FORMAT_R32_UINT, 0);

            for (size_t j = 0; j < mesh.parts.size(); j++)
            {
                const auto& part = mesh.parts[j];
                imc->DrawIndexed((UINT)part.indexCount, (UINT)part.indexStart, 0);
            }
        }
    }

    void DirectXModel::setupBuffers(Mesh& mesh, const Vertices& vertices, const Indices& indices)
    {
        // Vertex buffers
        mesh.vertexBuffers.resize(vertices.size());
        for (size_t i = 0; i < vertices.size(); i++)
        {
            const auto& stream = vertices[i];

            D3D11_BUFFER_DESC desc = {0};
            desc.Usage = D3D11_USAGE_DEFAULT;
            desc.ByteWidth = UINT(stream.data.size() * sizeof(float));
            desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

            D3D11_SUBRESOURCE_DATA initData = {0};
            initData.pSysMem = &stream.data[0];

            HRESULT hr = mRenderer->getDevice()->CreateBuffer(&desc, &initData, mesh.vertexBuffers[i].getInitRef());
            TB::runtimeCheck(hr == S_OK);

            mesh.transientBuffers[i] = mesh.vertexBuffers[i];
            mesh.transientStrides[i] = stream.elements * sizeof(float);
            mesh.transientOffsets[i] = 0;
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

        mesh.inputLayoutID = DirectXInputLayoutRegistry::create(vertices);
    }
}

#endif
