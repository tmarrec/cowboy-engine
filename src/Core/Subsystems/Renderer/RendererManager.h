#pragma once

#include <array>
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <iostream>
#include <optional>
#include <set>
#include <vulkan/vulkan_core.h>
#include <glm/glm.hpp>
#include <cstring>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <chrono>

#include "Shader.h"

const std::uint8_t MAX_FRAMES_IN_FLIGHT = 2;

struct Vertex
{
    glm::vec2 pos;
    glm::vec3 color;

    static VkVertexInputBindingDescription getBindingDescription()
    {
        VkVertexInputBindingDescription bindindDescription{};
        bindindDescription.binding = 0;
        bindindDescription.stride = sizeof(Vertex);
        bindindDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindindDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions()
    {
        std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};

        // Position
        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, pos);

        // Color
        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, color);

        return attributeDescriptions;
    }
};

struct QueueFamilyIndices
{
	std::optional<std::uint32_t> graphics;
	std::optional<std::uint32_t> present;

	bool isComplete()
	{
		return graphics.has_value() && present.has_value();
	}
};

struct SwapchainSupportDetails
{
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

struct UniformBufferObject
{
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
};

class RendererManager
{
 public:
    RendererManager();
    ~RendererManager();
    void drawFrame();
    void waitIdle();
    void updateGraphicsPipeline();

 private:
    void createInstance();
    void createSurface();
    void pickPhysicalDevice();
    bool isPhysicalDeviceSuitable(VkPhysicalDevice device);
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
    void createLogicalDevice();
    bool checkDeviceExtensionSupport(const VkPhysicalDevice device);

    // Swapchain
    SwapchainSupportDetails querySwapchainSupport(VkPhysicalDevice device);
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
    void createSwapchain();
    void cleanupSwapchain();

    // Image Views
    void createImageViews();

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
    void createUniformBuffers();
    void updateUniformBuffer(std::uint32_t currentImage);
    void createDescriptorPool();
    void createDescriptorSets();



    // Shaders (temp, should use shader class in future)
    VkShaderModule createShaderModule(const std::vector<std::uint32_t>& code);

    const std::vector<const char*> _deviceExtensions =
    {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    VkInstance                      _vkInstance                 = VK_NULL_HANDLE;
    VkPhysicalDevice                _vkPhysicalDevice           = VK_NULL_HANDLE;
    VkSurfaceKHR                    _vkSurface                  = VK_NULL_HANDLE;
    VkDevice                        _vkDevice                   = VK_NULL_HANDLE;
    VkQueue                         _vkGraphicsQueue            = VK_NULL_HANDLE;
    VkQueue                         _vkPresentQueue             = VK_NULL_HANDLE;
    VkSwapchainKHR                  _vkSwapchain                = VK_NULL_HANDLE;
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

    VkFormat                        _vkSwapchainFormat = {};
    VkExtent2D                      _vkSwapchainExtent = {};

    std::vector<VkImage>            _vkSwapchainImages          = {};
    std::vector<VkImageView>        _vkSwapchainImageViews      = {};
    std::vector<VkFramebuffer>      _vkSwapchainFramebuffers    = {};
    std::vector<VkCommandBuffer>    _vkCommandBuffers           = {};
    std::vector<VkSemaphore>        _vkImageAvailableSemaphores = {};
    std::vector<VkSemaphore>        _vkRenderFinishedSemaphores = {};
    std::vector<VkFence>            _vkInFlightFences           = {};
    std::vector<VkFence>            _vkImagesInFlight           = {};
    std::vector<VkBuffer>           _uniformBuffers             = {};
    std::vector<VkDeviceMemory>     _uniformBuffersMemory       = {};
    std::vector<VkDescriptorSet>    _vkDescriptorSets           = {};
    
    Shader                          _vertShader = {"vert.vert", SHADER_TYPE_VERTEX};
    Shader                          _fragShader = {"frag.frag", SHADER_TYPE_FRAGMENT};

    std::uint8_t                    _currentFrame = 0;
    
    const std::vector<Vertex>       _vertices =
    {
        {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
        {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
        {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
        {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
    };

    const std::vector<std::uint16_t> _indices =
    {
        0, 1, 2, 2, 3, 0
    };

};
