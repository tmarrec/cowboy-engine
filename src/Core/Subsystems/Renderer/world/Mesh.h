#pragma once

#include <tiny_gltf.h>
#include <cstdint>
#include <vector>
#include <glm/glm.hpp>
#include <glad/gl.h>

#include "./../../../utils.h"
#include "MikkTSpace/mikktspace.h"

template<typename T>
struct Buffer
{
    const T*        buffer = nullptr;
    const size_t    size = 0;
};

struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoords;
    glm::vec4 tangent;
};

struct Material
{
    bool        hasAlbedoTexture = false;
    GLuint      albedoTexture;
    glm::dvec3  albedoFactor;

    bool        hasMetallicRoughnessTexture = false;
    GLuint      metallicRoughnessTexture;
    double      metallicFactor;
    double      roughnessFactor;

    bool        hasEmissiveTexture = false;
    GLuint      emissiveTexture;
    glm::dvec3  emissiveFactor;

    bool        hasNormalTexture = false;
    GLuint      normalTexture;
    double      normalTextureScale;

    bool        hasOcclusionTexture = false;
    GLuint      occlusionTexture;
    double      occlusionTextureStrength;
};

struct Primitive
{
    GLuint                  VAO;
    GLuint                  VBO;
    GLuint                  EBO;
    std::vector<Vertex>     vertices;
    std::vector<GLuint>     indices;
    Material                material;
};

class Mesh
{
 public:
    Mesh(const int idx, const tinygltf::Model& model, std::vector<uint16_t>& indicesBuffer, std::vector<float>& vertexBuffer, std::vector<Primitive>& primitives);
    ~Mesh();
    const std::vector<Primitive>& getPrimitives() const;

 private:
    template<typename T>
    const Buffer<T> getBuffer(const int from, const tinygltf::Model& model) const
    {
        const auto&  accessor   = model.accessors[from];
        const auto&  bufferView = model.bufferViews[accessor.bufferView];
        const auto&  buffer     = model.buffers[bufferView.buffer];
        const size_t typeSize   = [&accessor]()
        {
            switch(accessor.type)
            {
                case TINYGLTF_TYPE_SCALAR:
                    return 1;
                case TINYGLTF_TYPE_VEC2:
                    return 2;
                case TINYGLTF_TYPE_VEC3:
                    return 3;
                case TINYGLTF_TYPE_VEC4:
                    return 4;
                default:
                    ERROR_EXIT("Accessor data type unknown yet");
            }
        }();
        const size_t bufferSize = accessor.count * typeSize;

        return
        {
            .buffer = reinterpret_cast<const T*>(&buffer.data[bufferView.byteOffset + accessor.byteOffset]),
            .size = bufferSize,
        };
    }

    std::vector<Primitive> _primitives;
};

static int  getVertexIndex(const SMikkTSpaceContext* context, int iFace, int iVert);
static int  getNumFaces(const SMikkTSpaceContext* context);
static int  getNumVerticesOfFace(const SMikkTSpaceContext* context, int iFace);
static void getNormal(const SMikkTSpaceContext* context, float outnormal[], int iFace, int iVert);
static void getPosition(const SMikkTSpaceContext* context, float outpos[], int iFace, int iVert);
static void getTexCoords(const SMikkTSpaceContext* context, float outuv[], int iFace, int iVert);
static void setTSpaceBasic(const SMikkTSpaceContext* context, const float tangentu[], float fSign, int iFace, int iVert);