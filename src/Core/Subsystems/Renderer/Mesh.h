#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>
#define TINYGLTF_NO_STB_IMAGE
#include <tiny_gltf.h>
#include <glm/glm.hpp>

#include "../../utils.h"

template<typename T>
struct Buffer
{
    const T*        buffer = nullptr;
    const size_t    size = 0;
};

struct GeometryBuffers
{
    std::vector<Buffer<std::uint16_t>>    indicesBuffers;
    std::vector<Buffer<float>>            positionsBuffers;
};

class Mesh
{
 public:
    Mesh(const int idx, const tinygltf::Model& model);
    const GeometryBuffers& getGeometryBuffers() const;

 private:
    template<typename T>
    const Buffer<T> getBuffer(const int from, const tinygltf::Model& model) const
    {
        const auto& accessor = model.accessors[from];
        const auto& bufferView = model.bufferViews[accessor.bufferView];
        const auto& buffer = model.buffers[bufferView.buffer];
        return
        {
            .buffer = reinterpret_cast<const T*>(&buffer.data[bufferView.byteOffset + accessor.byteOffset]),
            .size = accessor.count,
        };
    }

    GeometryBuffers _geometryBuffers;
};
