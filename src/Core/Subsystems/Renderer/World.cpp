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

    //bool ret = loader.LoadASCIIFromFile(&model, &err, &warn, "models/BoxTextured.glb");
    bool ret = loader.LoadBinaryFromFile(&model, &err, &warn, "models/BoxTextured.glb");

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

    _indicesBuffer = _scenes[_currentScene].getIndicesBuffer();
    _vertexBuffer = _scenes[_currentScene].getPositionBuffer();
}

const std::vector<std::uint16_t>& World::getIndicesBuffer() const
{
    return _indicesBuffer;
}

const std::vector<float>& World::getVertexBuffer() const
{
    return _vertexBuffer;
}

const std::vector<Node>& World::getNodes() const
{
    return _scenes[_currentScene].getNodes();
}
