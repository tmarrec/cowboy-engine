#include "Renderer.h"

#include "../Window/Window.h"

#include <glm/gtx/string_cast.hpp>

#include <memory>

extern Window g_Window;

// Initialize the Renderer manager
Renderer::Renderer()
{
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);  
    loadDefaultTextures();
}

// Clean all the objects related to Vulkan
Renderer::~Renderer()
{
}

void Renderer::loadDefaultTextures()
{
    // Default Albedo texture
    uint8_t albedo[3] = {255,0,0};
    glGenTextures(1, &_defaultAlbedoTexture);
    glBindTexture(GL_TEXTURE_2D, _defaultAlbedoTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, &albedo);

    // Default MetallicRoughness texture
    uint8_t metallicRoughness[3] = {0,0,0};
    glGenTextures(1, &_defaultMetallicRoughnessTexture);
    glBindTexture(GL_TEXTURE_2D, _defaultMetallicRoughnessTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, &metallicRoughness);

    OK("Default textures loaded");
}

// Draw the frame by executing the queues while staying synchronised
void Renderer::drawFrame()
{
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    _mainShader.use();
    _mainShader.setMat4f("view", glm::lookAt(_cameraParameters.position, _cameraParameters.position+_cameraParameters.front, _cameraParameters.up));
    _mainShader.setMat4f("projection", glm::perspective(glm::radians(_cameraParameters.FOV), 1280.0f / 720.0f, 0.1f, 100.0f));
    _mainShader.set3f("camPos", _cameraParameters.position);

    const auto& textures = _world.getTextures();
    _mainShader.set1i("albedoMap", 0);
    _mainShader.set1i("metallicRoughnessMap", 1);

    // Temp lights
    glm::vec3 lightPositions[] = {
        glm::vec3(-10.0f,  10.0f, 10.0f),
        glm::vec3( 10.0f,  10.0f, 10.0f),
        glm::vec3(-10.0f, -10.0f, 10.0f),
        glm::vec3( 10.0f, -10.0f, 10.0f),
    };
    glm::vec3 lightColors[] = {
        glm::vec3(300.0f, 300.0f, 300.0f),
        glm::vec3(300.0f, 300.0f, 300.0f),
        glm::vec3(300.0f, 300.0f, 300.0f),
        glm::vec3(300.0f, 300.0f, 300.0f)
    };

    for (unsigned int i = 0; i < sizeof(lightPositions) / sizeof(lightPositions[0]); ++i)
    {
        glm::vec3 newPos = lightPositions[i] + glm::vec3(sin(glfwGetTime() * 5.0) * 5.0, 0.0, 0.0);
        newPos = lightPositions[i];
        _mainShader.set3f("lightPositions[" + std::to_string(i) + "]", newPos);
        _mainShader.set3f("lightColors[" + std::to_string(i) + "]", lightColors[i]);
    }

    for (const auto& node : _world.getNodes())
    {
        if (node.gotMesh())
        {
            _mainShader.setMat4f("model", node.getTransform());

            for (const auto& primitive : node.getPrimitives())
            {
                // Albedo texture
                glActiveTexture(GL_TEXTURE0);
                if (primitive.material.hasAlbedoTexture)
                {
                    glBindTexture(GL_TEXTURE_2D, textures[primitive.material.albedoTexture].id);
                }
                else
                {
                    glBindTexture(GL_TEXTURE_2D, _defaultAlbedoTexture);
                }

                // MetallicRoughness texture
                glActiveTexture(GL_TEXTURE1);
                if (primitive.material.hasMetallicRoughnessTexture)
                {
                    glBindTexture(GL_TEXTURE_2D, textures[primitive.material.metallicRoughnessTexture].id);
                }
                else
                {
                    glBindTexture(GL_TEXTURE_2D, _defaultMetallicRoughnessTexture);
                }

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

