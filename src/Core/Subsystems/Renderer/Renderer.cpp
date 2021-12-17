#include "Renderer.h"

#include "../Window/Window.h"

#include <glm/gtx/string_cast.hpp>

#include <memory>

extern Window g_Window;

// Initialize the Renderer manager
Renderer::Renderer()
{
    initShaders();
    glEnable(GL_DEPTH_TEST);
}

// Clean all the objects related to Vulkan
Renderer::~Renderer()
{
}

void Renderer::initShaders()
{
}


// Draw the frame by executing the queues while staying synchronised
void Renderer::drawFrame()
{
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    _mainShader.use();
    _mainShader.setMat4f("view", glm::lookAt(_cameraParameters.position, _cameraParameters.position+_cameraParameters.front, _cameraParameters.up));
    _mainShader.setMat4f("projection", glm::perspective(glm::radians(_cameraParameters.FOV), 800.0f / 800.0f, 0.1f, 100.0f));

    const auto& textures = _world.getTextures();
    for (const auto& node : _world.getNodes())
    {
        if (node.gotMesh())
        {
            _mainShader.setMat4f("model", node.getTransform());
            for (const auto& primitive : node.getPrimitives())
            {
                glBindTexture(GL_TEXTURE_2D, textures[primitive.material.textureID].id);
                glBindVertexArray(primitive.VAO);
                glDrawElements(GL_TRIANGLES, primitive.indices.size(), GL_UNSIGNED_INT, 0);
            }
        }
    }
    glBindVertexArray(0);
}

void Renderer::setCameraParameters(const glm::vec3& position, const float FOV, const glm::vec3& front, const glm::vec3& up)
{
    _cameraParameters.position = position;
    _cameraParameters.FOV = FOV;
    _cameraParameters.front = front;
    _cameraParameters.up = up;
}

