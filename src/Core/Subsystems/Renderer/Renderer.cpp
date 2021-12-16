#include "Renderer.h"

#include "../Window/Window.h"

#include <memory>

extern Window g_Window;

// Initialize the Renderer manager
Renderer::Renderer()
{
    initShaders();
    loadModels();
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
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    _mainShader.use();

    glBindVertexArray(_VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void Renderer::loadModels()
{
    //const auto& badVertices = _world.getVertexBuffer();
    //const auto& badIndices = _world.getIndicesBuffer();


    
    //
    glGenVertexArrays(1, &_VAO);
    glGenBuffers(1, &_VBO);
    glGenBuffers(1, &_EBO);

    glBindVertexArray(_VAO);

    glBindBuffer(GL_ARRAY_BUFFER, _VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(_vertices), _vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(_indices), _indices, GL_STATIC_DRAW);

    // Position attributes
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);
    // Color attributes
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Renderer::setCameraParameters(const glm::vec3& position, const float FOV, const glm::vec3& front, const glm::vec3& up)
{
    _cameraParameters.position = position;
    _cameraParameters.FOV = FOV;
    _cameraParameters.front = front;
    _cameraParameters.up = up;
}

void Renderer::createTexture(const Image& image)
{
}

void Renderer::loadTextures()
{
    const auto& textures = _world.getTextures();
    for (const auto& texture : textures)
    {
        createTexture(texture.getImage());
    }
}
