#pragma once

#include "./../../../utils.h"
#include "./Scene.h"
#include "./Texture.h"

class World
{
 public:
    World();
    const std::vector<Node>& getNodes() const;
    const std::vector<Texture>& getTextures() const;

 private:
    std::vector<Scene>          _scenes;
    uint16_t                    _currentScene;
    std::vector<Texture>        _textures;
};
