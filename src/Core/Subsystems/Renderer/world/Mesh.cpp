#include "./Mesh.h"

#include <sstream>

Mesh::Mesh(const int idx, const tinygltf::Model& model, std::vector<uint16_t>& indicesBuffer, std::vector<float>& vertexBuffer, std::vector<Primitive>& primitives)
{
    for (const auto& pData : model.meshes[idx].primitives)
    {
        Primitive p;

        const auto& pIndicesBuffer   = getBuffer<uint16_t>(pData.indices, model);
        const auto& pVertexBuffer    = getBuffer<GLfloat>(pData.attributes.at("POSITION"), model);
        const auto& pNormalBuffer    = getBuffer<GLfloat>(pData.attributes.at("NORMAL"), model);
        const auto& pTexCoordBuffer  = [&]()
        {
            // If has material
            if (pData.material >= 0)
            {
                const auto material = model.materials[pData.material];
                std::stringstream s;
                s << "TEXCOORD_" << material.pbrMetallicRoughness.baseColorTexture.texCoord;
                // If has texture
                if (pData.attributes.find(s.str()) != pData.attributes.end())
                {
                    p.material.hasTexture = true;
                    p.material.textureID = model.materials[pData.material].pbrMetallicRoughness.baseColorTexture.index;
                    return getBuffer<GLfloat>(pData.attributes.at(s.str()), model);
                }
            }
            // If no texture, zeros for all texcoord
            p.material.hasTexture = false;
            std::vector<GLfloat> buffer((pVertexBuffer.size / 3) * 2, 0);
            return Buffer<GLfloat>
            {
                .buffer = reinterpret_cast<const GLfloat*>(buffer.data()),
                .size = (pVertexBuffer.size / 3) * 2,
            };
        }();

        size_t t = 0;
        for (size_t i = 0; i < pVertexBuffer.size; i += 3)
        {
            const Vertex v =
            {
                .position = {pVertexBuffer.buffer[i], pVertexBuffer.buffer[i+1], pVertexBuffer.buffer[i+2]},
                .normal = {pNormalBuffer.buffer[i],pNormalBuffer.buffer[i+1],pNormalBuffer.buffer[i+2]},
                .texCoords = {pTexCoordBuffer.buffer[t],pTexCoordBuffer.buffer[t+1]},
            };
            p.vertices.emplace_back(v);
            t += 2;
        }

        p.indices = std::vector<GLuint>(pIndicesBuffer.buffer, pIndicesBuffer.buffer+pIndicesBuffer.size);

        /*
        for (size_t i = 0; i < pIndicesBuffer.size; ++i)
            p.indices.emplace_back(pIndicesBuffer.buffer[i]);
        */


        glGenVertexArrays(1, &p.VAO);
        glGenBuffers(1, &p.VBO);
        glGenBuffers(1, &p.EBO);

        glBindVertexArray(p.VAO);

        glBindBuffer(GL_ARRAY_BUFFER, p.VBO);
        glBufferData(GL_ARRAY_BUFFER, p.vertices.size() * sizeof(Vertex), p.vertices.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, p.EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, p.indices.size() * sizeof(GLuint), p.indices.data(), GL_STATIC_DRAW);

        // Vertex positions
        glEnableVertexAttribArray(0);	
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
        // Vertex normals
        glEnableVertexAttribArray(1);	
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
        // Vertex texture coords
        glEnableVertexAttribArray(2);	
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));

        glBindVertexArray(0);

        _primitives.emplace_back(p);
    }
}

const std::vector<Primitive>& Mesh::getPrimitives() const
{
    return _primitives;
}
