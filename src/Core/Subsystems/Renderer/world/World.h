#pragma once

#include "./../../../utils.h"
#include "./Scene.h"
#include "./Texture.h"

class World
{
 public:
    World();
    const std::vector<uint16_t>& getIndicesBuffer() const;
    const std::vector<float>& getVertexBuffer() const;
    const std::vector<Node>& getNodes() const;
    const std::vector<Texture>& getTextures() const;

 private:
    std::vector<Scene>          _scenes;
    std::vector<uint16_t>       _indicesBuffer;
    std::vector<float>          _vertexBuffer;
    uint16_t                    _currentScene;
    std::vector<Texture>        _textures;
};
