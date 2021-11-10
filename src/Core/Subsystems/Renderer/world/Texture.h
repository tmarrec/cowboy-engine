#pragma once

#include "./../../../utils.h"
#include "Mesh.h"

#include <memory>

struct Image
{
    const int width;
    const int height;
    const std::vector<uint8_t>& data;
};

class Texture
{
 public:
    Texture(const tinygltf::Texture& gltfTexture, tinygltf::Model& model);
    const Image& getImage() const;

 private:
    std::shared_ptr<tinygltf::Image> _image;
    const Image _img;
};
