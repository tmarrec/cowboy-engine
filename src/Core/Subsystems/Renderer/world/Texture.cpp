#include "Texture.h"

Texture::Texture(const tinygltf::Texture& gltfTexture, tinygltf::Model& model)
: _image{std::make_shared<tinygltf::Image>(model.images[gltfTexture.source])}
, _img
{
    .width = _image->width,
    .height = _image->height,
    .data = _image->image
}
{
    std::cout << "Texture !" << std::endl;
}

const Image& Texture::getImage() const
{
    return _img;
}
