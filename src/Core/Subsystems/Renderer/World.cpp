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

    bool ret = loader.LoadASCIIFromFile(&model, &err, &warn, "models/scene.gltf");

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
        ERROR_EXIT("Failed to load glTF.");
    }

    for (const auto& gltfScene : model.scenes)
    {
        Scene scene {gltfScene.nodes, model};
        _scenes.emplace_back(scene);

        // TODO : Select between multiples scenes
        _indicesBuffer = scene.getIndicesBuffer();
        _vertexBuffer = scene.getPositionBuffer();
    }
    for (const auto& v : _vertexBuffer)
    {
        std::cout << v << ' ';
    }
    std::cout << '\n';
    for (const auto& i : _indicesBuffer)
    {
        std::cout << i << ' ';
    }
    std::cout << '\n';

}

const std::vector<std::uint16_t>& World::getIndicesBuffer() const
{
    return _indicesBuffer;
}

const std::vector<float>& World::getVertexBuffer() const
{
    return _vertexBuffer;
}

const Scene& World::getScene() const
{
    return _scenes[0];
}

const std::vector<Primitive>& World::getPrimitives() const
{
    return _scenes[0].getPrimitives();
}
