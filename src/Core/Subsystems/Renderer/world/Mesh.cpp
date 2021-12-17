#include "./Mesh.h"

#include <sstream>

Mesh::Mesh(const int idx, const tinygltf::Model& model, std::vector<uint16_t>& indicesBuffer, std::vector<float>& vertexBuffer, std::vector<Primitive>& primitives)
{
    for (const auto& primitiveData : model.meshes[idx].primitives)
    {
        const Buffer<uint16_t> primIndicesBuffer = getBuffer<uint16_t>(primitiveData.indices, model);
        const Buffer<float> primVertexBuffer = getBuffer<float>(primitiveData.attributes.at("POSITION"), model);
        const Buffer<float> primNormalBuffer = getBuffer<float>(primitiveData.attributes.at("NORMAL"), model);

        Primitive primitive;

        const auto material = model.materials[primitiveData.material];
        std::stringstream s;
        s << "TEXCOORD_" << material.pbrMetallicRoughness.baseColorTexture.texCoord;
        const Buffer<float> primTexCoordBuffer = getBuffer<float>(primitiveData.attributes.at(s.str()), model);

        int t = 0;
        for (int i = 0; i < primVertexBuffer.size; i += 3)
        {
            const Vertex v =
            {
                .position = {primVertexBuffer.buffer[i], primVertexBuffer.buffer[i+1], primVertexBuffer.buffer[i+2]},
                .normal = {primNormalBuffer.buffer[i],primNormalBuffer.buffer[i+1],primNormalBuffer.buffer[i+2]},
                .texCoords = {primTexCoordBuffer.buffer[t],primTexCoordBuffer.buffer[t+1]},
            };
            primitive.vertices.emplace_back(v);
            t += 2;
        }

        for (int i = 0; i < primIndicesBuffer.size; ++i)
        {
            primitive.indices.emplace_back(primIndicesBuffer.buffer[i]);
        }

        glGenVertexArrays(1, &primitive.VAO);
        glGenBuffers(1, &primitive.VBO);
        glGenBuffers(1, &primitive.EBO);

        glBindVertexArray(primitive.VAO);

        glBindBuffer(GL_ARRAY_BUFFER, primitive.VBO);
        glBufferData(GL_ARRAY_BUFFER, primitive.vertices.size() * sizeof(Vertex), primitive.vertices.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, primitive.EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, primitive.indices.size() * sizeof(GLuint), primitive.indices.data(), GL_STATIC_DRAW);

        // vertex positions
        glEnableVertexAttribArray(0);	
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
        // vertex normals
        glEnableVertexAttribArray(1);	
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
        // vertex texture coords
        glEnableVertexAttribArray(2);	
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));

        glBindVertexArray(0);

        primitive.material.textureID = model.materials[primitiveData.material].pbrMetallicRoughness.baseColorTexture.index;

        _primitives.emplace_back(primitive);
    }
}

const std::vector<Primitive>& Mesh::getPrimitives() const
{
    return _primitives;
}
