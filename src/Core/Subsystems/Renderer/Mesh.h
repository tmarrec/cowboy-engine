#pragma once

#define TINYGLTF_NO_STB_IMAGE
#include <tiny_gltf.h>
#include <cstdint>
#include <vector>
#include <glm/glm.hpp>

#include "../../utils.h"

template<typename T>
struct Buffer
{
    const T*        buffer = nullptr;
    const size_t    size = 0;
};

struct Primitive
{
    const size_t firstIndex;
    const size_t indexCount;
    const size_t vertexOffset;
};

class Mesh
{
 public:
    Mesh(const int idx, const tinygltf::Model& model, std::vector<std::uint16_t>& indicesBuffer, std::vector<float>& vertexBuffer, std::vector<Primitive>& primitives);

 private:
    template<typename T>
    const Buffer<T> getBuffer(const int from, const tinygltf::Model& model) const
    {
        const auto& accessor    = model.accessors[from];
        const auto& bufferView  = model.bufferViews[accessor.bufferView];
        const auto& buffer      = model.buffers[bufferView.buffer];

        const size_t bufferSize = [&]()
        {
            switch (bufferView.target)
            {
                case TINYGLTF_TARGET_ELEMENT_ARRAY_BUFFER:
                    return accessor.count;
                case TINYGLTF_TARGET_ARRAY_BUFFER:
                    return accessor.count * 3;
                default:
                    ERROR_EXIT("Bufferview target unrecognized.");
            }
        }();

        return
        {
            .buffer = reinterpret_cast<const T*>(&buffer.data[bufferView.byteOffset + accessor.byteOffset]),
            .size = bufferSize,
        };
    }
};
