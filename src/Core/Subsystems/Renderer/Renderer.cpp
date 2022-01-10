#include "Renderer.h"

#include "../Window/Window.h"

#include <glm/gtx/string_cast.hpp>

#include <memory>

#include <cstdlib>
#include <ctime>

extern Window g_Window;
const uint16_t NR_LIGHTS = 64;

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
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glDebugMessageCallback(MessageCallback, 0);

    loadDefaultTextures();
    prepareGBuffer();
    prepareDepthBuffer();


    generateRandomLights();
    glGenBuffers(1, &_lightsBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, _lightsBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, pointLights.size() * sizeof(PointLight), nullptr, GL_STATIC_DRAW);

    glGenBuffers(1, &_lightIndexCounterBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, _lightIndexCounterBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, 1 * sizeof(uint32_t), nullptr, GL_STATIC_DRAW);

    glGenBuffers(1, &_lightIndexListBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, _lightIndexListBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, pointLights.size() * sizeof(uint32_t), nullptr, GL_STATIC_DRAW);

    glGenTextures(1, &_gLightGrid);
    glBindTexture(GL_TEXTURE_RECTANGLE, _gLightGrid);
    glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RG32UI, 1280, 720, 0, GL_RG_INTEGER, GL_UNSIGNED_INT, nullptr);
    glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glGenTextures(1, &_debugTexture);
    glBindTexture(GL_TEXTURE_2D, _debugTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 1280, 720, 0, GL_RGBA, GL_UNSIGNED_INT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    generateRenderingQuad();
    generateSphereVAO();
}

void Renderer::init()
{
    initTiledFrustum();
}

void Renderer::initTiledFrustum()
{
    // Compute frustum once and for all
    _computeFrustumShader.use();
    _computeFrustumShader.setMat4f("invProjection", glm::inverse(_cameraParameters.projection));
    _computeFrustumShader.setMat4f("view", _cameraParameters.view);

    _computeFrustumShader.set1i("blockSize", 16);
    _computeFrustumShader.set1i("screenWidth", 1280);
    _computeFrustumShader.set1i("screenHeight", 720);

    glGenBuffers(1, &_frustumBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, _frustumBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, 80 * 45 * 1 * sizeof(Frustum), nullptr, GL_STATIC_DRAW);
     
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, _frustumBuffer);
    glDispatchCompute(80, 45, 1);
}

void Renderer::loadDefaultTextures()
{
    // Default Albedo texture
    const uint8_t albedo[3] = {255, 0, 0};
    glGenTextures(1, &_defaultAlbedoTexture);
    glBindTexture(GL_TEXTURE_2D, _defaultAlbedoTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, &albedo);

    // Default MetallicRoughness texture
    const uint8_t metallicRoughness[3] = {0, 0, 0};
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
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 1280, 720, 0, GL_RGBA, GL_FLOAT, nullptr);
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
}

void Renderer::prepareDepthBuffer()
{
    glGenFramebuffers(1, &_gDepthBuffer); 
    glBindFramebuffer(GL_FRAMEBUFFER, _gDepthBuffer);

    glGenTextures(1, &_gDepth);
    glBindTexture(GL_TEXTURE_2D, _gDepth);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 1280, 720, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _gDepth, 0);

    unsigned int attachments[1] = { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(1, attachments);

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
}

// Draw the frame by executing the queues while staying synchronised
void Renderer::drawFrame()
{
    // rotate lights
    for (int i = 0; i < pointLights.size(); ++i)
    {
        auto& l = pointLights[i];
        float angle = -pointLightsSpeed[i];
        float s = sin(angle);
        float c = cos(angle);

        float x = l.position.x * c - l.position.z * s;
        float z = l.position.x * s + l.position.z * c;

        //l.position.x = x;
        //l.position.z = z;
    }
    copyLightDataToGPU();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    depthPass();

    _tiledForwardShader.use();
    _tiledForwardShader.setMat4f("invProjection", glm::inverse(_cameraParameters.projection));
    _tiledForwardShader.setMat4f("view", _cameraParameters.view);

    _tiledForwardShader.set1i("numLights", NR_LIGHTS);
    _tiledForwardShader.set1i("blockSize", 16);
    _tiledForwardShader.set1i("screenWidth", 1280);
    _tiledForwardShader.set1i("screenHeight", 720);

    glBindImageTexture(                         0, _gDepth,             0, GL_FALSE, 0, GL_READ_ONLY,  GL_RGBA32F);
    glBindImageTexture(                         1, _gLightGrid,         0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG32UI);
    glBindImageTexture(                         2, _debugTexture,       0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER,  3, _lightIndexCounterBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER,  4, _lightIndexListBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER,  5, _lightsBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER,  6, _frustumBuffer);
    glDispatchCompute(80, 45, 1);

    tiledForwardPass();
    drawTextureToScreen(_debugTexture);

    glBindVertexArray(0);
}

void Renderer::tiledForwardPass()
{
    const auto projection = glm::perspective(glm::radians(_cameraParameters.FOV), 1280.0f / 720.0f, 0.1f, 1000.0f);
    const auto view = glm::lookAt(_cameraParameters.position, _cameraParameters.position+_cameraParameters.front, _cameraParameters.up);

    _tiledForwardPassShader.use();
    _tiledForwardPassShader.set1i("lightGrid", 0);
    _tiledForwardPassShader.set1i("albedoMap", 1);
    _tiledForwardPassShader.set1i("metallicRoughnessMap", 2);

    _tiledForwardPassShader.setMat4f("projection", projection);
    _tiledForwardPassShader.setMat4f("view", view);
    _tiledForwardPassShader.set3f("viewPos", _cameraParameters.position);

    
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, _lightIndexListBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, _lightsBuffer);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_RECTANGLE, _gLightGrid);

    const auto& textures = _world.getTextures();
    for (const auto& node : _world.getNodes())
    {
        if (node.gotMesh())
        {
            _tiledForwardPassShader.setMat4f("model", node.getTransform());

            for (const auto& primitive : node.getPrimitives())
            {
                // Albedo texture
                glActiveTexture(GL_TEXTURE1);
                if (primitive.material.hasAlbedoTexture)
                {
                    glBindTexture(GL_TEXTURE_2D, textures[primitive.material.albedoTexture].id);
                }
                else
                {
                    glBindTexture(GL_TEXTURE_2D, _defaultAlbedoTexture);
                }

                // MetallicRoughness texture
                glActiveTexture(GL_TEXTURE2);
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
    
    for (uint16_t i = 0; i < pointLights.size(); ++i)
    {
        glm::mat4 model{1.0f};
        model = glm::translate(model, pointLights[i].position);
        model = glm::scale(model, glm::vec3(0.5f));
        _tiledForwardPassShader.setMat4f("model", model);

        glBindVertexArray(_sphereVAO);
        glDrawElements(GL_TRIANGLES, sphereIndices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::depthPass()
{
    const auto projection = glm::perspective(glm::radians(_cameraParameters.FOV), 1280.0f / 720.0f, 0.1f, 1000.0f);
    const auto view = glm::lookAt(_cameraParameters.position, _cameraParameters.position+_cameraParameters.front, _cameraParameters.up);
    glBindFramebuffer(GL_FRAMEBUFFER, _gDepthBuffer);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    _depthShader.use();
    _depthShader.setMat4f("projection", projection);
    _depthShader.setMat4f("view", view);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _gDepth);

    for (const auto& node : _world.getNodes())
    {
        if (node.gotMesh())
        {
            _depthShader.setMat4f("model", node.getTransform());

            for (const auto& primitive : node.getPrimitives())
            {
                glBindVertexArray(primitive.VAO);
                glDrawElements(GL_TRIANGLES, primitive.indices.size(), GL_UNSIGNED_INT, 0);
            }
        }
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
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
        model = glm::scale(model, glm::vec3(0.7f));
        _lightSpheresShader.setMat4f("model", model);
        _lightSpheresShader.set3f("color", pointLights[i].color);

        glBindVertexArray(_sphereVAO);
        glDrawElements(GL_TRIANGLES, sphereIndices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
}

void Renderer::generateRandomLights()
{
    srand (static_cast <unsigned> (time(0)));

    for (unsigned int i = 0; i < NR_LIGHTS; i++)
    {
        float x = (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * 10;
        float y = (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * 20;
        float z = 0;
        float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
        float g = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
        float b = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
        pointLights.emplace_back(glm::vec3{r, g, b}, 1.5f, glm::vec3{x+4,y-8,z});
        pointLightsSpeed.emplace_back(r/2000);
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
    _cameraParameters.FOV = FOV;
    _cameraParameters.position = position;
    _cameraParameters.front = front;
    _cameraParameters.up = up;
    _cameraParameters.projection = glm::perspective(glm::radians(_cameraParameters.FOV), 1280.0f / 720.0f, 0.1f, 1000.0f);
    _cameraParameters.view = glm::lookAt(_cameraParameters.position, _cameraParameters.position+_cameraParameters.front, _cameraParameters.up);
}

void Renderer::generateSphereVAO()
{
    const uint8_t sectorCount = 16;
    const uint8_t stackCount = 16;
    float x, y, z, xy;
    const float sectorStep = 2 * std::numbers::pi / sectorCount;
    const float stackStep = std::numbers::pi / stackCount;
    float sectorAngle, stackAngle;
    for(uint8_t i = 0; i <= stackCount; ++i)
    {
        stackAngle = std::numbers::pi / 2 - i * stackStep;
        xy = cosf(stackAngle);
        z = sinf(stackAngle);
        for(uint8_t j = 0; j <= sectorCount; ++j)
        {
            sectorAngle = j * sectorStep;
            x = xy * cosf(sectorAngle);
            y = xy * sinf(sectorAngle);
            sphereVertices.push_back(x);
            sphereVertices.push_back(y);
            sphereVertices.push_back(z);
        }
    }
    int k1, k2;
    for(uint8_t i = 0; i < stackCount; ++i)
    {
        k1 = i * (sectorCount + 1);
        k2 = k1 + sectorCount + 1;

        for(uint8_t j = 0; j < sectorCount; ++j, ++k1, ++k2)
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

    glGenVertexArrays(1, &_sphereVAO);
    glGenBuffers(1, &_sphereVBO);
    glGenBuffers(1, &_sphereEBO);
    glBindVertexArray(_sphereVAO);

    glBindBuffer(GL_ARRAY_BUFFER, _sphereVBO);
    glBufferData(GL_ARRAY_BUFFER, sphereVertices.size()*sizeof(GLfloat), sphereVertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(GLfloat), (GLvoid*)nullptr);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _sphereEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphereIndices.size()*sizeof(GLuint), sphereIndices.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);
}
