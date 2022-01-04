#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <array>
#include <vector>
#include <memory>
#include <iostream>
#include <optional>
#include <set>
#include <cstring>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <chrono>
#include <unordered_map>

#include "world/World.h"
#include "Shader.h"

struct CameraParameters
{
    float FOV;
    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 up;
    glm::mat4 projection;
    glm::mat4 view;
};

struct alignas(16) PointLight
{
    glm::vec3   color;
    float       radius;
    glm::vec3   position;
};

struct alignas(16) Plane
{
    glm::vec3   N; // Normal
    float       d; // Distance to origin
};

struct alignas(16) Frustum
{
    Plane planes[4]; // Left, Right, Top, Bottom
};

class Renderer
{
 public:
    Renderer();
    void drawFrame();
    void setCameraParameters(const glm::vec3& position, const float FOV, const glm::vec3& front, const glm::vec3& up);

 private:
    void loadDefaultTextures();
    void prepareGBuffer();
    void geometryPass();
    void prepareDepthBuffer();
    void depthPass();
    void generateRandomLights();
    void copyLightDataToGPU();
    void drawTextureToScreen(const GLuint texture);
    void generateRenderingQuad();
    void tiledForwardPass();

    void debugPass();
    void generateSphereVAO();

    CameraParameters _cameraParameters;
    glm::mat4 _projView = {};

    World _world {};

    Shader _gPassShader             {"./shaders/gpass.vert",            "./shaders/gpass.frag"};
    Shader _depthShader             {"./shaders/depth.vert",            "./shaders/depth.frag"};
    Shader _lightSpheresShader      {"./shaders/lightSpheres.vert",     "./shaders/lightSpheres.frag"};
    Shader _textureShader           {"./shaders/texture.vert",          "./shaders/texture.frag"};
    Shader _tiledForwardPassShader  {"./shaders/test.vert",            "./shaders/test.frag"};

    Shader _computeFrustumShader    {"./shaders/computeFrustum.comp"};
    Shader _tiledDeferredShader     {"./shaders/tiledDeferred.comp"};
    Shader _tiledForwardShader      {"./shaders/tiledForward.comp"};

    GLuint _defaultAlbedoTexture;
    GLuint _defaultMetallicRoughnessTexture;

    GLuint _gBuffer;
    GLuint _gDepthBuffer;
    GLuint _gDepth;
    GLuint _gPosition;
    GLuint _gNormal;
    GLuint _gAlbedo;
    GLuint _gMetallicRoughness;
    unsigned int rboDepth;

    GLuint _frustumBuffer;
    GLuint _lightsBuffer;
    GLuint _lightIndexCounterBuffer;
    GLuint _lightIndexListBuffer;

    GLuint _debugTexture;
    GLuint _gLightGrid;

    GLuint _quadVAO = 0;
    GLuint _quadVBO = 0;
    GLuint _sphereVAO = 0;
    GLuint _sphereVBO = 0;
    GLuint _sphereEBO = 0;
    std::vector<GLfloat> sphereVertices;
    std::vector<GLuint> sphereIndices;

    std::vector<PointLight> pointLights;
    std::vector<float> pointLightsSpeed;
};
