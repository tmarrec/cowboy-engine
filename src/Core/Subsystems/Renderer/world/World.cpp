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

    //bool ret = loader.LoadASCIIFromFile(&model, &err, &warn, "models/Sponza.gltf");
    //bool ret = loader.LoadBinaryFromFile(&model, &err, &warn, "models/MetalRoughSpheres.glb");
    bool ret = loader.LoadBinaryFromFile(&model, &err, &warn, "models/AlphaBlendModeTest.glb");
 
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

    _scenes.reserve(model.scenes.size());
    for (const auto& gltfScene : model.scenes)
    {
        _scenes.emplace_back(gltfScene.nodes, model);
    }

    _currentScene = model.defaultScene;

    _textures.reserve(model.textures.size());
    for (const auto& gltfTexture : model.textures)
    {
        _textures.emplace_back(gltfTexture, model);
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
