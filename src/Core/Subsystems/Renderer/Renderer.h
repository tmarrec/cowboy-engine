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

struct UniformBufferObject
{
    glm::mat4 view;
    glm::mat4 proj;
};

struct CameraParameters
{
    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 up;
    float FOV;
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
    CameraParameters _cameraParameters;
    glm::mat4 _projView = {};

    World _world {};

    Shader _mainShader {"./shaders/vert.vert", "./shaders/frag.frag"};

    GLuint _defaultAlbedoTexture;
    GLuint _defaultMetallicRoughnessTexture;
};
