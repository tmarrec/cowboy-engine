#include "./Mesh.h"

#include <sstream>

#include "../Renderer.h"

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
                    return getBuffer<GLfloat>(pData.attributes.at(s.str()), model);
                }         
            }
            // If no texture, zeros for all texcoord
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
                .tangent = {0,0,0,0}
            };
            p.vertices.emplace_back(v);
            t += 2;
        }

        // If has material
        if (pData.material >= 0)
        {
            const auto& material = model.materials[pData.material];
            const auto& pbr = material.pbrMetallicRoughness;
            if (pbr.baseColorTexture.index >= 0)
            {
                p.material.hasAlbedoTexture = true;
                p.material.albedoTexture = pbr.baseColorTexture.index;
            }
            if (pbr.metallicRoughnessTexture.index >= 0)
            {
                p.material.hasMetallicRoughnessTexture = true;
                p.material.metallicRoughnessTexture = pbr.metallicRoughnessTexture.index;
            }
            if (material.emissiveTexture.index >= 0)
            {
                p.material.hasEmissiveTexture = true;
                p.material.emissiveTexture = material.emissiveTexture.index;
            }
            if (material.normalTexture.index >= 0)
            {
                p.material.hasNormalTexture = true;
                p.material.normalTexture = material.normalTexture.index;
            }
            if (material.occlusionTexture.index >= 0)
            {
                p.material.hasOcclusionTexture = true;
                p.material.occlusionTexture = material.occlusionTexture.index;
            }
            
            p.material.albedoFactor = glm::dvec3(pbr.baseColorFactor[0], pbr.baseColorFactor[1], pbr.baseColorFactor[2]);
            p.material.metallicFactor = pbr.metallicFactor;
            p.material.roughnessFactor = pbr.roughnessFactor;
            p.material.emissiveFactor = glm::dvec3(material.emissiveFactor[0], material.emissiveFactor[1], material.emissiveFactor[2]);
            p.material.normalTextureScale = material.normalTexture.scale;
            p.material.occlusionTextureStrength = material.occlusionTexture.strength;
        }

        p.indices = std::vector<GLuint>(pIndicesBuffer.buffer, pIndicesBuffer.buffer+pIndicesBuffer.size);

        // Primitive Tangent calculation
        SMikkTSpaceInterface interface =
        {
            .m_getNumFaces = getNumFaces,
            .m_getNumVerticesOfFace = getNumVerticesOfFace,
            .m_getPosition = getPosition,
            .m_getNormal = getNormal,
            .m_getTexCoord = getTexCoords,
            .m_setTSpaceBasic = setTSpaceBasic,
            .m_setTSpace = nullptr
        };

        SMikkTSpaceContext context =
        {
            .m_pInterface = &interface,
            .m_pUserData = &p
        };

        genTangSpaceDefault(&context);

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
        // Vertex tangent
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tangent));

        glBindVertexArray(0);

        _primitives.emplace_back(p);
    }
}

const std::vector<Primitive>& Mesh::getPrimitives() const
{
    return _primitives;
}

int getVertexIndex(const SMikkTSpaceContext* context, int iFace, int iVert)
{
    const Primitive* prim = static_cast<const Primitive*>(context->m_pUserData);
    const int64_t faceSize = getNumVerticesOfFace(context, iFace);
    return prim->indices[(iFace * faceSize) + iVert];
}

int getNumFaces(const SMikkTSpaceContext* context)
{
    const Primitive* prim = static_cast<const Primitive*>(context->m_pUserData);
    return static_cast<int>(prim->indices.size() / 3);
}

int getNumVerticesOfFace(const SMikkTSpaceContext* context, int iFace)
{
    return 3;
}

void getNormal(const SMikkTSpaceContext* context, float outnormal[], int iFace, int iVert)
{
    const Primitive* prim = static_cast<const Primitive*>(context->m_pUserData);
    const Vertex& vertex = prim->vertices[getVertexIndex(context, iFace, iVert)];
    outnormal[0] = vertex.normal.x;
    outnormal[1] = vertex.normal.y;
    outnormal[2] = vertex.normal.z;
}

void getPosition(const SMikkTSpaceContext* context, float outpos[], int iFace, int iVert)
{
    const Primitive* prim = static_cast<const Primitive*>(context->m_pUserData);
    const Vertex& vertex = prim->vertices[getVertexIndex(context, iFace, iVert)];
    outpos[0] = vertex.position.x;
    outpos[1] = vertex.position.y;
    outpos[2] = vertex.position.z;
}

void getTexCoords(const SMikkTSpaceContext* context, float outuv[], int iFace, int iVert)
{
    const Primitive* prim = static_cast<const Primitive*>(context->m_pUserData);
    const Vertex& vertex = prim->vertices[getVertexIndex(context, iFace, iVert)];
    outuv[0] = vertex.texCoords.x;
    outuv[1] = vertex.texCoords.y;
}

void setTSpaceBasic(const SMikkTSpaceContext* context, const float tangentu[], float fSign, int iFace, int iVert)
{
    Primitive* prim = static_cast<Primitive*>(context->m_pUserData);
    Vertex& vertex = prim->vertices[getVertexIndex(context, iFace, iVert)];

    vertex.tangent.x = tangentu[0];
    vertex.tangent.y = tangentu[1];
    vertex.tangent.z = tangentu[2];
    vertex.tangent.w = fSign;
}