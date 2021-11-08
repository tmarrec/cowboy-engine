#pragma once

#include "../../utils.h"
#include "Scene.h"

class World
{
 public:
    World();
    const std::vector<std::uint16_t>& getIndicesBuffer() const;
    const std::vector<float>& getVertexBuffer() const;
    const std::vector<Primitive>& getPrimitives() const;
    const Scene& getScene() const;
    const std::vector<Node>& getNodes() const;

 private:
    std::vector<Scene>          _scenes;
    std::vector<std::uint16_t>  _indicesBuffer;
    std::vector<float>          _vertexBuffer;
};
