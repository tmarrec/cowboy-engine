#include "RendererManager.h"

#include "../Window/WindowManager.h"
#include <type_traits>

extern WindowManager g_WindowManager;

// Initialize the Renderer manager
RendererManager::RendererManager()
{
    createInstance();
    createSurface();
    pickPhysicalDevice();
}

// Choose the GPU to use
void RendererManager::pickPhysicalDevice()
{
    // Get all the availables GPUs
    std::uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(_vkInstance, &deviceCount, nullptr);
	if (deviceCount == 0)
	{
		ERROR("Failed to find GPUs with Vulkan support.");
	}
	std::vector<VkPhysicalDevice> devices{deviceCount};
	vkEnumeratePhysicalDevices(_vkInstance, &deviceCount, devices.data());
	
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
		ERROR("Failed to find suitable GPU.");
	}
}

// Create a Vulkan surface
void RendererManager::createSurface()
{
    g_WindowManager.windowCreateSurface(_vkInstance, &_vkSurface);
}

// Check if the device is suitable for renderings
bool RendererManager::isPhysicalDeviceSuitable(VkPhysicalDevice device)
{
    VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);

	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

	QueueFamilyIndices queueFamilyIndices = findQueueFamilies(device);

	// Temporary using only discrete GPU
	if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU
        && deviceFeatures.geometryShader && queueFamilyIndices.isComplete())
	{
		INFO("Found suitable GPU: " << deviceProperties.deviceName);
		return true;
	}
	return false;
}

// Returnsthe queue families of a device
QueueFamilyIndices RendererManager::findQueueFamilies(VkPhysicalDevice device)
{
	QueueFamilyIndices indices;

	std::uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	std::uint32_t i = 0;
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

// Create the Vulkan instance
void RendererManager::createInstance()
{
    // Engine informations
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pNext = nullptr;
	appInfo.pApplicationName = "vulkan-testings";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "vulkan-testings";
	appInfo.engineVersion = VK_MAKE_VERSION(0, 1, 0);
	appInfo.apiVersion = VK_API_VERSION_1_2;

    // Instance informations
    VkInstanceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.flags = 0;
	createInfo.pApplicationInfo = &appInfo;

    // No validation layers as we are using the vkconfig overwrite
    createInfo.enabledLayerCount = 0;
    createInfo.ppEnabledLayerNames = nullptr;
    createInfo.pNext = nullptr;

    // Get required extensions by the Window Manager
    std::pair<const char**, std::uint32_t> windowRequiredInstanceExt = g_WindowManager.windowGetRequiredInstanceExtensions();
	std::vector<const char*> vkExtensions(windowRequiredInstanceExt.first, windowRequiredInstanceExt.first + windowRequiredInstanceExt.second);

    // Specify those required extensions to the Vulkan instance
	createInfo.enabledExtensionCount = static_cast<std::uint32_t>(vkExtensions.size());
	createInfo.ppEnabledExtensionNames = vkExtensions.data();

	if (vkCreateInstance(&createInfo, nullptr, &_vkInstance))
	{
		ERROR("Cannot create Vulkan instance.");
	}
}

// Clean Vulkan
RendererManager::~RendererManager()
{
    vkDestroyDevice(_vkDevice, nullptr);
	vkDestroySurfaceKHR(_vkInstance, _vkSurface, nullptr);
    vkDestroyInstance(_vkInstance, nullptr);
}
