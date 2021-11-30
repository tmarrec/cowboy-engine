#include "Instance.h"
#include <vector>

#include "../Window/Window.h"

extern Window g_Window;

Instance::Instance()
{
    // Engine informations
    const VkApplicationInfo appInfo =
    {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pNext = VK_NULL_HANDLE,
        .pApplicationName = "vulkan-testings",
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName = "vulkan-testings",
        .engineVersion = VK_MAKE_VERSION(0, 1, 0),
        .apiVersion = VK_API_VERSION_1_2,
    };

    // Get required extensions by the Window Manager
    const std::pair<const char**, uint32_t> windowRequiredInstanceExt = g_Window.windowGetRequiredInstanceExtensions();
	const std::vector<const char*> vkExtensions(windowRequiredInstanceExt.first, windowRequiredInstanceExt.first + windowRequiredInstanceExt.second);

    // Instance informations
    const VkInstanceCreateInfo instanceInfo =
    {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = VK_NULL_HANDLE,
        .flags = 0,
        .pApplicationInfo = &appInfo,
        // No validation layers as we are using the vkconfig overwrite
        .enabledLayerCount = 0,
        .ppEnabledLayerNames = VK_NULL_HANDLE,
	    .enabledExtensionCount = static_cast<uint32_t>(vkExtensions.size()),
	    .ppEnabledExtensionNames = vkExtensions.data(),
    };

    // Specify those required extensions to the Vulkan instance
	if (vkCreateInstance(&instanceInfo, VK_NULL_HANDLE, &_vkInstance))
	{
		ERROR_EXIT("Cannot create Vulkan instance.");
	}
    OK("Vulkan instance");
}

Instance::~Instance()
{
    vkDestroySurfaceKHR(_vkInstance, _vkSurface, VK_NULL_HANDLE);
    vkDestroyInstance(_vkInstance, VK_NULL_HANDLE);
}

const VkInstance Instance::vkInstance() const
{
    return _vkInstance;
}

const VkSurfaceKHR Instance::vkSurface() const
{
    return _vkSurface;
}

VkSurfaceKHR* Instance::vkSurfacePtr()
{
    return &_vkSurface;
}
