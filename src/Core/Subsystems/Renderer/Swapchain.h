#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <optional>
#include <array>

#include "./../../utils.h"
#include "./../Window/Window.h"
#include "Framebuffer.h"

struct QueueFamilyIndices
{
	std::optional<uint32_t> graphics;
	std::optional<uint32_t> present;

	bool isComplete() const
	{
		return graphics.has_value() && present.has_value();
	}
};

struct SwapchainSupportDetails
{
    VkSurfaceCapabilitiesKHR        capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR>   presentModes;
};

class Swapchain
{
 public:
    Swapchain();
    const SwapchainSupportDetails querySupport(const VkPhysicalDevice device) const;
    const uint32_t imageCount() const;
    const VkFormat format() const;
    const VkExtent2D extent() const;
    VkSwapchainKHR& vkSwapchain();
    const std::vector<VkImage> vkSwapchainImages() const;
    void createFramebuffers();

 private:
    VkSurfaceFormatKHR chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR choosePresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D chooseExtent(const VkSurfaceCapabilitiesKHR capabilities);
    void createImages();
    void createImagesViews();
    void createDepthResources();
    VkImageView createImageView(const VkImage image, const VkFormat format, const VkImageAspectFlags aspectFlags);
    void createImage(const uint32_t width, const uint32_t height, const VkFormat format, const VkImageTiling tiling, const VkImageUsageFlags usage, const VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
    uint32_t findMemoryType(const uint32_t typeFilter, const VkMemoryPropertyFlags properties);

    VkSwapchainKHR  _vkSwapchain        = VK_NULL_HANDLE;
    VkFormat        _vkSurfaceFormat    = {};
    VkExtent2D      _vkExtent           = {};
    uint32_t        _imageCount         = 0;

    std::vector<VkImage>            _vkSwapchainImages          = {};
    std::vector<VkImageView>        _vkSwapchainImageViews      = {};
    VkImage _depthImage;
    VkDeviceMemory _depthImageMemory;
    VkImageView _depthImageView;

    std::vector<std::unique_ptr<Framebuffer>>        _swapchainFramebuffers      = {};
};
