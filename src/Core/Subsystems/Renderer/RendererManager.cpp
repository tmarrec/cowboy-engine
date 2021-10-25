#include "RendererManager.h"

#include "../Window/WindowManager.h"
#include <cstdint>
#include <limits>
#include <vulkan/vulkan_core.h>

extern WindowManager g_WindowManager;

// Initialize the Renderer manager
RendererManager::RendererManager()
{
    createInstance();
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
    createSwapChain();
}

// Clean Vulkan
RendererManager::~RendererManager()
{
    vkDestroySwapchainKHR(_vkDevice, _vkSwapChain, VK_NULL_HANDLE);
    vkDestroyDevice(_vkDevice, VK_NULL_HANDLE);
	vkDestroySurfaceKHR(_vkInstance, _vkSurface, VK_NULL_HANDLE);
    vkDestroyInstance(_vkInstance, VK_NULL_HANDLE);
}

// Choose the GPU to use
void RendererManager::pickPhysicalDevice()
{
    // Get all the availables GPUs
    std::uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(_vkInstance, &deviceCount, VK_NULL_HANDLE);
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

    // Get selected GPU informations
    VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(_vkPhysicalDevice, &deviceProperties);
	INFO("GPU: " << deviceProperties.deviceName);
}

// Create a Vulkan surface
void RendererManager::createSurface()
{
    g_WindowManager.windowCreateSurface(_vkInstance, &_vkSurface);
}

// Check if the device is suitable for renderings
bool RendererManager::isPhysicalDeviceSuitable(VkPhysicalDevice device)
{
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
    return findQueueFamilies(device).isComplete() 
        && checkDeviceExtensionSupport(device)
        && !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
}

// Check if the device has the extensions required
bool RendererManager::checkDeviceExtensionSupport(const VkPhysicalDevice device)
{
    std::uint32_t extensionsCount;
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

// Returns the queue families of a device
QueueFamilyIndices RendererManager::findQueueFamilies(VkPhysicalDevice device)
{
	QueueFamilyIndices indices;

	std::uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, VK_NULL_HANDLE);

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
	appInfo.pNext = VK_NULL_HANDLE;
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
    createInfo.ppEnabledLayerNames = VK_NULL_HANDLE;
    createInfo.pNext = VK_NULL_HANDLE;

    // Get required extensions by the Window Manager
    std::pair<const char**, std::uint32_t> windowRequiredInstanceExt = g_WindowManager.windowGetRequiredInstanceExtensions();
	std::vector<const char*> vkExtensions(windowRequiredInstanceExt.first, windowRequiredInstanceExt.first + windowRequiredInstanceExt.second);

    // Specify those required extensions to the Vulkan instance
	createInfo.enabledExtensionCount = static_cast<std::uint32_t>(vkExtensions.size());
	createInfo.ppEnabledExtensionNames = vkExtensions.data();

	if (vkCreateInstance(&createInfo, VK_NULL_HANDLE, &_vkInstance))
	{
		ERROR("Cannot create Vulkan instance.");
	}
}

// Create the logical Vulkan device
void RendererManager::createLogicalDevice()
{
    QueueFamilyIndices indices = findQueueFamilies(_vkPhysicalDevice);
	float queuePriority = 1.0f;

    // Graphics queue informations
    VkDeviceQueueCreateInfo queueCreateInfo{};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.pNext = VK_NULL_HANDLE;
    queueCreateInfo.flags = 0;
    queueCreateInfo.queueFamilyIndex = indices.graphics.value();
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;

	VkPhysicalDeviceFeatures deviceFeatures{};

    // Logical device informations
	VkDeviceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.pNext = VK_NULL_HANDLE;
	createInfo.flags = 0;
	createInfo.queueCreateInfoCount = 1;
	createInfo.pQueueCreateInfos = &queueCreateInfo;
	createInfo.enabledExtensionCount = static_cast<std::uint32_t>(_deviceExtensions.size());
	createInfo.ppEnabledExtensionNames = _deviceExtensions.data();
	createInfo.pEnabledFeatures = &deviceFeatures;
	createInfo.enabledLayerCount = 0;

	if (vkCreateDevice(_vkPhysicalDevice, &createInfo, VK_NULL_HANDLE, &_vkDevice) != VK_SUCCESS)
	{
		ERROR("Failed to create logical Vulkan device.")
	}

	vkGetDeviceQueue(_vkDevice, indices.graphics.value(), 0, &_vkGraphicsQueue);
}

// Get the details of the device swapchain support
SwapChainSupportDetails RendererManager::querySwapChainSupport(VkPhysicalDevice device)
{
    SwapChainSupportDetails details;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, _vkSurface, &details.capabilities);

    std::uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, _vkSurface, &formatCount, VK_NULL_HANDLE);

    if (formatCount != 0)
    {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, _vkSurface, &formatCount, details.formats.data());
    }

    std::uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, _vkSurface, &presentModeCount, VK_NULL_HANDLE);
    if (presentModeCount != 0)
    {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, _vkSurface, &presentModeCount, details.presentModes.data());
    }
    return details;
}

// Get the swapchain surface format, aiming for RGBA 32bits sRGB 
VkSurfaceFormatKHR RendererManager::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
    for (const auto& availableFormat : availableFormats)
    {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB
            && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return availableFormat;
        }
    }
    WARNING("Unable to find desired swapchain surface formats. Will use the first found.");
    return availableFormats[0];
}

// Get the swapchain present mode, aiming for MAILBOX mode
VkPresentModeKHR RendererManager::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
    for (const auto& availablePresentMode : availablePresentModes)
    {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            return availablePresentMode;
        }
    }
    WARNING("Unable to find desired swapchain present mode. Will use VK_PRESENT_MODE_FIFO_KHR.");
    return VK_PRESENT_MODE_FIFO_KHR;
}

// Choose the swap extent based on the window manager informations
VkExtent2D RendererManager::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
    if (capabilities.currentExtent.width != std::numeric_limits<std::uint32_t>::max())
    {
        return capabilities.currentExtent;
    }
    else
    {
        std::uint32_t width, height;
        g_WindowManager.windowGetFramebufferSize(width, height);
        VkExtent2D actualExtent = {width, height};

        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }
}

// Create the Vulkan swapchain
void RendererManager::createSwapChain()
{
    // Get the informations needed for the swapchain
    const SwapChainSupportDetails swapChainSupport = querySwapChainSupport(_vkPhysicalDevice);
    const VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    const VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    const std::uint32_t imageCount = [&]()
    {
        std::uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
        if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
        {
            imageCount = swapChainSupport.capabilities.maxImageCount;
        }
        return imageCount;
    }();
    _vkSwapChainFormat = surfaceFormat.format;
    _vkSwapChainExtent = chooseSwapExtent(swapChainSupport.capabilities);

    // Fill the swapchain informations
    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.pNext = VK_NULL_HANDLE;
    createInfo.flags = 0;
    createInfo.surface = _vkSurface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = _vkSwapChainFormat;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = _vkSwapChainExtent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices = findQueueFamilies(_vkPhysicalDevice);
    std::array<std::uint32_t, 2> queueFamilyIndices = {indices.graphics.value(), indices.present.value()};

    if (indices.graphics != indices.present)
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices.data();
    }
    else
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = VK_NULL_HANDLE;
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    // Create the swapchain
    if (vkCreateSwapchainKHR(_vkDevice, &createInfo, VK_NULL_HANDLE, &_vkSwapChain) != VK_SUCCESS)
    {
        ERROR("Failed to create swap chain.");
    }

    // Link the swapchain to the vector of images
    std::uint32_t swapchainImageCount;
    vkGetSwapchainImagesKHR(_vkDevice, _vkSwapChain, &swapchainImageCount, VK_NULL_HANDLE);
    _vkSwapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(_vkDevice, _vkSwapChain, &swapchainImageCount, _vkSwapChainImages.data());
}

