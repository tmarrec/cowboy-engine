#pragma once

#include <memory>

#include "Mesh.h"

class Node
{
 public:
    Node(const int idx, const tinygltf::Model& model, std::vector<uint16_t>& indicesBuffer, std::vector<float>& vertexBuffer, std::vector<Primitive>& primitives, std::vector<Node>& nodes, const glm::mat4& parentTransform);
    const std::vector<Primitive>& getPrimitives() const;
    const glm::mat4& getTransform() const;
    const bool gotMesh() const;

 private:
    std::shared_ptr<Mesh>   _mesh       = nullptr;
    glm::mat4               _transform;
};
