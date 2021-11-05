#include "Scene.h"

#include <iostream>

Scene::Scene(const std::vector<int>& nodesIdx, const tinygltf::Model& model)
{
    for (const auto idx : nodesIdx)
    {
        const Node node {idx, model};
        _nodes.emplace_back(node);
    }
    setGeometryBuffers();
}

void Scene::setGeometryBuffers()
{
    size_t offsetIndices = 0;
    size_t offsetVertex = 0;
    for (const auto& node : _nodes)
    {
        const auto& geoBuffers = node.getGeometryBuffers();
        for (const auto& indicesBuffer : geoBuffers.indicesBuffers)
        {
            _allIndicesBuffer.resize(_allIndicesBuffer.size()+indicesBuffer.size);
            memcpy(&_allIndicesBuffer[offsetIndices], indicesBuffer.buffer, indicesBuffer.size * sizeof(std::uint16_t));
            offsetIndices += indicesBuffer.size;
        }
        for (const auto& positionsBuffer : geoBuffers.positionsBuffers)
        {
            _allVertexBuffer.resize(_allVertexBuffer.size()+positionsBuffer.size);
            memcpy(&_allVertexBuffer[offsetVertex], positionsBuffer.buffer, positionsBuffer.size * sizeof(float));
            offsetVertex += positionsBuffer.size;
        }
    }
}

const std::vector<std::uint16_t>& Scene::getIndicesBuffer() const
{
    return _allIndicesBuffer;
}

const std::vector<float>& Scene::getPositionBuffer() const
{
    return _allVertexBuffer;
}

