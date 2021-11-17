#include "PhysicalDevice.h"

#include <vector>
#include <set>

inline std::shared_ptr<Swapchain> g_swapchain;

PhysicalDevice::PhysicalDevice(const VkInstance vkInstance, const VkSurfaceKHR vkSurface)
: _vkSurface(vkSurface)
{
    // Get all the availables GPUs
    uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(vkInstance, &deviceCount, VK_NULL_HANDLE);
	if (deviceCount == 0)
	{
		ERROR_EXIT("Failed to find GPUs with Vulkan support.");
	}
	std::vector<VkPhysicalDevice> devices{deviceCount};
	vkEnumeratePhysicalDevices(vkInstance, &deviceCount, devices.data());
	
    // Takes the first suitable GPU
	for (const VkPhysicalDevice& device : devices)
	{
		if (isPhysicalDeviceSuitable(device))
		{
			_vkPhysicalDevice = device;
			break;
		}
	}
	
	if (_vkPhysicalDevice == VK_NULL_HANDLE)
	{
		ERROR_EXIT("Failed to find suitable GPU.");
	}

    // Get selected GPU informations
    VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(_vkPhysicalDevice, &deviceProperties);

	INFO("GPU: " << deviceProperties.deviceName);
    if (deviceProperties.limits.maxPushConstantsSize < 256)
    {
        ERROR_EXIT("maxPushConstantsSize should be > 256.");
    }
}

// Check if the device is suitable for renderings
bool PhysicalDevice::isPhysicalDeviceSuitable(const VkPhysicalDevice device) const
{
    const SwapchainSupportDetails swapchainSupport = g_swapchain->querySupport(device, _vkSurface);
    return findQueueFamilies(device).isComplete() 
        && checkDeviceExtensionSupport(device)
        && !swapchainSupport.formats.empty() && !swapchainSupport.presentModes.empty();
}

// Returns the queue families of a device
const QueueFamilyIndices PhysicalDevice::findQueueFamilies(const VkPhysicalDevice device) const
{
	QueueFamilyIndices indices;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, VK_NULL_HANDLE);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	uint32_t i = 0;
    // Search for the bits required
	for (const VkQueueFamilyProperties queueFamily : queueFamilies)
	{
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			indices.graphics = i;
		}

		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, _vkSurface, &presentSupport);
		if (presentSupport)
		{
			indices.present = i;
		}

		// Stop the search if all indices required are found
		if (indices.isComplete())
		{
			break;
		}
		++i;
	}

	return indices;
}

// Check if the device has the extensions required
bool PhysicalDevice::checkDeviceExtensionSupport(const VkPhysicalDevice device) const
{
    uint32_t extensionsCount;
    vkEnumerateDeviceExtensionProperties(device, VK_NULL_HANDLE, &extensionsCount, VK_NULL_HANDLE);

    std::vector<VkExtensionProperties> availableExtensions(extensionsCount);
    vkEnumerateDeviceExtensionProperties(device, VK_NULL_HANDLE, &extensionsCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(_deviceExtensions.begin(), _deviceExtensions.end());
    for (const auto& extension : availableExtensions)
    {
        requiredExtensions.erase(extension.extensionName);
    }
    return requiredExtensions.empty();
}

const std::array<const char*, 1>& PhysicalDevice::deviceExtensions() const
{
    return _deviceExtensions;
}

const VkPhysicalDevice PhysicalDevice::vkPhysicalDevice() const
{
    return _vkPhysicalDevice;
}
