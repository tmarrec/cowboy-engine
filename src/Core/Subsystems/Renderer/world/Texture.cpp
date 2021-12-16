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
    /*
    glGenTextures(1, &_id);
    glBindTexture(GL_TEXTURE_2D, _id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, _img.width, _img.height, 0, GL_RGB, GL_UNSIGNED_BYTE, _img.data.data());
    glGenerateMipmap(GL_TEXTURE_2D);
    */
}

const Image& Texture::getImage() const
{
    return _img;
}
