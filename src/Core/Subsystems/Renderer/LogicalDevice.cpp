#include "LogicalDevice.h"

inline std::shared_ptr<PhysicalDevice> g_physicalDevice;

LogicalDevice::LogicalDevice()
{
    const QueueFamilyIndices indices = g_physicalDevice->findQueueFamilies(g_physicalDevice->vkPhysicalDevice());
	const float queuePriority = 1.0f;

    // Graphics queue informations
    const VkDeviceQueueCreateInfo queueCreateInfo =
    {
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .pNext = VK_NULL_HANDLE,
        .flags = 0,
        .queueFamilyIndex = indices.graphics.value(),
        .queueCount = 1,
        .pQueuePriorities = &queuePriority,
    };

	const VkPhysicalDeviceFeatures deviceFeatures =
    {
        .samplerAnisotropy = VK_TRUE,
    };

    const VkPhysicalDeviceDescriptorIndexingFeatures indexingFeatures =
    {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES,
        .shaderSampledImageArrayNonUniformIndexing = VK_TRUE,
        .descriptorBindingPartiallyBound = VK_TRUE,
        .descriptorBindingVariableDescriptorCount = VK_TRUE,
        .runtimeDescriptorArray = VK_TRUE,
    };

    const auto deviceExtensions = g_physicalDevice->deviceExtensions();

    // Logical device informations
	const VkDeviceCreateInfo createInfo =
    {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = &indexingFeatures,
        .flags = 0,
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = &queueCreateInfo,
        .enabledLayerCount = 0,
        .enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size()),
        .ppEnabledExtensionNames = deviceExtensions.data(),
        .pEnabledFeatures = &deviceFeatures,
    };

	if (vkCreateDevice(g_physicalDevice->vkPhysicalDevice(), &createInfo, VK_NULL_HANDLE, &_vkDevice) != VK_SUCCESS)
	{
		ERROR_EXIT("Failed to create logical Vulkan device.")
	}

    /* TODO FIX THIS
	vkGetDeviceQueue(_vkDevice, indices.graphics.value(), 0, &_vkGraphicsQueue);
	vkGetDeviceQueue(_vkDevice, indices.present.value(), 0, &_vkPresentQueue);
    */
}

const VkDevice LogicalDevice::vkDevice() const
{
    return _vkDevice;
}
