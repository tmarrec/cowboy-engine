#include "RendererManager.h"

#include "../Window/WindowManager.h"
#include <cstdint>
#include <limits>
#include <shaderc/shaderc.h>
#include <shaderc/shaderc.hpp>
#include <vulkan/vulkan_core.h>

extern WindowManager g_WindowManager;

// Initialize the Renderer manager
RendererManager::RendererManager()
{
    createInstance();
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
    createSwapchain();
    createImageViews();
    createRenderPass();
    createGraphicsPipeline();
    createFramebuffers();
    createCommandPool();
    createCommandBuffers();
    createSyncObjects();
}

// Clean all the objects related to Vulkan
RendererManager::~RendererManager()
{
    for (std::uint8_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        vkDestroySemaphore(_vkDevice, _vkImageAvailableSemaphores[i], VK_NULL_HANDLE);
        vkDestroySemaphore(_vkDevice, _vkRenderFinishedSemaphores[i], VK_NULL_HANDLE);
        vkDestroyFence(_vkDevice, _vkInFlightFences[i], VK_NULL_HANDLE);
    }
    vkDestroyCommandPool(_vkDevice, _vkCommandPool, VK_NULL_HANDLE);
    for (auto& framebuffer : _vkSwapchainFramebuffers)
    {
        vkDestroyFramebuffer(_vkDevice, framebuffer, VK_NULL_HANDLE);
    }
    vkDestroyPipeline(_vkDevice, _vkGraphicsPipeline, VK_NULL_HANDLE);
    vkDestroyPipelineLayout(_vkDevice, _vkPipelineLayout, VK_NULL_HANDLE);
    vkDestroyRenderPass(_vkDevice, _vkRenderPass, VK_NULL_HANDLE);
    for (auto& imageView : _vkSwapchainImageViews)
    {
        vkDestroyImageView(_vkDevice, imageView, VK_NULL_HANDLE);
    }
    vkDestroySwapchainKHR(_vkDevice, _vkSwapchain, VK_NULL_HANDLE);
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
		ERROR_EXIT("Failed to find GPUs with Vulkan support.");
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
		ERROR_EXIT("Failed to find suitable GPU.");
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
    SwapchainSupportDetails swapchainSupport = querySwapchainSupport(device);
    return findQueueFamilies(device).isComplete() 
        && checkDeviceExtensionSupport(device)
        && !swapchainSupport.formats.empty() && !swapchainSupport.presentModes.empty();
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
		ERROR_EXIT("Cannot create Vulkan instance.");
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
		ERROR_EXIT("Failed to create logical Vulkan device.")
	}

	vkGetDeviceQueue(_vkDevice, indices.graphics.value(), 0, &_vkGraphicsQueue);
	vkGetDeviceQueue(_vkDevice, indices.present.value(), 0, &_vkPresentQueue);
}

// Get the details of the device swapchain support
SwapchainSupportDetails RendererManager::querySwapchainSupport(VkPhysicalDevice device)
{
    SwapchainSupportDetails details;
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
    WARNING("Unable to find desired swapchain present mode. Will use VK_PRESENT_MODE_IMMEDIATE_KHR.");
    return VK_PRESENT_MODE_IMMEDIATE_KHR;
}

// Choose the swap extent based on the window manager informations
VkExtent2D RendererManager::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
    if (capabilities.currentExtent.width != UINT32_MAX)
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
void RendererManager::createSwapchain()
{
    // Get the informations needed for the swapchain
    const SwapchainSupportDetails swapchainSupport = querySwapchainSupport(_vkPhysicalDevice);
    const VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapchainSupport.formats);
    const VkPresentModeKHR presentMode = chooseSwapPresentMode(swapchainSupport.presentModes);
    const std::uint32_t imageCount = [&]()
    {
        std::uint32_t imageCount = swapchainSupport.capabilities.minImageCount + 1;
        if (swapchainSupport.capabilities.maxImageCount > 0 && imageCount > swapchainSupport.capabilities.maxImageCount)
        {
            imageCount = swapchainSupport.capabilities.maxImageCount;
        }
        return imageCount;
    }();
    _vkSwapchainFormat = surfaceFormat.format;
    _vkSwapchainExtent = chooseSwapExtent(swapchainSupport.capabilities);

    // Fill the swapchain informations
    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.pNext = VK_NULL_HANDLE;
    createInfo.flags = 0;
    createInfo.surface = _vkSurface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = _vkSwapchainFormat;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = _vkSwapchainExtent;
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

    createInfo.preTransform = swapchainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    // Create the swapchain
    if (vkCreateSwapchainKHR(_vkDevice, &createInfo, VK_NULL_HANDLE, &_vkSwapchain) != VK_SUCCESS)
    {
        ERROR_EXIT("Failed to create swap chain.");
    }

    // Link the swapchain to the vector of images
    std::uint32_t swapchainImageCount;
    vkGetSwapchainImagesKHR(_vkDevice, _vkSwapchain, &swapchainImageCount, VK_NULL_HANDLE);
    _vkSwapchainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(_vkDevice, _vkSwapchain, &swapchainImageCount, _vkSwapchainImages.data());
}

// Create the image view for all swapchain images
void RendererManager::createImageViews()
{
    _vkSwapchainImageViews.resize(_vkSwapchainImages.size());
    for (std::uint8_t i = 0; i < _vkSwapchainImages.size(); ++i)
    {
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.pNext = VK_NULL_HANDLE;
        createInfo.flags = 0;
        createInfo.image = _vkSwapchainImages[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = _vkSwapchainFormat;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(_vkDevice, &createInfo, VK_NULL_HANDLE, &_vkSwapchainImageViews[i]) != VK_SUCCESS)
        {
            ERROR_EXIT("Failed to create image view.");
        }
    }
}

// Create the graphics pipeline
void RendererManager::createGraphicsPipeline()
{
    loadShaders(); 

    VkShaderModule vertShaderModule = createShaderModule(_vertShaderCode);
    VkShaderModule fragShaderModule = createShaderModule(_fragShaderCode);

    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.pNext = VK_NULL_HANDLE;
    vertShaderStageInfo.flags = 0;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";
    vertShaderStageInfo.pSpecializationInfo = VK_NULL_HANDLE;

    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.pNext = VK_NULL_HANDLE;
    fragShaderStageInfo.flags = 0;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";
    fragShaderStageInfo.pSpecializationInfo = VK_NULL_HANDLE;

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    // Vertex input
    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.pNext = VK_NULL_HANDLE;
    vertexInputInfo.flags = 0;
    vertexInputInfo.vertexBindingDescriptionCount = 0;
    vertexInputInfo.pVertexBindingDescriptions = VK_NULL_HANDLE;
    vertexInputInfo.vertexAttributeDescriptionCount = 0;
    vertexInputInfo.pVertexAttributeDescriptions = VK_NULL_HANDLE;

    // Input assembly
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo{};
    inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyInfo.pNext = VK_NULL_HANDLE;
    inputAssemblyInfo.flags = 0;
    inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

    // Viewport and Scissor
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(_vkSwapchainExtent.width);
    viewport.height = static_cast<float>(_vkSwapchainExtent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 0.0f;

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = _vkSwapchainExtent;

    VkPipelineViewportStateCreateInfo viewportStateInfo{};
    viewportStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportStateInfo.pNext = VK_NULL_HANDLE;
    viewportStateInfo.flags = 0;
    viewportStateInfo.viewportCount = 1;
    viewportStateInfo.pViewports = &viewport;
    viewportStateInfo.scissorCount = 1;
    viewportStateInfo.pScissors = &scissor;

    // Rasterizer
    VkPipelineRasterizationStateCreateInfo rasterizerInfo{};
    rasterizerInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizerInfo.pNext = VK_NULL_HANDLE;
    rasterizerInfo.flags = 0;
    rasterizerInfo.depthClampEnable = VK_FALSE;
    rasterizerInfo.rasterizerDiscardEnable = VK_FALSE;
    rasterizerInfo.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizerInfo.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizerInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizerInfo.depthBiasEnable = VK_FALSE;
    rasterizerInfo.depthBiasConstantFactor = 0.0f;
    rasterizerInfo.depthBiasClamp = 0.0f;
    rasterizerInfo.depthBiasSlopeFactor = 0.0f;
    rasterizerInfo.lineWidth = 1.0f;

    // Multisampling
    VkPipelineMultisampleStateCreateInfo multisamplingInfo{};
    multisamplingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisamplingInfo.pNext = VK_NULL_HANDLE;
    multisamplingInfo.flags = 0;
    multisamplingInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisamplingInfo.sampleShadingEnable = VK_FALSE;
    multisamplingInfo.minSampleShading = 1.0f;
    multisamplingInfo.pSampleMask = VK_NULL_HANDLE;
    multisamplingInfo.alphaToCoverageEnable = VK_FALSE;
    multisamplingInfo.alphaToOneEnable = VK_FALSE;

    // Color blending
    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.blendEnable = VK_TRUE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    VkPipelineColorBlendStateCreateInfo colorBlendingInfo{};
    colorBlendingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendingInfo.pNext = VK_NULL_HANDLE;
    colorBlendingInfo.flags = 0;
    colorBlendingInfo.logicOpEnable = VK_FALSE;
    colorBlendingInfo.logicOp = VK_LOGIC_OP_COPY;
    colorBlendingInfo.attachmentCount = 1;
    colorBlendingInfo.pAttachments = &colorBlendAttachment;
    colorBlendingInfo.blendConstants[0] = 0.0f;
    colorBlendingInfo.blendConstants[1] = 0.0f;
    colorBlendingInfo.blendConstants[2] = 0.0f;
    colorBlendingInfo.blendConstants[3] = 0.0f;

    // Pipeline layout
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.pNext = VK_NULL_HANDLE;
    pipelineLayoutInfo.flags = 0;
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pSetLayouts = VK_NULL_HANDLE;
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.pPushConstantRanges = VK_NULL_HANDLE;

    if (vkCreatePipelineLayout(_vkDevice, &pipelineLayoutInfo, VK_NULL_HANDLE, &_vkPipelineLayout) != VK_SUCCESS)
    {
        ERROR("Failed to create pipeline layout.");
    }

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.pNext = VK_NULL_HANDLE;
    pipelineInfo.flags = 0;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssemblyInfo;
    pipelineInfo.pTessellationState = VK_NULL_HANDLE;
    pipelineInfo.pViewportState = &viewportStateInfo;
    pipelineInfo.pRasterizationState = &rasterizerInfo;
    pipelineInfo.pMultisampleState = &multisamplingInfo;
    pipelineInfo.pDepthStencilState = VK_NULL_HANDLE;
    pipelineInfo.pColorBlendState = &colorBlendingInfo;
    pipelineInfo.pDynamicState = VK_NULL_HANDLE;
    pipelineInfo.layout = _vkPipelineLayout;
    pipelineInfo.renderPass = _vkRenderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex = -1;

    if (vkCreateGraphicsPipelines(_vkDevice, VK_NULL_HANDLE, 1, &pipelineInfo, VK_NULL_HANDLE, &_vkGraphicsPipeline) != VK_SUCCESS)
    {
        ERROR("Failed to create graphics pipeline.");
    }

    vkDestroyShaderModule(_vkDevice, vertShaderModule, VK_NULL_HANDLE);
    vkDestroyShaderModule(_vkDevice, fragShaderModule, VK_NULL_HANDLE);
}

// load shaders from string to bytecode
void RendererManager::loadShaders()
{
    // Vertex shader
    const char vertGlslString[] = "#version 460 core\n                  \
        layout(location = 0) out vec3 fragColor;                        \
                                                                        \
        vec2 positions[3] = vec2[](                                     \
            vec2(0.0, -0.5),                                            \
            vec2(0.5, 0.5),                                             \
            vec2(-0.5, 0.5)                                             \
        );                                                              \
                                                                        \
        vec3 colors[3] = vec3[](                                        \
            vec3(1.0, 0.0, 0.0),                                        \
            vec3(0.0, 1.0, 0.0),                                        \
            vec3(0.0, 0.0, 1.0)                                         \
        );                                                              \
                                                                        \
        void main() {                                                   \
            gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);    \
            fragColor = colors[gl_VertexIndex];                         \
        }                                                               \
    ";
    shaderc::Compiler compiler;
    shaderc::CompileOptions options;
    options.SetOptimizationLevel(shaderc_optimization_level_performance);

    // Preprocess
    shaderc::PreprocessedSourceCompilationResult preprocessed = compiler.PreprocessGlsl(vertGlslString, shaderc_glsl_vertex_shader, "source_name", options);
    if (preprocessed.GetCompilationStatus() != shaderc_compilation_status_success)
    {
        ERROR(preprocessed.GetErrorMessage());
        ERROR_EXIT("Shader preprocess failed.");
    }
    
    // Compile
    shaderc::CompilationResult assembly = compiler.CompileGlslToSpv(vertGlslString, shaderc_glsl_vertex_shader, "source_name", options);
    if (assembly.GetCompilationStatus() != shaderc_compilation_status_success)
    {
        ERROR(assembly.GetErrorMessage());
        ERROR_EXIT("Shader compilation failed.");
    }
    
    _vertShaderCode = {assembly.cbegin(), assembly.cend()};

    // Fragment shader
    const char fragGlslString[] = "#version 460 core\n  \
                                                        \
        layout(location = 0) in vec3 fragColor;         \
                                                        \
        layout(location = 0) out vec4 outColor;         \
                                                        \
        void main() {                                   \
            outColor = vec4(fragColor, 1.0);            \
        }                                               \
    ";
    shaderc::Compiler compiler2;
    shaderc::CompileOptions options2;
    options2.SetOptimizationLevel(shaderc_optimization_level_performance);

    // Preprocess
    shaderc::PreprocessedSourceCompilationResult preprocessed2 = compiler2.PreprocessGlsl(fragGlslString, shaderc_glsl_fragment_shader, "source_name", options2);
    if (preprocessed2.GetCompilationStatus() != shaderc_compilation_status_success)
    {
        ERROR(preprocessed2.GetErrorMessage());
        ERROR_EXIT("Shader preprocess failed.");
    }
    
    // Compile
    shaderc::CompilationResult assembly2 = compiler2.CompileGlslToSpv(fragGlslString, shaderc_glsl_fragment_shader, "source_name", options2);
    if (assembly2.GetCompilationStatus() != shaderc_compilation_status_success)
    {
        ERROR(assembly2.GetErrorMessage());
        ERROR_EXIT("Shader compilation failed.");
    }
    
    _fragShaderCode = {assembly2.cbegin(), assembly2.cend()};
}

// Create Vulkan shader module from shader bytecode
VkShaderModule RendererManager::createShaderModule(const std::vector<std::uint32_t>& code)
{
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.pNext = VK_NULL_HANDLE;
    createInfo.flags = 0;
    createInfo.codeSize = code.size() * sizeof(std::uint32_t);
    createInfo.pCode = code.data();

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(_vkDevice, &createInfo, VK_NULL_HANDLE, &shaderModule) != VK_SUCCESS)
    {
        ERROR_EXIT("Failed to create shader module.");
    }

    return shaderModule;
}

// Create the Vulkan render pass
void RendererManager::createRenderPass()
{
    VkAttachmentDescription colorAttachment{};
    colorAttachment.flags = 0;
    colorAttachment.format = _vkSwapchainFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.flags = 0;
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.inputAttachmentCount = 0;
    subpass.pInputAttachments = VK_NULL_HANDLE;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pResolveAttachments = VK_NULL_HANDLE;
    subpass.pDepthStencilAttachment = VK_NULL_HANDLE;
    subpass.preserveAttachmentCount = 0;
    subpass.pPreserveAttachments = VK_NULL_HANDLE;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT; 

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.pNext = VK_NULL_HANDLE;
    renderPassInfo.flags = 0;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(_vkDevice, &renderPassInfo, VK_NULL_HANDLE, &_vkRenderPass) != VK_SUCCESS)
    {
        ERROR_EXIT("Failed to create render pass.");
    }
}

// Create all the framebuffers needed
void RendererManager::createFramebuffers()
{
    _vkSwapchainFramebuffers.resize(_vkSwapchainImageViews.size());
    for (std::uint64_t i = 0; i < _vkSwapchainImageViews.size(); ++i)
    {
        VkImageView attachments[] =
        {
            _vkSwapchainImageViews[i]
        };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.pNext = VK_NULL_HANDLE;
        framebufferInfo.flags = 0;
        framebufferInfo.renderPass = _vkRenderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = _vkSwapchainExtent.width;
        framebufferInfo.height = _vkSwapchainExtent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(_vkDevice, &framebufferInfo, VK_NULL_HANDLE, &_vkSwapchainFramebuffers[i]) != VK_SUCCESS)
        {
            ERROR_EXIT("Failed to create framebuffer.");
        }
    }
}

// Create the Vulkan command pool
void RendererManager::createCommandPool()
{
    QueueFamilyIndices queueFamilyIndices = findQueueFamilies(_vkPhysicalDevice);

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.pNext = VK_NULL_HANDLE;
    poolInfo.flags = 0;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphics.value();

    if (vkCreateCommandPool(_vkDevice, &poolInfo, VK_NULL_HANDLE, &_vkCommandPool) != VK_SUCCESS)
    {
        ERROR_EXIT("Failed to create command pool.");
    }
}

// Create all the Vulkan command buffers and initialize them
void RendererManager::createCommandBuffers()
{
    _vkCommandBuffers.resize(_vkSwapchainFramebuffers.size());

    VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
    commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocateInfo.pNext = VK_NULL_HANDLE;
    commandBufferAllocateInfo.commandPool = _vkCommandPool;
    commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferAllocateInfo.commandBufferCount = static_cast<std::uint32_t>(_vkCommandBuffers.size());

    if (vkAllocateCommandBuffers(_vkDevice, &commandBufferAllocateInfo, _vkCommandBuffers.data()) != VK_SUCCESS)
    {
        ERROR_EXIT("Failed to create command buffers.");
    }

    for (std::uint64_t i = 0; i < _vkCommandBuffers.size(); ++i)
    {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.pNext = VK_NULL_HANDLE;
        beginInfo.flags = 0;
        beginInfo.pInheritanceInfo = VK_NULL_HANDLE;

        if (vkBeginCommandBuffer(_vkCommandBuffers[i], &beginInfo) != VK_SUCCESS)
        {
            ERROR_EXIT("Failed to begin recording command buffer.");
        }

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.pNext = VK_NULL_HANDLE;
        renderPassInfo.renderPass = _vkRenderPass;
        renderPassInfo.framebuffer = _vkSwapchainFramebuffers[i];
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = _vkSwapchainExtent;
        VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearColor;

        vkCmdBeginRenderPass(_vkCommandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindPipeline(_vkCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, _vkGraphicsPipeline);

        vkCmdDraw(_vkCommandBuffers[i], 3, 1, 0, 0);

        vkCmdEndRenderPass(_vkCommandBuffers[i]);

        if (vkEndCommandBuffer(_vkCommandBuffers[i]) != VK_SUCCESS)
        {
            ERROR_EXIT("Failed to record command buffer.");
        }
    }
}

// Create all the synchronisation objects for the drawFrame
void RendererManager::createSyncObjects()
{
    _vkImageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    _vkRenderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    _vkInFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
    _vkImagesInFlight.resize(_vkSwapchainImages.size(), VK_NULL_HANDLE);


    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semaphoreInfo.pNext = VK_NULL_HANDLE;
    semaphoreInfo.flags = 0;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.pNext = VK_NULL_HANDLE;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (std::uint8_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        if (
                vkCreateSemaphore(_vkDevice, &semaphoreInfo, VK_NULL_HANDLE, &_vkImageAvailableSemaphores[i]) != VK_SUCCESS
            ||  vkCreateSemaphore(_vkDevice, &semaphoreInfo, VK_NULL_HANDLE, &_vkRenderFinishedSemaphores[i]) != VK_SUCCESS
            ||  vkCreateFence(_vkDevice, &fenceInfo, VK_NULL_HANDLE, &_vkInFlightFences[i]) != VK_SUCCESS
        )
        {
            ERROR_EXIT("Failed to create synchronisation objects.");
        }
    }
}

// Draw the frame by executing the queues while staying synchronised
void RendererManager::drawFrame()
{
    vkWaitForFences(_vkDevice, 1, &_vkInFlightFences[_currentFrame], VK_TRUE, UINT64_MAX);

    std::uint32_t imageIndex;
    vkAcquireNextImageKHR(_vkDevice, _vkSwapchain, UINT64_MAX, _vkImageAvailableSemaphores[_currentFrame], VK_NULL_HANDLE, &imageIndex);

    // Check if a previous frame is using this image
    if (_vkImagesInFlight[imageIndex] != VK_NULL_HANDLE)
    {
        vkWaitForFences(_vkDevice, 1, &_vkImagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
    }
    // Mark the image as now being in use by this frame
    _vkImagesInFlight[imageIndex] = _vkInFlightFences[_currentFrame];

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pNext = VK_NULL_HANDLE;

    VkSemaphore waitSemaphores[] = {_vkImageAvailableSemaphores[_currentFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &_vkCommandBuffers[imageIndex];

    VkSemaphore signalSemaphores[] = {_vkRenderFinishedSemaphores[_currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    vkResetFences(_vkDevice, 1, &_vkInFlightFences[_currentFrame]);
    if (vkQueueSubmit(_vkGraphicsQueue, 1, &submitInfo, _vkInFlightFences[_currentFrame]) != VK_SUCCESS)
    {
        ERROR_EXIT("Failed to submit draw command buffer.");
    }

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pNext = VK_NULL_HANDLE;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapchains[] = {_vkSwapchain};
    presentInfo.swapchainCount= 1;
    presentInfo.pSwapchains = swapchains;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = VK_NULL_HANDLE;

    vkQueuePresentKHR(_vkPresentQueue, &presentInfo);

    vkQueueWaitIdle(_vkPresentQueue);

    _currentFrame = (_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

// Wait that the Vulkan device is idle
void RendererManager::waitIdle()
{
    vkDeviceWaitIdle(_vkDevice);
}
