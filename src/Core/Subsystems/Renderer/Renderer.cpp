#include "Renderer.h"

#include "../Window/Window.h"

#include <glm/gtx/string_cast.hpp>

#include <memory>

#include <cstdlib>
#include <ctime>

extern Window g_Window;

void GLAPIENTRY MessageCallback(const GLenum source, const GLenum type, const GLuint id, const GLenum severity, const GLsizei length, const GLchar* message, const void* userParam)
{
    if (type == GL_DEBUG_TYPE_ERROR)
    {
        ERROR(message);
    }
    else
    {
        WARNING(message);
    }
}

// Initialize the Renderer manager
Renderer::Renderer()
{
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(MessageCallback, 0);

    loadDefaultTextures();
    prepareGBuffer();

    generateRandomLights();
    glGenBuffers(1, &_lightsBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, _lightsBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, pointLights.size() * sizeof(PointLight), nullptr, GL_STATIC_DRAW);

    glGenTextures(1, &_output);
    glBindTexture(GL_TEXTURE_2D, _output);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 1280, 720, 0, GL_RGBA, GL_UNSIGNED_INT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);


    generateRenderingQuad();
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
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 1280, 720, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, _gAlbedo, 0);

    // MetallicRoughness
    glGenTextures(1, &_gMetallicRoughness);
    glBindTexture(GL_TEXTURE_2D, _gMetallicRoughness);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 1280, 720, 0, GL_RGBA, GL_FLOAT, nullptr);
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

    _pbrShader.use();
    _pbrShader.set1i("gPosition", 0);
    _pbrShader.set1i("gNormal", 1);
    _pbrShader.set1i("gAlbedo", 2);
    _pbrShader.set1i("gMetallicRoughness", 3);
}

// Draw the frame by executing the queues while staying synchronised
void Renderer::drawFrame()
{
    // rotate lights
    for (int i = 0; i < pointLights.size(); ++i)
    {
        auto& l = pointLights[i];
        float angle = pointLightsSpeed[i];
        float s = sin(angle);
        float c = cos(angle);

        float x = l.position.x * c - l.position.z * s;
        float z = l.position.x * s + l.position.z * c;

        l.position.x = x;
        l.position.z = z;
    }
    copyLightDataToGPU();

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

    geometryPass();


    const auto projection = glm::perspective(glm::radians(_cameraParameters.FOV), 1280.0f / 720.0f, 0.1f, 1000.0f);
    const auto view = glm::lookAt(_cameraParameters.position, _cameraParameters.position+_cameraParameters.front, _cameraParameters.up);

    // Tiled Shading
    _cullLightsShader.use();
    _cullLightsShader.setMat4f("projection", projection);
    _cullLightsShader.setMat4f("view", view);
    _cullLightsShader.set3f("viewPos", _cameraParameters.position);
    glBindImageTexture(0, _gPosition,           0, GL_FALSE, 0, GL_READ_ONLY,  GL_RGBA16F);
    glBindImageTexture(1, _gNormal,             0, GL_FALSE, 0, GL_READ_ONLY,  GL_RGBA16F);
    glBindImageTexture(2, _gAlbedo,             0, GL_FALSE, 0, GL_READ_ONLY,  GL_RGBA16F);
    glBindImageTexture(3, _gMetallicRoughness,  0, GL_FALSE, 0, GL_READ_ONLY,  GL_RGBA16F);
    glBindImageTexture(4, _output,              0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, _lightsBuffer);
    glDispatchCompute(80, 45, 1);

    //lightsPass();

    /*
    // Final screenspace render
    glBindVertexArray(_quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    */

    // Copy depth buffer to default framebuffer
    glBindFramebuffer(GL_READ_FRAMEBUFFER, _gBuffer);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, 1280, 720, 0, 0, 1280, 720, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    drawTextureToScreen(_output);

    //debugPass();
    glBindVertexArray(0);
}

void Renderer::geometryPass()
{
    const auto projection = glm::perspective(glm::radians(_cameraParameters.FOV), 1280.0f / 720.0f, 0.1f, 1000.0f);
    const auto view = glm::lookAt(_cameraParameters.position, _cameraParameters.position+_cameraParameters.front, _cameraParameters.up);
    glBindFramebuffer(GL_FRAMEBUFFER, _gBuffer);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_BLEND);
    _gPassShader.use();
    _gPassShader.set1i("albedoMap", 0);
    _gPassShader.set1i("metallicRoughnessMap", 1);
    _gPassShader.setMat4f("projection", projection);
    _gPassShader.setMat4f("view", view);

    const auto& textures = _world.getTextures();
    for (const auto& node : _world.getNodes())
    {
        if (node.gotMesh())
        {
            _gPassShader.setMat4f("model", node.getTransform());

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
}

void Renderer::lightsPass()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    _pbrShader.use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _gPosition);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, _gNormal);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, _gAlbedo);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, _gMetallicRoughness);

    for (uint16_t i = 0; i < pointLights.size(); ++i)
    {
        _pbrShader.set3f("lights[" + std::to_string(i) + "].position", pointLights[i].position);
        _pbrShader.set3f("lights[" + std::to_string(i) + "].color", pointLights[i].color);
    }

    _pbrShader.set3f("viewPos", _cameraParameters.position);
}

void Renderer::copyLightDataToGPU()
{

    glBindBuffer(GL_ARRAY_BUFFER, _lightsBuffer);
    PointLight* ptr = reinterpret_cast<PointLight*>(glMapBufferRange(GL_ARRAY_BUFFER, 0, pointLights.size() * sizeof(PointLight), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT));
    for (uint16_t i = 0; i < pointLights.size(); ++i)
    {
        ptr[i].color = pointLights[i].color;
        ptr[i].radius = pointLights[i].radius;
        ptr[i].position = pointLights[i].position;
    }
    glUnmapBuffer(GL_ARRAY_BUFFER);
}

void Renderer::generateRenderingQuad()
{
    const std::array<float, 20> quadVertices =
    {
        -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
         1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
         1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
    };
    glGenVertexArrays(1, &_quadVAO);
    glGenBuffers(1, &_quadVBO);
    glBindVertexArray(_quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, _quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
}

void Renderer::debugPass()
{
    const auto projection = glm::perspective(glm::radians(_cameraParameters.FOV), 1280.0f / 720.0f, 0.1f, 1000.0f);
    const auto view = glm::lookAt(_cameraParameters.position, _cameraParameters.position+_cameraParameters.front, _cameraParameters.up);
    _lightSpheresShader.use();
    _lightSpheresShader.setMat4f("projection", projection);
    _lightSpheresShader.setMat4f("view", view);
    for (uint16_t i = 0; i < pointLights.size(); ++i)
    {
        glm::mat4 model{1.0f};
        model = glm::translate(model, pointLights[i].position);
        model = glm::scale(model, glm::vec3(0.25f));
        _lightSpheresShader.setMat4f("model", model);
        if (cubeVAO == 0)
        {
            float radius = 0.1f;
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
                for(int j = 0; j <= sectorCount; ++j)
                {
                    sectorAngle = j * sectorStep;           // starting from 0 to 2pi
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
                    if(i != 0)
                    {
                        sphereIndices.push_back(k1);
                        sphereIndices.push_back(k2);
                        sphereIndices.push_back(k1 + 1);
                    }
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
            glBindVertexArray(cubeVAO);

                glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
                glBufferData(GL_ARRAY_BUFFER, sphereVertices.size()*sizeof(GLfloat), sphereVertices.data(), GL_STATIC_DRAW);
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(GLfloat), (GLvoid*)nullptr);
		        glEnableVertexAttribArray(0);

                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeEBO);
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphereIndices.size()*sizeof(GLuint), sphereIndices.data(), GL_STATIC_DRAW);

            glBindVertexArray(0);
        }
        glBindVertexArray(cubeVAO);
        glDrawElements(GL_TRIANGLES, sphereIndices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
}

void Renderer::generateRandomLights()
{
    const unsigned int NR_LIGHTS = 12000;

    srand (static_cast <unsigned> (time(0)));

    for (unsigned int i = 0; i < NR_LIGHTS; i++)
    {
        float x = (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * 16;
        float y = (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * 16;
        float z = 0;
        float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
        float g = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
        float b = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
        pointLights.emplace_back(glm::vec3{r, g, b}, 1.0f, glm::vec3{x,y,z});
        pointLightsSpeed.emplace_back(r/50);
    }
}

void Renderer::drawTextureToScreen(const GLuint texture)
{
    _textureShader.use();
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glBindVertexArray(_quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void Renderer::setCameraParameters(const glm::vec3& position, const float FOV, const glm::vec3& front, const glm::vec3& up)
{
    _cameraParameters.position = position;
    _cameraParameters.FOV = FOV;
    _cameraParameters.front = front;
    _cameraParameters.up = up;
}

