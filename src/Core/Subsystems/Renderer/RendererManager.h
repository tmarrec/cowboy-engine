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

#include "Shader.h"
#include "Swapchain.h"
#include "World.h"

const std::uint8_t MAX_FRAMES_IN_FLIGHT = 3;

struct Vertex
{
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec2 texCoord;

    static VkVertexInputBindingDescription getBindingDescription()
    {
        return 
        {
            .binding = 0,
            .stride = sizeof(Vertex),
            .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
        };
    }

    static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions()
    {
        return
        {
            VkVertexInputAttributeDescription
            {   // Position
                .location = 0,
                .binding = 0,
                .format = VK_FORMAT_R32G32B32_SFLOAT,
                .offset = offsetof(Vertex, pos),
            },
            {   // Color
                .location = 1,
                .binding = 0,
                .format = VK_FORMAT_R32G32B32_SFLOAT,
                .offset = offsetof(Vertex, color),
            },
            {   // Texture Coordinates
                .location = 2,
                .binding = 0,
                .format = VK_FORMAT_R32G32_SFLOAT,
                .offset = offsetof(Vertex, texCoord),
            }
        };
    }
};

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

class RendererManager
{
 public:
    RendererManager();
    ~RendererManager();
    void drawFrame();
    void waitIdle();
    void updateGraphicsPipeline();
    void setCameraParameters(const glm::vec3& position, const float FOV, const glm::vec3& front, const glm::vec3& up);

 private:
    void createInstance();
    void createSurface();
    void pickPhysicalDevice();
    bool isPhysicalDeviceSuitable(VkPhysicalDevice device);
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
    void createLogicalDevice();
    bool checkDeviceExtensionSupport(const VkPhysicalDevice device);

    // Swapchain
    void createSwapchain();
    
    Swapchain _swapchain = {};

    // Image Views
    void createImageViews();
    void createImages();

    void createTextureImage();

    VkImage _textureImage;
    VkDeviceMemory _textureImageMemory;
    void createImage(const std::uint32_t width, const std::uint32_t height, const VkFormat format, const VkImageTiling tiling, const VkImageUsageFlags usage, const VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
    VkCommandBuffer beginSingleTimeCommands();
    void endSingleTimeCommands(const VkCommandBuffer& commandBuffer);
    void transitionImageLayout(const VkImage image, const VkFormat format, const VkImageLayout oldLayout, const VkImageLayout newLayout);
    void copyBufferToImage(const VkBuffer buffer, const VkImage image, const std::uint32_t width, const std::uint32_t height);
    VkImageView _textureImageView;
    void createTextureImageView();
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
    void createGraphicsPipeline();

    void createFramebuffers();
    void createCommandPool();
    void allocateCommandBuffers();
    void createSyncObjects();
    void createCommandBuffer(const std::uint32_t commandBufferIndex);

    void createVertexBuffer();
    void createIndexBuffer();
    std::uint32_t findMemoryType(std::uint32_t typeFilter, VkMemoryPropertyFlags properties);
    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
    void copyBuffer(VkBuffer& srcBuffer, VkBuffer& dstBuffer, VkDeviceSize& size);

    void createDescriptorSetLayout();
    void updateUniformBuffer(const std::uint32_t currentImage);
    void createDescriptorPool();
    void createDescriptorSets();



    // Shaders (temp, should use shader class in future)
    VkShaderModule createShaderModule(const std::vector<std::uint32_t>& code);

    const std::array<const char*, 1> _deviceExtensions =
    {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    VkInstance                      _vkInstance                 = VK_NULL_HANDLE;
    VkPhysicalDevice                _vkPhysicalDevice           = VK_NULL_HANDLE;
    VkSurfaceKHR                    _vkSurface                  = VK_NULL_HANDLE;
    VkDevice                        _vkDevice                   = VK_NULL_HANDLE;
    VkQueue                         _vkGraphicsQueue            = VK_NULL_HANDLE;
    VkQueue                         _vkPresentQueue             = VK_NULL_HANDLE;
    VkPipelineLayout                _vkPipelineLayout           = VK_NULL_HANDLE;
    VkRenderPass                    _vkRenderPass               = VK_NULL_HANDLE;
    VkPipeline                      _vkGraphicsPipeline         = VK_NULL_HANDLE;
    VkCommandPool                   _vkCommandPool              = VK_NULL_HANDLE;
    VkBuffer                        _vkVertexBuffer             = VK_NULL_HANDLE;
    VkDeviceMemory                  _vkVertexBufferMemory       = VK_NULL_HANDLE;
    VkBuffer                        _vkIndexBuffer              = VK_NULL_HANDLE;
    VkDeviceMemory                  _vkIndexBufferMemory        = VK_NULL_HANDLE;
    VkDescriptorSetLayout           _vkDescriptorSetLayout      = VK_NULL_HANDLE;
    VkDescriptorPool                _vkDescriptorPool           = VK_NULL_HANDLE;

    std::vector<VkImage>            _vkSwapchainImages          = {};
    std::vector<VkImageView>        _vkSwapchainImageViews      = {};
    std::vector<VkFramebuffer>      _vkSwapchainFramebuffers    = {};
    std::vector<VkCommandBuffer>    _vkCommandBuffers           = {};
    std::vector<VkSemaphore>        _vkImageAvailableSemaphores = {};
    std::vector<VkSemaphore>        _vkRenderFinishedSemaphores = {};
    std::vector<VkFence>            _vkInFlightFences           = {};
    std::vector<VkFence>            _vkImagesInFlight           = {};
    std::vector<VkDescriptorSet>    _vkDescriptorSets           = {};
    
    Shader                          _vertShader = {"vert.vert", SHADER_TYPE_VERTEX};
    Shader                          _fragShader = {"frag.frag", SHADER_TYPE_FRAGMENT};

    std::uint8_t                    _currentFrame = 0;
    
    std::vector<Vertex>             _vertices = {};
    std::vector<uint16_t>           _indices = {};

    void loadModels();
    
    CameraParameters _cameraParameters;
    glm::mat4 _projView = {};

    World _world {};
};
