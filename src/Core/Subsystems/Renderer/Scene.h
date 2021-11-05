#pragma once

#include "Node.h"

class Scene
{
 public:
    Scene(const std::vector<int>& nodesIdx, const tinygltf::Model& model);
    const std::vector<std::uint16_t>& getIndicesBuffer() const;
    const std::vector<float>& getPositionBuffer() const;

 private:
    void setGeometryBuffers();

    std::vector<Node>           _nodes;
    std::vector<std::uint16_t>  _allIndicesBuffer;
    std::vector<float>          _allVertexBuffer;
};
