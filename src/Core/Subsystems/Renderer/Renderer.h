#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <array>
#include <vulkan/vulkan.h>
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

#include "Shader.h"
#include "world/World.h"
#include "GraphicsPipeline.h"
#include "DescriptorPool.h"
#include "DescriptorSet.h"
#include "CommandPool.h"

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
    void waitIdle();
    void updateGraphicsPipeline();
    void setCameraParameters(const glm::vec3& position, const float FOV, const glm::vec3& front, const glm::vec3& up);
    void createTexture(const Image& image);

 private:
    friend class PhysicalDevice;

    void createInstance();
    void createSurface();
    void pickPhysicalDevice();
    bool isPhysicalDeviceSuitable(VkPhysicalDevice device);
    void createLogicalDevice();

    // Swapchain
    void createSwapchain();

    void createTextureImage();

    void createImage(const uint32_t width, const uint32_t height, const VkFormat format, const VkImageTiling tiling, const VkImageUsageFlags usage, const VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
    VkCommandBuffer beginSingleTimeCommands();
    void endSingleTimeCommands(const VkCommandBuffer& commandBuffer);
    void transitionImageLayout(const VkImage image, const VkFormat format, const VkImageLayout oldLayout, const VkImageLayout newLayout);
    void copyBufferToImage(const VkBuffer buffer, const VkImage image, const uint32_t width, const uint32_t height);
    VkImageView createImageView(const VkImage image, const VkFormat format, const VkImageAspectFlags aspectFlags);
    void createTextureSampler();
    VkSampler _textureSampler;

    VkImage _depthImage;
    VkDeviceMemory _depthImageMemory;
    VkImageView _depthImageView;
    void createDepthResources();
    VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
    VkFormat findDepthFormat();

    // Graphics Pipeline
    void createRenderPass();

    void createCommandPool();
    void allocateCommandBuffers();
    void createSyncObjects();
    void createCommandBuffer(const uint32_t commandBufferIndex);

    void createVertexBuffer();
    void createIndexBuffer();
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
    void copyBuffer(VkBuffer& srcBuffer, VkBuffer& dstBuffer, VkDeviceSize& size);

    void updateUniformBuffer(const uint32_t currentImage);
    void createDescriptorPool();
    void createDescriptorSets();

    // Shaders (temp, should use shader class in future)
    VkShaderModule createShaderModule(const std::vector<uint32_t>& code);

    VkInstance                      _vkInstance                 = VK_NULL_HANDLE;
    VkSurfaceKHR                    _vkSurface                  = VK_NULL_HANDLE;
    VkCommandPool                   _vkCommandPool              = VK_NULL_HANDLE;
    VkBuffer                        _vkVertexBuffer             = VK_NULL_HANDLE;
    VkDeviceMemory                  _vkVertexBufferMemory       = VK_NULL_HANDLE;
    VkBuffer                        _vkIndexBuffer              = VK_NULL_HANDLE;
    VkDeviceMemory                  _vkIndexBufferMemory        = VK_NULL_HANDLE;

    std::vector<VkCommandBuffer>    _vkCommandBuffers           = {};
    std::vector<VkSemaphore>        _vkImageAvailableSemaphores = {};
    std::vector<VkSemaphore>        _vkRenderFinishedSemaphores = {};
    std::vector<VkFence>            _vkInFlightFences           = {};
    std::vector<VkFence>            _vkImagesInFlight           = {};
    std::vector<VkDescriptorSet>    _vkDescriptorSets           = {};
    
    uint8_t                         _currentFrame = 0;
    
    std::vector<Vertex>             _vertices = {};
    std::vector<uint16_t>           _indices = {};

    void loadModels();
    
    CameraParameters _cameraParameters;
    glm::mat4 _projView = {};

    void loadTextures();
    
    World _world {};
};
