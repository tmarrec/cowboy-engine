#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_SWIZZLE

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
#include <numbers>
#include <algorithm>
#include <execution>


#include "world/World.h"
#include "Shader.h"
#include "Camera.h"
#include "../../../Components/Transform.h"

struct Frustum
{
    glm::vec4   N[4]; // Normals
    float       d[4]; // Distance to origins
};

class Renderer
{
 public:
    Renderer();
    void init();
    void drawFrame();

    const uint64_t  TILE_SIZE = 16;
    const uint64_t  NR_LIGHTS = 32768*2;
    const uint64_t  MAX_LIGHTS_PER_TILE = 256;
    const uint64_t  SCREEN_WIDTH = 1280;
    const uint64_t  SCREEN_HEIGHT = 720;

 private:
    void initDefaultTextures();
    void initDepthBuffer();
    void initForwardPass();

    void computeTiledFrustum();

    void depthPass();
    void copyLightDataToGPU();
    void drawTextureToScreen(const GLuint texture);
    void generateRenderingQuad();
    void tiledForwardPass();
    

    void debugPass();
    void generateSphereVAO();

    World _world {};

    Shader _depthShader             {"./shaders/depth.vert",            "./shaders/depth.frag"};
    Shader _lightSpheresShader      {"./shaders/lightSpheres.vert",     "./shaders/lightSpheres.frag"};
    Shader _textureShader           {"./shaders/texture.vert",          "./shaders/texture.frag"};
    Shader _tiledForwardPassShader  {"./shaders/forwardplus.vert",      "./shaders/forwardplus.frag"};

    Shader _computeFrustumShader    {"./shaders/computeFrustum.comp"};
    Shader _tiledForwardShader      {"./shaders/tiledLightCulling.comp"};

    GLuint _defaultAlbedoTexture;
    GLuint _defaultMetallicRoughnessTexture;
    GLuint _defaultEmissiveTexture;
    GLuint _defaultNormalTexture;
    GLuint _defaultOcclusionTexture;

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
};
