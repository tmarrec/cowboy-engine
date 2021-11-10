#pragma once

#include "Node.h"

class Scene
{
 public:
    Scene(const std::vector<int>& nodesIdx, const tinygltf::Model& model);
    const std::vector<uint16_t>& getIndicesBuffer() const;
    const std::vector<float>& getPositionBuffer() const;
    const std::vector<Primitive>& getPrimitives() const;
    const std::vector<Node>& getNodes() const;

 private:
    std::vector<Node>           _nodes;
    std::vector<uint16_t>  _indicesBuffer;
    std::vector<float>          _vertexBuffer;
    std::vector<Primitive>      _primitives;
};
