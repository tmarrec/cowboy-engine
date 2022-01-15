#include "Renderer.h"

#include "../Window/Window.h"

#include <glm/gtx/string_cast.hpp>

#include <memory>

#include <cstdlib>
#include <ctime>

extern Window g_Window;

const uint64_t  TILE_SIZE           = 16;
const uint64_t  NR_LIGHTS           = 14000;
const uint64_t  MAX_LIGHTS_PER_TILE = 12000;
const uint64_t  SCREEN_WIDTH        = 1280;
const uint64_t  SCREEN_HEIGHT       = 720;


uint64_t  X_DISPATCH      = 0;
uint64_t  Y_DISPATCH      = 0;
uint64_t  THREAD_DISPATCH = 0;

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
    X_DISPATCH = static_cast<uint64_t>(ceil(static_cast<float>(SCREEN_WIDTH)  / TILE_SIZE));
    Y_DISPATCH = static_cast<uint64_t>(ceil(static_cast<float>(SCREEN_HEIGHT) / TILE_SIZE));
    THREAD_DISPATCH = X_DISPATCH * Y_DISPATCH;

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_DEBUG_OUTPUT);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glDebugMessageCallback(MessageCallback, 0);

    initDefaultTextures();
    initDepthBuffer();

    generateRandomLights();

    initForwardPass();

    glGenTextures(1, &_debugTexture);
    glBindTexture(GL_TEXTURE_2D, _debugTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_INT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    generateRenderingQuad();
    generateSphereVAO();
}

void Renderer::init()
{
    computeTiledFrustum();
}

// Compute tiles frustum once and for all
void Renderer::computeTiledFrustum()
{
    _computeFrustumShader.use();
    _computeFrustumShader.setMat4f("invProjection", glm::inverse(_cameraParameters.projection));

    _computeFrustumShader.set1i("tileSize", TILE_SIZE);
    _computeFrustumShader.set1i("screenWidth", SCREEN_WIDTH);
    _computeFrustumShader.set1i("screenHeight", SCREEN_HEIGHT);

    glGenBuffers(1, &_frustumBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, _frustumBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, THREAD_DISPATCH * sizeof(Frustum), nullptr, GL_STATIC_DRAW);
     
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, _frustumBuffer);
    glDispatchCompute(ceil(static_cast<float>(X_DISPATCH) / TILE_SIZE), ceil(static_cast<float>(Y_DISPATCH) / TILE_SIZE), 1);
}

void Renderer::initDefaultTextures()
{
    glGenTextures(1, &_defaultAlbedoTexture);
    glGenTextures(1, &_defaultMetallicRoughnessTexture);
    glGenTextures(1, &_defaultEmissiveTexture);
    glGenTextures(1, &_defaultNormalTexture);
    glGenTextures(1, &_defaultOcclusionTexture);
    
    OK("Default textures loaded");
}

void Renderer::initDepthBuffer()
{
    glGenFramebuffers(1, &_gDepthBuffer); 
    glBindFramebuffer(GL_FRAMEBUFFER, _gDepthBuffer);

    glGenTextures(1, &_gDepth);
    glBindTexture(GL_TEXTURE_2D, _gDepth);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _gDepth, 0);

    const GLuint attachments[1] = { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(1, attachments);

    GLuint rboDepth;
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCREEN_WIDTH, SCREEN_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        ERROR_EXIT("Depth framebuffer not complete");
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::initForwardPass()
{
    glGenBuffers(1, &_lightsBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, _lightsBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, NR_LIGHTS * sizeof(PointLight), nullptr, GL_STATIC_DRAW);

    glGenBuffers(1, &_lightIndexCounterBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, _lightIndexCounterBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, 1 * sizeof(uint32_t), nullptr, GL_STATIC_DRAW);

    glGenBuffers(1, &_lightIndexListBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, _lightIndexListBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, THREAD_DISPATCH * MAX_LIGHTS_PER_TILE * sizeof(uint32_t), nullptr, GL_STATIC_DRAW);

    glGenTextures(1, &_gLightGrid);
    glBindTexture(GL_TEXTURE_RECTANGLE, _gLightGrid);
    glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RG32UI, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RG_INTEGER, GL_UNSIGNED_INT, nullptr);
    glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

// Draw the frame by executing the queues while staying synchronised
void Renderer::drawFrame()
{
    // rotate lights
    for (int i = 0; i < pointLights.size(); ++i)
    {
        auto& l = pointLights[i];
        l.position.y += pointLightsSpeed[i];
        if (l.position.y > 25)
        {
            l.position.y = -5;
        }
        /*
        float angle = -pointLightsSpeed[i];
        float s = sin(angle);
        float c = cos(angle);

        float x = l.position.x * c - l.position.z * s;
        float z = l.position.x * s + l.position.z * c;

        l.position.x = x;
        l.position.z = z;
        */
    }

    copyLightDataToGPU();
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    depthPass();

    _tiledForwardShader.use();
    _tiledForwardShader.setMat4f("invProjection", glm::inverse(_cameraParameters.projection));

    _tiledForwardShader.set1i("numLights",      NR_LIGHTS);
    _tiledForwardShader.set1i("tileSize",       TILE_SIZE);
    _tiledForwardShader.set1i("screenWidth",    SCREEN_WIDTH);
    _tiledForwardShader.set1i("screenHeight",   SCREEN_HEIGHT);

    glBindImageTexture(                         0, _gDepth,             0, GL_FALSE, 0, GL_READ_ONLY,  GL_RGBA32F);
    glBindImageTexture(                         1, _gLightGrid,         0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG32UI);
    glBindImageTexture(                         2, _debugTexture,       0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER,  3, _lightIndexCounterBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER,  4, _lightIndexListBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER,  5, _lightsBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER,  6, _frustumBuffer);
    glDispatchCompute(X_DISPATCH, Y_DISPATCH, 1);
    
    //debugPass();

    tiledForwardPass();
    //drawTextureToScreen(_debugTexture);

    glBindVertexArray(0);
}

void Renderer::tiledForwardPass()
{
    _tiledForwardPassShader.use();
    _tiledForwardPassShader.set1i("lightGrid", 0);
    _tiledForwardPassShader.set1i("albedoMap", 1);
    _tiledForwardPassShader.set1i("metallicRoughnessMap", 2);
    _tiledForwardPassShader.set1i("emissiveMap", 3);
    _tiledForwardPassShader.set1i("normalMap", 4);
    _tiledForwardPassShader.set1i("occlusionMap", 5);

    _tiledForwardPassShader.setMat4f("projection", _cameraParameters.projection);
    _tiledForwardPassShader.setMat4f("view", _cameraParameters.view);
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
                    // Using default texture
                    glBindTexture(GL_TEXTURE_2D, _defaultAlbedoTexture);
                    const uint8_t albedo[3] = { 255 * primitive.material.albedoFactor.x, 255 * primitive.material.albedoFactor.y, 255 * primitive.material.albedoFactor.z };
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, &albedo);
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
                    const float metallic[4] = { 0, primitive.material.roughnessFactor, primitive.material.metallicFactor, 0 };
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 1, 1, 0, GL_RGBA, GL_FLOAT, &metallic);
                }

                // Emissive texture
                glActiveTexture(GL_TEXTURE3);
                if (primitive.material.hasEmissiveTexture)
                {
                    glBindTexture(GL_TEXTURE_2D, textures[primitive.material.emissiveTexture].id);
                }
                else
                {
                    glBindTexture(GL_TEXTURE_2D, _defaultEmissiveTexture);
                    const uint8_t emissive[3] = { 255*primitive.material.emissiveFactor.x, 255 * primitive.material.emissiveFactor.y, 255 * primitive.material.emissiveFactor.z };
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &emissive);
                }

                // Normal texture
                glActiveTexture(GL_TEXTURE4);
                if (primitive.material.hasNormalTexture)
                {
                    glBindTexture(GL_TEXTURE_2D, textures[primitive.material.normalTexture].id);
                }
                else
                {
                    glBindTexture(GL_TEXTURE_2D, _defaultNormalTexture);
                    const uint8_t normal[3] = { 128, 128, 255 };
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &normal);
                }

                // Occlusion texture
                glActiveTexture(GL_TEXTURE5);
                if (primitive.material.hasOcclusionTexture)
                {
                    glBindTexture(GL_TEXTURE_2D, textures[primitive.material.occlusionTexture].id);
                }
                else
                {
                    glBindTexture(GL_TEXTURE_2D, _defaultOcclusionTexture);
                    const uint8_t occlusion[3] = { 255, 0, 0 };
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &occlusion);
                }

                glBindVertexArray(primitive.VAO);
                glDrawElements(GL_TRIANGLES, primitive.indices.size(), GL_UNSIGNED_INT, 0);
            }
        }
    }
        
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::depthPass()
{
    glBindFramebuffer(GL_FRAMEBUFFER, _gDepthBuffer);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    _depthShader.use();
    _depthShader.setMat4f("projection", _cameraParameters.projection);
    _depthShader.setMat4f("view", _cameraParameters.view);
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

void Renderer::copyLightDataToGPU()
{
    glBindBuffer(GL_ARRAY_BUFFER, _lightsBuffer);
    PointLight* ptr = reinterpret_cast<PointLight*>(glMapBufferRange(GL_ARRAY_BUFFER, 0, pointLights.size() * sizeof(PointLight), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT));

    for (uint32_t i = 0; i < pointLights.size(); ++i)
    {
        ptr[i].color = pointLights[i].color;
        ptr[i].range = pointLights[i].range;
        ptr[i].position = pointLights[i].position;
        ptr[i].positionVS = _cameraParameters.view * glm::vec4(pointLights[i].position.xyz(), 1);
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
    _lightSpheresShader.use();
    _lightSpheresShader.setMat4f("projection", _cameraParameters.projection);
    _lightSpheresShader.setMat4f("view", _cameraParameters.view);
    
    for (uint16_t i = 0; i < pointLights.size(); ++i)
    {
        glm::mat4 model{1.0f};
        model = glm::translate(model, pointLights[i].position.xyz());
        model = glm::scale(model, glm::vec3(0.05f));
        _lightSpheresShader.setMat4f("model", model);
        _lightSpheresShader.set3f("color", pointLights[i].color);

        glBindVertexArray(_sphereVAO);
        glDrawElements(GL_TRIANGLES, sphereIndices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
}

void Renderer::generateRandomLights()
{
    srand(static_cast <unsigned> (time(0)));
    for (unsigned int i = 0; i < NR_LIGHTS; i++)
    {
        float x = (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * 18 - 9;
        float y = (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * 7 + 0.5f;
        float z = (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * 18 - 9;

        float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
        float g = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
        float b = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
        float k = 1.0f;
        pointLights.emplace_back(glm::vec3{r*k, g*k, b*k}, 2.0f, glm::vec4{x,-5,z,0});
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
    _cameraParameters.FOV = FOV;
    _cameraParameters.position = position;
    _cameraParameters.front = front;
    _cameraParameters.up = up;
    _cameraParameters.projection = glm::perspective(glm::radians(_cameraParameters.FOV), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 1024.0f);
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
