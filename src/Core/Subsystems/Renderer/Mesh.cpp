#include "Mesh.h"

#include <cstdint>
#include <iostream>

#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/quaternion.hpp>

Mesh::Mesh(const int idx, const tinygltf::Model& model)
{
    std::vector<Buffer<std::uint16_t>> indicesBuffers;
    std::vector<Buffer<float>> positionsBuffers;
    for (const auto& primitive : model.meshes[idx].primitives)
    {
        indicesBuffers.emplace_back(getBuffer<std::uint16_t>(primitive.indices, model));
        positionsBuffers.emplace_back(getBuffer<float>(primitive.attributes.at("POSITION"), model));
    }

    _geometryBuffers =
    {
        .indicesBuffers = indicesBuffers,
        .positionsBuffers = positionsBuffers
    };
}

const GeometryBuffers& Mesh::getGeometryBuffers() const
{
    return _geometryBuffers;
}

