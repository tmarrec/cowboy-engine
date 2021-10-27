#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <iostream>
#include <optional>
#include <set>
#include <shaderc/shaderc.hpp>
#include <vulkan/vulkan_core.h>

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

class RendererManager
{
 public:
    RendererManager();
    ~RendererManager();

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

    // Image Views
    void createImageViews();

    // Graphics Pipeline
    void createRenderPass();
    void createGraphicsPipeline();

    // Shaders (temp, should use shader class in future)
    void loadShaders();
    VkShaderModule createShaderModule(const std::vector<std::uint32_t>& code);

    const std::vector<const char*> _deviceExtensions =
    {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    VkInstance                  _vkInstance         = VK_NULL_HANDLE;
    VkPhysicalDevice            _vkPhysicalDevice   = VK_NULL_HANDLE;
    VkSurfaceKHR                _vkSurface          = VK_NULL_HANDLE;
    VkDevice                    _vkDevice           = VK_NULL_HANDLE;
    VkQueue                     _vkGraphicsQueue    = VK_NULL_HANDLE;
    VkSwapchainKHR              _vkSwapchain        = VK_NULL_HANDLE;
    VkPipelineLayout            _vkPipelineLayout   = VK_NULL_HANDLE;
    VkRenderPass                _vkRenderPass       = VK_NULL_HANDLE;
    VkPipeline                  _vkGraphicsPipeline = VK_NULL_HANDLE;

    VkFormat                    _vkSwapchainFormat      = {};
    VkExtent2D                  _vkSwapchainExtent      = {};
    std::vector<VkImage>        _vkSwapchainImages      = {};
    std::vector<VkImageView>    _vkSwapchainImageViews  = {};

    std::vector<std::uint32_t>  _vertShaderCode = {};
    std::vector<std::uint32_t>  _fragShaderCode = {};

};
