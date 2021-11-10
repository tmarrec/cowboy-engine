#include "./Mesh.h"

Mesh::Mesh(const int idx, const tinygltf::Model& model, std::vector<uint16_t>& indicesBuffer, std::vector<float>& vertexBuffer, std::vector<Primitive>& primitives)
{
    for (const auto& primitiveData : model.meshes[idx].primitives)
    {
        const Buffer<uint16_t> primIndicesBuffer = getBuffer<uint16_t>(primitiveData.indices, model);
        const size_t oldSizeIndices = indicesBuffer.size();
        indicesBuffer.resize(indicesBuffer.size()+primIndicesBuffer.size);
        memcpy(&indicesBuffer[oldSizeIndices], primIndicesBuffer.buffer, primIndicesBuffer.size * sizeof(uint16_t));

        const Buffer<float> primVertexBuffer = getBuffer<float>(primitiveData.attributes.at("POSITION"), model);
        const size_t oldSizeVertex = vertexBuffer.size();
        vertexBuffer.resize(vertexBuffer.size()+primVertexBuffer.size);
        memcpy(&vertexBuffer[oldSizeVertex], primVertexBuffer.buffer, primVertexBuffer.size * sizeof(float));

        const Primitive primitive =
        {
            .firstIndex     = oldSizeIndices,
            .indexCount     = primIndicesBuffer.size,
            .vertexOffset   = oldSizeVertex,
        };

        _primitives.emplace_back(primitive);
    }
}

const std::vector<Primitive>& Mesh::getPrimitives() const
{
    return _primitives;
}
