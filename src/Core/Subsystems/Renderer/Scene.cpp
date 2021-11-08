#include "Scene.h"

#include <iostream>

Scene::Scene(const std::vector<int>& nodesIdx, const tinygltf::Model& model)
{
    for (const auto idx : nodesIdx)
    {
        const Node node {idx, model, _indicesBuffer, _vertexBuffer, _primitives};
        _nodes.emplace_back(node);
    }
}

const std::vector<std::uint16_t>& Scene::getIndicesBuffer() const
{
    return _indicesBuffer;
}

const std::vector<float>& Scene::getPositionBuffer() const
{
    return _vertexBuffer;
}

const std::vector<Primitive>& Scene::getPrimitives() const
{
    return _primitives;
}

