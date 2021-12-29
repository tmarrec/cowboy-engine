#include "Renderer.h"

#include "../Window/Window.h"

#include <glm/gtx/string_cast.hpp>

#include <memory>

#include <cstdlib>
#include <ctime>

extern Window g_Window;

// Initialize the Renderer manager
Renderer::Renderer()
{
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);  
    loadDefaultTextures();
    prepareGBuffer();

    const unsigned int NR_LIGHTS = 16;

    srand (static_cast <unsigned> (time(0)));

    for (unsigned int i = 0; i < NR_LIGHTS; i++)
    {
        // calculate slightly random offsets
        float x = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
        //float y = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
        float z = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
        x -= 0.5;
        //y -= 0.5;
        z -= 0.5;
        lightPositions.push_back(glm::vec3(x*20, 0.5, z*10));
        // also calculate random color
        float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
        float g = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
        float b = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
        lightColors.push_back(glm::vec3(r, g, b));
    }
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

void Renderer::prepareGBuffer()
{
    glGenFramebuffers(1, &_gBuffer); 
    glBindFramebuffer(GL_FRAMEBUFFER, _gBuffer);

    // Position
    glGenTextures(1, &_gPosition);
    glBindTexture(GL_TEXTURE_2D, _gPosition);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 1280, 720, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _gPosition, 0);

    // Normal
    glGenTextures(1, &_gNormal);
    glBindTexture(GL_TEXTURE_2D, _gNormal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 1280, 720, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, _gNormal, 0);

    // Albedo
    glGenTextures(1, &_gAlbedo);
    glBindTexture(GL_TEXTURE_2D, _gAlbedo);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 1280, 720, 0, GL_RGB, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, _gAlbedo, 0);

    // MetallicRoughness
    glGenTextures(1, &_gMetallicRoughness);
    glBindTexture(GL_TEXTURE_2D, _gMetallicRoughness);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 1280, 720, 0, GL_RGB, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, _gMetallicRoughness, 0);

    unsigned int attachments[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
    glDrawBuffers(4, attachments);

    unsigned int rboDepth;
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, 1280, 720);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
    // finally check if framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "Framebuffer not complete!" << std::endl;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    _pbr.use();
    _pbr.set1i("gPosition", 0);
    _pbr.set1i("gNormal", 1);
    _pbr.set1i("gAlbedo", 2);
    _pbr.set1i("gMetallicRoughness", 3);
}

// Draw the frame by executing the queues while staying synchronised
void Renderer::drawFrame()
{
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

    const auto projection = glm::perspective(glm::radians(_cameraParameters.FOV), 1280.0f / 720.0f, 0.1f, 100.0f);
    const auto view = glm::lookAt(_cameraParameters.position, _cameraParameters.position+_cameraParameters.front, _cameraParameters.up);

    // 1. Geometry Pass
    glBindFramebuffer(GL_FRAMEBUFFER, _gBuffer);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_BLEND);
    _gPass.use();
    _gPass.set1i("albedoMap", 0);
    _gPass.set1i("metallicRoughnessMap", 1);
    _gPass.setMat4f("projection", projection);
    _gPass.setMat4f("view", view);

    const auto& textures = _world.getTextures();
    for (const auto& node : _world.getNodes())
    {
        if (node.gotMesh())
        {
            _gPass.setMat4f("model", node.getTransform());

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
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // 2. Lighting pass
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    _pbr.use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _gPosition);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, _gNormal);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, _gAlbedo);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, _gMetallicRoughness);

    for (unsigned int i = 0; i < lightPositions.size(); ++i)
    {
        glm::vec3 newPos = lightPositions[i] + glm::vec3(sin(glfwGetTime() * 5.0) * 5.0, 0.0, 0.0);
        newPos = lightPositions[i];
        _pbr.set3f("lights[" + std::to_string(i) + "].position", lightPositions[i]);
        _pbr.set3f("lights[" + std::to_string(i) + "].color", lightColors[i]);
    }

    _pbr.set3f("viewPos", _cameraParameters.position);

    // Final screenspace render
    if (quadVAO == 0)
    {
        float quadVertices[] = {
            // positions        // texture Coords
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
             1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
             1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        // setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    // Copy depth buffer to default framebuffer
    glBindFramebuffer(GL_READ_FRAMEBUFFER, _gBuffer);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, 1280, 720, 0, 0, 1280, 720, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    _lightSpheres.use();
    _lightSpheres.setMat4f("projection", projection);
    _lightSpheres.setMat4f("view", view);
    for (unsigned int i = 0; i < lightPositions.size(); ++i)
    {
        glm::mat4 model{1.0f};
        model = glm::translate(model, lightPositions[i]);
        model = glm::scale(model, glm::vec3(0.25f));
        _lightSpheres.setMat4f("model", model);
        // initialize (if necessary)
        if (cubeVAO == 0)
        {
            float radius = 1.0f;
            float PI = 3.14159265359;
            float sectorCount = 16;
            float stackCount = 16;
            float x, y, z, xy;                              // vertex position

            float sectorStep = 2 * PI / sectorCount;
            float stackStep = PI / stackCount;
            float sectorAngle, stackAngle;

            for(int i = 0; i <= stackCount; ++i)
            {
                stackAngle = PI / 2 - i * stackStep;        // starting from pi/2 to -pi/2
                xy = radius * cosf(stackAngle);             // r * cos(u)
                z = radius * sinf(stackAngle);              // r * sin(u)

                // add (sectorCount+1) vertices per stack
                // the first and last vertices have same position and normal, but different tex coords
                for(int j = 0; j <= sectorCount; ++j)
                {
                    sectorAngle = j * sectorStep;           // starting from 0 to 2pi

                    // vertex position (x, y, z)
                    x = xy * cosf(sectorAngle);             // r * cos(u) * cos(v)
                    y = xy * sinf(sectorAngle);             // r * cos(u) * sin(v)
                    sphereVertices.push_back(x);
                    sphereVertices.push_back(y);
                    sphereVertices.push_back(z);
                }
            }
            int k1, k2;
            for(int i = 0; i < stackCount; ++i)
            {
                k1 = i * (sectorCount + 1);     // beginning of current stack
                k2 = k1 + sectorCount + 1;      // beginning of next stack

                for(int j = 0; j < sectorCount; ++j, ++k1, ++k2)
                {
                    // 2 triangles per sector excluding first and last stacks
                    // k1 => k2 => k1+1
                    if(i != 0)
                    {
                        sphereIndices.push_back(k1);
                        sphereIndices.push_back(k2);
                        sphereIndices.push_back(k1 + 1);
                    }

                    // k1+1 => k2 => k2+1
                    if(i != (stackCount-1))
                    {
                        sphereIndices.push_back(k1 + 1);
                        sphereIndices.push_back(k2);
                        sphereIndices.push_back(k2 + 1);
                    }
                }
            }

            glGenVertexArrays(1, &cubeVAO);
            glGenBuffers(1, &cubeVBO);
            glGenBuffers(1, &cubeEBO);
            // fill buffer
            glBindVertexArray(cubeVAO);

                glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
                glBufferData(GL_ARRAY_BUFFER, sphereVertices.size()*sizeof(GLfloat), sphereVertices.data(), GL_STATIC_DRAW);
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(GLfloat), (GLvoid*)nullptr);
		        glEnableVertexAttribArray(0);

                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeEBO);
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphereIndices.size()*sizeof(GLuint), sphereIndices.data(), GL_STATIC_DRAW);

            glBindVertexArray(0);
        }
        // render Cube
        glBindVertexArray(cubeVAO);
        glDrawElements(GL_TRIANGLES, sphereIndices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
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

