#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <iostream>
#include <optional>

struct QueueFamilyIndices
{
	std::optional<std::uint32_t> graphics;
	std::optional<std::uint32_t> present;

	bool isComplete()
	{
		return graphics.has_value() && present.has_value();
	}
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

    VkInstance _vkInstance = VK_NULL_HANDLE;
    VkPhysicalDevice _vkPhysicalDevice = VK_NULL_HANDLE;
    VkSurfaceKHR _vkSurface = VK_NULL_HANDLE;
    VkDevice _vkDevice = VK_NULL_HANDLE;
};
