#pragma once

#include "./../../../utils.h"
#include "Mesh.h"

#include <memory>

struct Texture
{
    Texture(const tinygltf::Texture& gltfTexture, tinygltf::Model& model)
    {
        const auto& img = model.images[gltfTexture.source];
        const GLenum format = [&img]()
        {
            switch(img.component)
            {
                case 1:
                    return GL_RED;
                case 2:
                    return GL_RG;
                case 3:
                    return GL_RGB;
                case 4:
                    return GL_RGBA;
                default:
                    ERROR_EXIT("Texture image format");
            }
        }();
        const GLenum type = [&img]()
        {
            switch(img.bits)
            {
                case 8:
                    return GL_UNSIGNED_BYTE;
                case 16:
                    return GL_UNSIGNED_SHORT;
                default:
                    ERROR_EXIT("Texture image bits");
            }
        }();
        glGenTextures(1, &id);
        glBindTexture(GL_TEXTURE_2D, id);
        if (gltfTexture.sampler >= 0)
        {
            const auto& sampler = model.samplers[gltfTexture.sampler];
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, sampler.wrapS);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, sampler.wrapT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, sampler.minFilter);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, sampler.magFilter);
        }
        glTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGB, img.width, img.height, 0, format, type, img.image.data());
        glGenerateMipmap(GL_TEXTURE_2D);
        OK("Texture " << gltfTexture.source);
    }

    ~Texture()
    {
        glDeleteTextures(1, &id);
    }

    GLuint id;
};
