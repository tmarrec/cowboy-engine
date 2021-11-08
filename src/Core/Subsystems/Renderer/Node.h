#pragma once

#include <memory>

#include "Mesh.h"

class Node
{
 public:
    Node(const int idx, const tinygltf::Model& model, std::vector<std::uint16_t>& indicesBuffer, std::vector<float>& vertexBuffer, std::vector<Primitive>& primitives);

 private:
    std::vector<Node>       _nodes;
    std::shared_ptr<Mesh>   _mesh;
    glm::mat4               _transform;
};
