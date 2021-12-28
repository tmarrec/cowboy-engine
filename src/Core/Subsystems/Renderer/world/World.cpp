#include "World.h"

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define TINYGLTF_NOEXCEPTION
#include <tiny_gltf.h>

World::World()
{
    tinygltf::Model model;
    tinygltf::TinyGLTF loader;
    std::string err;
    std::string warn;

    //bool ret = loader.LoadASCIIFromFile(&model, &err, &warn, "models/scene.gltf");
    bool ret = loader.LoadBinaryFromFile(&model, &err, &warn, "models/DamagedHelmet.glb");
 
    if (!warn.empty())
    {
        WARNING(warn.c_str());
    }

    if (!err.empty())
    {
        ERROR(err.c_str());
    }

    if (!ret)
    {
        ERROR_EXIT("Failed to load glTF file.");
    }

    for (const auto& gltfScene : model.scenes)
    {
        const Scene scene {gltfScene.nodes, model};
        _scenes.emplace_back(scene);
    }

    _currentScene = model.defaultScene;

    for (const auto& gltfTexture : model.textures)
    {
        const Texture texture {gltfTexture, model};
        _textures.emplace_back(texture);
    }
}

const std::vector<Node>& World::getNodes() const
{
    return _scenes[_currentScene].getNodes();
}

const std::vector<Texture>& World::getTextures() const
{
    return _textures;
}
