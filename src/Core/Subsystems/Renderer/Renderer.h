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
    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 up;
    float FOV;
};

struct PointLight
{
    glm::vec3   color;
    float       radius;
    glm::vec3   position;
    unsigned int : 32;
};

class Renderer
{
 public:
    Renderer();
    ~Renderer();
    void drawFrame();
    void setCameraParameters(const glm::vec3& position, const float FOV, const glm::vec3& front, const glm::vec3& up);

 private:
    void loadDefaultTextures();
    void prepareGBuffer();
    void geometryPass();
    void lightsPass();
    void debugPass();
    void generateRandomLights();
    void copyLightDataToGPU();
    void drawTextureToScreen(const GLuint texture);
    void generateRenderingQuad();

    CameraParameters _cameraParameters;
    glm::mat4 _projView = {};

    World _world {};

    Shader _gPassShader {"./shaders/gpass.vert", "./shaders/gpass.frag"};
    Shader _pbrShader {"./shaders/pbr.vert", "./shaders/pbr.frag"};
    Shader _lightSpheresShader {"./shaders/lightSpheres.vert", "./shaders/lightSpheres.frag"};
    Shader _cullLightsShader {"./shaders/cullLights.comp"};
    Shader _textureShader {"./shaders/texture.vert", "./shaders/texture.frag"};


    GLuint _defaultAlbedoTexture;
    GLuint _defaultMetallicRoughnessTexture;

    GLuint _gBuffer;
    GLuint _gPosition;
    GLuint _gNormal;
    GLuint _gAlbedo;
    GLuint _gMetallicRoughness;

    GLuint _lightsBuffer;

    GLuint _output;

    GLuint _quadVAO = 0;
    GLuint _quadVBO = 0;
    unsigned int cubeVAO = 0;
    unsigned int cubeVBO = 0;
    unsigned int cubeEBO = 0;
    std::vector<GLfloat> sphereVertices;
    std::vector<GLuint> sphereIndices;

    std::vector<PointLight> pointLights;
    std::vector<float> pointLightsSpeed;
};
