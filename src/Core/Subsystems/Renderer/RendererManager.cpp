#include "RendererManager.h"

#include "../Window/WindowManager.h"
#include <chrono>
#include <cstdint>
#include <vulkan/vulkan_core.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

extern WindowManager g_WindowManager;

// Initialize the Renderer manager
RendererManager::RendererManager()
{
    createInstance();
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
    createSwapchain();
    createImages();
    createImageViews();
    createRenderPass();
    createDescriptorSetLayout();
    updateGraphicsPipeline();
    createFramebuffers();
    createCommandPool();
    createTextureImage();
    createTextureImageView();
    createTextureSampler();
    createVertexBuffer();
    createIndexBuffer();
    createUniformBuffers();
    createDescriptorPool();
    createDescriptorSets();
    allocateCommandBuffers();
    createSyncObjects();
}

// Clean all the objects related to Vulkan
RendererManager::~RendererManager()
{
    for (auto& framebuffer : _vkSwapchainFramebuffers)
    {
        vkDestroyFramebuffer(_vkDevice, framebuffer, VK_NULL_HANDLE);
    }
    vkFreeCommandBuffers(_vkDevice, _vkCommandPool, static_cast<std::uint32_t>(_vkCommandBuffers.size()), _vkCommandBuffers.data());
    vkDestroyPipeline(_vkDevice, _vkGraphicsPipeline, VK_NULL_HANDLE);
    vkDestroyPipelineLayout(_vkDevice, _vkPipelineLayout, VK_NULL_HANDLE);
    vkDestroyRenderPass(_vkDevice, _vkRenderPass, VK_NULL_HANDLE);
    for (auto& imageView : _vkSwapchainImageViews)
    {
        vkDestroyImageView(_vkDevice, imageView, VK_NULL_HANDLE);
    }
    vkDestroySwapchainKHR(_vkDevice, _swapchain(), VK_NULL_HANDLE);

    for (std::uint8_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        vkDestroySemaphore(_vkDevice, _vkImageAvailableSemaphores[i], VK_NULL_HANDLE);
        vkDestroySemaphore(_vkDevice, _vkRenderFinishedSemaphores[i], VK_NULL_HANDLE);
        vkDestroyFence(_vkDevice, _vkInFlightFences[i], VK_NULL_HANDLE);
    }

    vkDestroyDescriptorSetLayout(_vkDevice, _vkDescriptorSetLayout, VK_NULL_HANDLE);
    vkDestroyDescriptorPool(_vkDevice, _vkDescriptorPool, VK_NULL_HANDLE);

    for (size_t i = 0; i < _vkSwapchainImages.size(); ++i)
    {
        vkDestroyBuffer(_vkDevice, _uniformBuffers[i], VK_NULL_HANDLE);
        vkFreeMemory(_vkDevice, _uniformBuffersMemory[i], VK_NULL_HANDLE);
    }

    vkDestroyBuffer(_vkDevice, _vkVertexBuffer, VK_NULL_HANDLE);
    vkFreeMemory(_vkDevice, _vkVertexBufferMemory, VK_NULL_HANDLE);
    vkDestroyBuffer(_vkDevice, _vkIndexBuffer, VK_NULL_HANDLE);
    vkFreeMemory(_vkDevice, _vkIndexBufferMemory, VK_NULL_HANDLE);

    vkDestroySampler(_vkDevice, _textureSampler, VK_NULL_HANDLE);

    vkDestroyImageView(_vkDevice, _textureImageView, VK_NULL_HANDLE);
    vkDestroyImage(_vkDevice, _textureImage, VK_NULL_HANDLE);
    vkFreeMemory(_vkDevice, _textureImageMemory, VK_NULL_HANDLE);

    vkDestroyCommandPool(_vkDevice, _vkCommandPool, VK_NULL_HANDLE);
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
    const SwapchainSupportDetails swapchainSupport = _swapchain.querySupport(device, _vkSurface);
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
    const std::pair<const char**, std::uint32_t> windowRequiredInstanceExt = g_WindowManager.windowGetRequiredInstanceExtensions();
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
	    .enabledExtensionCount = static_cast<std::uint32_t>(vkExtensions.size()),
	    .ppEnabledExtensionNames = vkExtensions.data(),
    };

    // Specify those required extensions to the Vulkan instance
	if (vkCreateInstance(&instanceInfo, VK_NULL_HANDLE, &_vkInstance))
	{
		ERROR_EXIT("Cannot create Vulkan instance.");
	}
}

// Create the logical Vulkan device
void RendererManager::createLogicalDevice()
{
    const QueueFamilyIndices indices = findQueueFamilies(_vkPhysicalDevice);
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

    // Logical device informations
	const VkDeviceCreateInfo createInfo =
    {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = VK_NULL_HANDLE,
        .flags = 0,
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = &queueCreateInfo,
        .enabledLayerCount = 0,
        .enabledExtensionCount = static_cast<std::uint32_t>(_deviceExtensions.size()),
        .ppEnabledExtensionNames = _deviceExtensions.data(),
        .pEnabledFeatures = &deviceFeatures,
    };

	if (vkCreateDevice(_vkPhysicalDevice, &createInfo, VK_NULL_HANDLE, &_vkDevice) != VK_SUCCESS)
	{
		ERROR_EXIT("Failed to create logical Vulkan device.")
	}

	vkGetDeviceQueue(_vkDevice, indices.graphics.value(), 0, &_vkGraphicsQueue);
	vkGetDeviceQueue(_vkDevice, indices.present.value(), 0, &_vkPresentQueue);
}

// Create the Vulkan swapchain
void RendererManager::createSwapchain()
{
    _swapchain.create(_vkPhysicalDevice, _vkDevice, _vkSurface, findQueueFamilies(_vkPhysicalDevice));
}

// Create the images for the swapchain
void RendererManager::createImages()
{
    // Link the swapchain to the vector of images
    std::uint32_t swapchainImageCount;
    const VkSwapchainKHR vkSwapchain = _swapchain();
    vkGetSwapchainImagesKHR(_vkDevice, vkSwapchain, &swapchainImageCount, VK_NULL_HANDLE);
    _vkSwapchainImages.resize(_swapchain.imageCount());
    vkGetSwapchainImagesKHR(_vkDevice, vkSwapchain, &swapchainImageCount, _vkSwapchainImages.data());
}

// Create the image view for all swapchain images
void RendererManager::createImageViews()
{
    _vkSwapchainImageViews.resize(_vkSwapchainImages.size());
    for (size_t i = 0; i < _vkSwapchainImages.size(); ++i)
    {
        _vkSwapchainImageViews[i] = createImageView(_vkSwapchainImages[i], _swapchain.format());
    }
}

// Create the graphics pipeline
void RendererManager::createGraphicsPipeline()
{
    _vertShader.compile();
    _fragShader.compile();
    const VkShaderModule vertShaderModule = createShaderModule(_vertShader.code());
    const VkShaderModule fragShaderModule = createShaderModule(_fragShader.code());

    const VkPipelineShaderStageCreateInfo vertShaderStageInfo =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .pNext = VK_NULL_HANDLE,
        .flags = 0,
        .stage = VK_SHADER_STAGE_VERTEX_BIT,
        .module = vertShaderModule,
        .pName = "main",
        .pSpecializationInfo = VK_NULL_HANDLE,
    };

    const VkPipelineShaderStageCreateInfo fragShaderStageInfo =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .pNext = VK_NULL_HANDLE,
        .flags = 0,
        .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
        .module = fragShaderModule,
        .pName = "main",
        .pSpecializationInfo = VK_NULL_HANDLE,
    };

    const VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    // Vertex input
    const auto bindingDescription = Vertex::getBindingDescription();
    const auto attributeDescription = Vertex::getAttributeDescriptions();

    const VkPipelineVertexInputStateCreateInfo vertexInputInfo =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .pNext = VK_NULL_HANDLE,
        .flags = 0,
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions = &bindingDescription,
        .vertexAttributeDescriptionCount = static_cast<std::uint32_t>(attributeDescription.size()),
        .pVertexAttributeDescriptions = attributeDescription.data(),
    };

    // Input assembly
    const VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .pNext = VK_NULL_HANDLE,
        .flags = 0,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE,
    };

    // Viewport and Scissor
    const VkExtent2D swapchainExtent = _swapchain.extent();
    const VkViewport viewport =
    {
        .x = 0.0f,
        .y = 0.0f,
        .width = static_cast<float>(swapchainExtent.width),
        .height = static_cast<float>(swapchainExtent.height),
        .minDepth = 0.0f,
        .maxDepth = 0.0f,
    };

    const VkRect2D scissor =
    {
        .offset = {0, 0},
        .extent = swapchainExtent,
    };

    const VkPipelineViewportStateCreateInfo viewportStateInfo =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .pNext = VK_NULL_HANDLE,
        .flags = 0,
        .viewportCount = 1,
        .pViewports = &viewport,
        .scissorCount = 1,
        .pScissors = &scissor,
    };

    // Rasterizer
    const VkPipelineRasterizationStateCreateInfo rasterizerInfo =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .pNext = VK_NULL_HANDLE,
        .flags = 0,
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .cullMode = VK_CULL_MODE_BACK_BIT,
        .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
        .depthBiasEnable = VK_FALSE,
        .depthBiasConstantFactor = 0.0f,
        .depthBiasClamp = 0.0f,
        .depthBiasSlopeFactor = 0.0f,
        .lineWidth = 1.0f,
    };

    // Multisampling
    const VkPipelineMultisampleStateCreateInfo multisamplingInfo =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .pNext = VK_NULL_HANDLE,
        .flags = 0,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
        .sampleShadingEnable = VK_FALSE,
        .minSampleShading = 1.0f,
        .pSampleMask = VK_NULL_HANDLE,
        .alphaToCoverageEnable = VK_FALSE,
        .alphaToOneEnable = VK_FALSE,
    };

    // Color blending
    const VkPipelineColorBlendAttachmentState colorBlendAttachment =
    {
        .blendEnable = VK_TRUE,
        .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
        .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
        .colorBlendOp = VK_BLEND_OP_ADD,
        .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
        .alphaBlendOp = VK_BLEND_OP_ADD,
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
    };

    const VkPipelineColorBlendStateCreateInfo colorBlendingInfo =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .pNext = VK_NULL_HANDLE,
        .flags = 0,
        .logicOpEnable = VK_FALSE,
        .logicOp = VK_LOGIC_OP_COPY,
        .attachmentCount = 1,
        .pAttachments = &colorBlendAttachment,
        .blendConstants = {0.0f, 0.0f, 0.0f, 0.0f},
    }; 

    // Dynamic state to handle window resize
    const std::array<VkDynamicState, 2> dynamicStates =
    {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    const VkPipelineDynamicStateCreateInfo dynamicStateInfo =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .pNext = VK_NULL_HANDLE,
        .flags = 0,
        .dynamicStateCount = 2,
        .pDynamicStates = dynamicStates.data(),
    };

    // Pipeline layout
    const VkPipelineLayoutCreateInfo pipelineLayoutInfo =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pNext = VK_NULL_HANDLE,
        .flags = 0,
        .setLayoutCount = 1,
        .pSetLayouts = &_vkDescriptorSetLayout,
        .pushConstantRangeCount = 0,
        .pPushConstantRanges = VK_NULL_HANDLE,
    };

    if (vkCreatePipelineLayout(_vkDevice, &pipelineLayoutInfo, VK_NULL_HANDLE, &_vkPipelineLayout) != VK_SUCCESS)
    {
        ERROR("Failed to create pipeline layout.");
    }

    const VkGraphicsPipelineCreateInfo pipelineInfo =
    {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext = VK_NULL_HANDLE,
        .flags = 0,
        .stageCount = 2,
        .pStages = shaderStages,
        .pVertexInputState = &vertexInputInfo,
        .pInputAssemblyState = &inputAssemblyInfo,
        .pTessellationState = VK_NULL_HANDLE,
        .pViewportState = &viewportStateInfo,
        .pRasterizationState = &rasterizerInfo,
        .pMultisampleState = &multisamplingInfo,
        .pDepthStencilState = VK_NULL_HANDLE,
        .pColorBlendState = &colorBlendingInfo,
        .pDynamicState = &dynamicStateInfo,
        .layout = _vkPipelineLayout,
        .renderPass = _vkRenderPass,
        .subpass = 0,
        .basePipelineHandle = VK_NULL_HANDLE,
        .basePipelineIndex = -1,
    };

    if (vkCreateGraphicsPipelines(_vkDevice, VK_NULL_HANDLE, 1, &pipelineInfo, VK_NULL_HANDLE, &_vkGraphicsPipeline) != VK_SUCCESS)
    {
        ERROR("Failed to create graphics pipeline.");
    }

    vkDestroyShaderModule(_vkDevice, vertShaderModule, VK_NULL_HANDLE);
    vkDestroyShaderModule(_vkDevice, fragShaderModule, VK_NULL_HANDLE);
}

// Update the Vulkan graphics pipeline
void RendererManager::updateGraphicsPipeline()
{
    createGraphicsPipeline();
}

// Create Vulkan shader module from shader bytecode
VkShaderModule RendererManager::createShaderModule(const std::vector<std::uint32_t>& code)
{
    const VkShaderModuleCreateInfo createInfo =
    {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .pNext = VK_NULL_HANDLE,
        .flags = 0,
        .codeSize = code.size() * sizeof(std::uint32_t),
        .pCode = code.data(),
    };

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
    const VkAttachmentDescription colorAttachment =
    {
        .flags = 0,
        .format = _swapchain.format(),
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
    };

    const VkAttachmentReference colorAttachmentRef =
    {
        .attachment = 0,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    };

    const VkSubpassDescription subpass =
    {
        .flags = 0,
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .inputAttachmentCount = 0,
        .pInputAttachments = VK_NULL_HANDLE,
        .colorAttachmentCount = 1,
        .pColorAttachments = &colorAttachmentRef,
        .pResolveAttachments = VK_NULL_HANDLE,
        .pDepthStencilAttachment = VK_NULL_HANDLE,
        .preserveAttachmentCount = 0,
        .pPreserveAttachments = VK_NULL_HANDLE,
    };

    const VkSubpassDependency dependency =
    {
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .dstSubpass = 0,
        .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .srcAccessMask = 0,
        .dstAccessMask= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
    };

    const VkRenderPassCreateInfo renderPassInfo =
    {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .pNext = VK_NULL_HANDLE,
        .flags = 0,
        .attachmentCount = 1,
        .pAttachments = &colorAttachment,
        .subpassCount = 1,
        .pSubpasses = &subpass,
        .dependencyCount = 1,
        .pDependencies = &dependency,
    };

    if (vkCreateRenderPass(_vkDevice, &renderPassInfo, VK_NULL_HANDLE, &_vkRenderPass) != VK_SUCCESS)
    {
        ERROR_EXIT("Failed to create render pass.");
    }
}

// Create all the framebuffers needed
void RendererManager::createFramebuffers()
{
    _vkSwapchainFramebuffers.resize(_vkSwapchainImageViews.size());
    for (size_t i = 0; i < _vkSwapchainImageViews.size(); ++i)
    {
        const std::array<VkImageView, 1> attachments =
        {
            _vkSwapchainImageViews[i]
        };

        const VkExtent2D swapchainExtent = _swapchain.extent();
        const VkFramebufferCreateInfo framebufferInfo =
        {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .pNext = VK_NULL_HANDLE,
            .flags = 0,
            .renderPass = _vkRenderPass,
            .attachmentCount = 1,
            .pAttachments = attachments.data(),
            .width = swapchainExtent.width,
            .height = swapchainExtent.height,
            .layers = 1,
        };

        if (vkCreateFramebuffer(_vkDevice, &framebufferInfo, VK_NULL_HANDLE, &_vkSwapchainFramebuffers[i]) != VK_SUCCESS)
        {
            ERROR_EXIT("Failed to create framebuffer.");
        }
    }
}

// Create the Vulkan command pool
void RendererManager::createCommandPool()
{
    const QueueFamilyIndices queueFamilyIndices = findQueueFamilies(_vkPhysicalDevice);

    const VkCommandPoolCreateInfo poolInfo =
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = VK_NULL_HANDLE,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = queueFamilyIndices.graphics.value(),
    };

    if (vkCreateCommandPool(_vkDevice, &poolInfo, VK_NULL_HANDLE, &_vkCommandPool) != VK_SUCCESS)
    {
        ERROR_EXIT("Failed to create command pool.");
    }
}

// Create all the Vulkan command buffers and initialize them
void RendererManager::allocateCommandBuffers()
{
    _vkCommandBuffers.resize(_vkSwapchainFramebuffers.size());

    const VkCommandBufferAllocateInfo commandBufferAllocateInfo =
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = VK_NULL_HANDLE,
        .commandPool = _vkCommandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = static_cast<std::uint32_t>(_vkCommandBuffers.size()),
    };

    if (vkAllocateCommandBuffers(_vkDevice, &commandBufferAllocateInfo, _vkCommandBuffers.data()) != VK_SUCCESS)
    {
        ERROR_EXIT("Failed to create command buffers.");
    }
}

// Create all the synchronisation objects for the drawFrame
void RendererManager::createSyncObjects()
{
    _vkImageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    _vkRenderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    _vkInFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
    _vkImagesInFlight.resize(_vkSwapchainImages.size(), VK_NULL_HANDLE);


    const VkSemaphoreCreateInfo semaphoreInfo =
    {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = VK_NULL_HANDLE,
        .flags = 0,
    };

    const VkFenceCreateInfo fenceInfo =
    {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .pNext = VK_NULL_HANDLE,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT,
    };

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

void RendererManager::createCommandBuffer(const std::uint32_t commandBufferIndex)
{
    const VkCommandBufferBeginInfo beginInfo =
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = VK_NULL_HANDLE,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        .pInheritanceInfo = VK_NULL_HANDLE,
    };

    if (vkBeginCommandBuffer(_vkCommandBuffers[commandBufferIndex], &beginInfo) != VK_SUCCESS)
    {
        ERROR_EXIT("Failed to begin recording command buffer.");
    }

    const VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    const VkRenderPassBeginInfo renderPassInfo =
    {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .pNext = VK_NULL_HANDLE,
        .renderPass = _vkRenderPass,
        .framebuffer = _vkSwapchainFramebuffers[commandBufferIndex],
        .renderArea = 
        {
            .offset = {0, 0},
            .extent = _swapchain.extent(),
        },
        .clearValueCount = 1,
        .pClearValues = &clearColor,
    };

    vkCmdBeginRenderPass(_vkCommandBuffers[commandBufferIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    // Viewport and Scissor
    std::uint32_t width, height;
    g_WindowManager.windowGetFramebufferSize(width, height);
    const VkViewport viewport =
    {
        .x = 0.0f,
        .y = 0.0f,
        .width = static_cast<float>(width),
        .height = static_cast<float>(height),
        .minDepth = 0.0f,
        .maxDepth = 0.0f,
    };
    vkCmdSetViewport(_vkCommandBuffers[commandBufferIndex], 0, 1, &viewport);

    const VkRect2D scissor =
    {
        .offset = {0, 0},
        .extent = _swapchain.extent(),
    };
    vkCmdSetScissor(_vkCommandBuffers[commandBufferIndex], 0, 1, &scissor);

    vkCmdBindPipeline(_vkCommandBuffers[commandBufferIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, _vkGraphicsPipeline);

    const std::array<VkBuffer, 1> vertexBuffers = {_vkVertexBuffer};
    const std::array<VkDeviceSize, 1> offsets = {0};
    vkCmdBindVertexBuffers(_vkCommandBuffers[commandBufferIndex], 0, 1, vertexBuffers.data(), offsets.data());

    vkCmdBindIndexBuffer(_vkCommandBuffers[commandBufferIndex], _vkIndexBuffer, 0, VK_INDEX_TYPE_UINT16);

    vkCmdBindDescriptorSets(_vkCommandBuffers[commandBufferIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, _vkPipelineLayout, 0, 1, &_vkDescriptorSets[commandBufferIndex], 0, VK_NULL_HANDLE);

    vkCmdDrawIndexed(_vkCommandBuffers[commandBufferIndex], static_cast<std::uint32_t>(_indices.size()), 1, 0, 0, 0);

    vkCmdEndRenderPass(_vkCommandBuffers[commandBufferIndex]);

    if (vkEndCommandBuffer(_vkCommandBuffers[commandBufferIndex]) != VK_SUCCESS)
    {
        ERROR_EXIT("Failed to record command buffer.");
    }
}

// Draw the frame by executing the queues while staying synchronised
void RendererManager::drawFrame()
{
    vkWaitForFences(_vkDevice, 1, &_vkInFlightFences[_currentFrame], VK_TRUE, UINT64_MAX);

    std::uint32_t imageIndex;
    VkResult acquireNextImageResult = vkAcquireNextImageKHR(_vkDevice, _swapchain(), UINT64_MAX, _vkImageAvailableSemaphores[_currentFrame], VK_NULL_HANDLE, &imageIndex);
    if (acquireNextImageResult == VK_ERROR_OUT_OF_DATE_KHR)
    {
        // Out of date image, the window has probably been resized
        // Need to update the swapchain, imageviews and framebuffers
        vkDeviceWaitIdle(_vkDevice);

        for (auto& framebuffer : _vkSwapchainFramebuffers)
        {
            vkDestroyFramebuffer(_vkDevice, framebuffer, VK_NULL_HANDLE);
        }
        for (auto& imageView : _vkSwapchainImageViews)
        {
            vkDestroyImageView(_vkDevice, imageView, VK_NULL_HANDLE);
        }

        createSwapchain();
        createImages();
        createImageViews();
        createFramebuffers();
        return;
    }
    else if (acquireNextImageResult != VK_SUCCESS && acquireNextImageResult != VK_SUBOPTIMAL_KHR)
    {
        ERROR_EXIT("Failed to acquire next image");
    }

    // Check if a previous frame is using this image
    if (_vkImagesInFlight[imageIndex] != VK_NULL_HANDLE)
    {
        vkWaitForFences(_vkDevice, 1, &_vkImagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
    }
    // Mark the image as now being in use by this frame
    _vkImagesInFlight[imageIndex] = _vkInFlightFences[_currentFrame];

    createCommandBuffer(imageIndex);
    updateUniformBuffer(imageIndex);

    const std::array<VkSemaphore, 1> waitSemaphores = {_vkImageAvailableSemaphores[_currentFrame]};
    const std::array<VkPipelineStageFlags, 1> waitStages = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    const std::array<VkSemaphore, 1> signalSemaphores = {_vkRenderFinishedSemaphores[_currentFrame]};

    const VkSubmitInfo submitInfo =
    {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pNext = VK_NULL_HANDLE,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = waitSemaphores.data(),
        .pWaitDstStageMask = waitStages.data(),
        .commandBufferCount = 1,
        .pCommandBuffers = &_vkCommandBuffers[imageIndex],
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = signalSemaphores.data(),
    };

    vkResetFences(_vkDevice, 1, &_vkInFlightFences[_currentFrame]);

    if (vkQueueSubmit(_vkGraphicsQueue, 1, &submitInfo, _vkInFlightFences[_currentFrame]) != VK_SUCCESS)
    {
        ERROR_EXIT("Failed to submit draw command buffer.");
    }

    const std::array<VkSwapchainKHR, 1> swapchains = {_swapchain()};
    const VkPresentInfoKHR presentInfo =
    {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .pNext = VK_NULL_HANDLE,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = signalSemaphores.data(),
        .swapchainCount= 1,
        .pSwapchains = swapchains.data(),
        .pImageIndices = &imageIndex,
        .pResults = VK_NULL_HANDLE,
    };

    vkQueuePresentKHR(_vkPresentQueue, &presentInfo);

    vkQueueWaitIdle(_vkPresentQueue);

    _currentFrame = (_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

// Wait that the Vulkan device is idle
void RendererManager::waitIdle()
{
    vkDeviceWaitIdle(_vkDevice);
}

void RendererManager::createVertexBuffer()
{
    VkDeviceSize bufferSize = sizeof(_vertices[0]) * _vertices.size(); 

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(_vkDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, _vertices.data(), static_cast<size_t>(bufferSize));
    vkUnmapMemory(_vkDevice, stagingBufferMemory);

    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _vkVertexBuffer, _vkVertexBufferMemory);
    copyBuffer(stagingBuffer, _vkVertexBuffer, bufferSize);

    vkDestroyBuffer(_vkDevice, stagingBuffer, VK_NULL_HANDLE);
    vkFreeMemory(_vkDevice, stagingBufferMemory, VK_NULL_HANDLE);
}

void RendererManager::createIndexBuffer()
{
    VkDeviceSize bufferSize = sizeof(_indices[0]) * _indices.size(); 

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(_vkDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, _indices.data(), static_cast<size_t>(bufferSize));
    vkUnmapMemory(_vkDevice, stagingBufferMemory);

    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _vkIndexBuffer, _vkIndexBufferMemory);
    copyBuffer(stagingBuffer, _vkIndexBuffer, bufferSize);

    vkDestroyBuffer(_vkDevice, stagingBuffer, VK_NULL_HANDLE);
    vkFreeMemory(_vkDevice, stagingBufferMemory, VK_NULL_HANDLE);
}

void RendererManager::copyBuffer(VkBuffer& srcBuffer, VkBuffer& dstBuffer, VkDeviceSize& size)
{
    const VkCommandBuffer commandBuffer = beginSingleTimeCommands();
    const VkBufferCopy copyRegion =
    {
        .srcOffset = 0,
        .dstOffset = 0,
        .size = size,
    };
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
    endSingleTimeCommands(commandBuffer);
}

std::uint32_t RendererManager::findMemoryType(std::uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(_vkPhysicalDevice, &memProperties);

    for (std::uint32_t i = 0; i < memProperties.memoryTypeCount; ++i)
    {
        if (typeFilter & (1 << i)
                && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }

    ERROR_EXIT("Failed to find suitable memory type.");
}

void RendererManager::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
{
    const VkBufferCreateInfo bufferInfo =
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = VK_NULL_HANDLE,
        .flags = 0,
        .size = size,
        .usage = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = VK_NULL_HANDLE,
    };

    if (vkCreateBuffer(_vkDevice, &bufferInfo, VK_NULL_HANDLE, &buffer) != VK_SUCCESS)
    {
        ERROR_EXIT("Failed to create vertex buffer.");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(_vkDevice, buffer, &memRequirements);

    const VkMemoryAllocateInfo allocateInfo =
    {
        .sType= VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = memRequirements.size,
        .memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties),
    };

    if (vkAllocateMemory(_vkDevice, &allocateInfo, VK_NULL_HANDLE, &bufferMemory) != VK_SUCCESS)
    {
        ERROR_EXIT("Failed to allocate vertex buffer memory.");
    }

    vkBindBufferMemory(_vkDevice, buffer, bufferMemory, 0);
}

void RendererManager::createDescriptorSetLayout()
{
    const VkDescriptorSetLayoutBinding uboLayoutBinding =
    {
        .binding = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
        .pImmutableSamplers = VK_NULL_HANDLE,
    };

    const VkDescriptorSetLayoutBinding samplerLayoutBinding =
    {
        .binding = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
        .pImmutableSamplers = VK_NULL_HANDLE,
    };

    const std::array<VkDescriptorSetLayoutBinding, 2> bindings = {uboLayoutBinding, samplerLayoutBinding};

    const VkDescriptorSetLayoutCreateInfo layoutInfo =
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .pNext = VK_NULL_HANDLE,
        .flags = 0,
        .bindingCount = static_cast<std::uint32_t>(bindings.size()),
        .pBindings = bindings.data(),
    };

    if (vkCreateDescriptorSetLayout(_vkDevice, &layoutInfo, VK_NULL_HANDLE, &_vkDescriptorSetLayout) != VK_SUCCESS)
    {
        ERROR_EXIT("Failed to create descriptor set layout.");
    }
}

void RendererManager::createUniformBuffers()
{
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);

    _uniformBuffers.resize(_vkSwapchainImages.size());
    _uniformBuffersMemory.resize(_vkSwapchainImages.size());

    for (size_t i = 0; i < _vkSwapchainImages.size(); ++i)
    {
        createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, _uniformBuffers[i], _uniformBuffersMemory[i]); 
    }
}

void RendererManager::updateUniformBuffer(std::uint32_t currentImage)
{
    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

    const VkExtent2D swapchainExtent = _swapchain.extent();
    const UniformBufferObject ubo =
    {
        .model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
        .view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
        .proj = [&]()
        {
            glm::mat4 proj = glm::perspective(glm::radians(45.0f), swapchainExtent.width / static_cast<float>(swapchainExtent.height), 0.1f, 10.0f);
            proj[1][1] *= -1;
            return proj;
        }(),
    };

    void* data;
    vkMapMemory(_vkDevice, _uniformBuffersMemory[currentImage], 0, sizeof(ubo), 0, &data);
    memcpy(data, &ubo, sizeof(ubo));
    vkUnmapMemory(_vkDevice, _uniformBuffersMemory[currentImage]);
}

void RendererManager::createDescriptorPool()
{
    const std::array<VkDescriptorPoolSize, 2> poolSizes =
    {
        VkDescriptorPoolSize
        {
            .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = static_cast<std::uint32_t>(_vkSwapchainImages.size()),
        },
        {
            .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = static_cast<std::uint32_t>(_vkSwapchainImages.size()),
        },
    };

    const VkDescriptorPoolCreateInfo poolInfo =
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .pNext = VK_NULL_HANDLE,
        .flags = 0,
        .maxSets = static_cast<std::uint32_t>(_vkSwapchainImages.size()),
        .poolSizeCount = static_cast<std::uint32_t>(poolSizes.size()),
        .pPoolSizes = poolSizes.data(),
    };

    if (vkCreateDescriptorPool(_vkDevice, &poolInfo, VK_NULL_HANDLE, &_vkDescriptorPool) != VK_SUCCESS)
    {
        ERROR_EXIT("Failed to create descriptor pool.");
    }
}

void RendererManager::createDescriptorSets()
{
    const std::vector<VkDescriptorSetLayout> layouts(_vkSwapchainImages.size(), _vkDescriptorSetLayout);
    const VkDescriptorSetAllocateInfo allocateInfo =
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext = VK_NULL_HANDLE,
        .descriptorPool = _vkDescriptorPool,
        .descriptorSetCount = static_cast<std::uint32_t>(_vkSwapchainImages.size()),
        .pSetLayouts = layouts.data(),
    };

    _vkDescriptorSets.resize(_vkSwapchainImages.size());
    if (vkAllocateDescriptorSets(_vkDevice, &allocateInfo, _vkDescriptorSets.data()) != VK_SUCCESS)
    {
        ERROR_EXIT("Failed to allocate descriptor sets.");
    }

    for (size_t i = 0; i < _vkSwapchainImages.size(); ++i)
    {
        const VkDescriptorBufferInfo bufferInfo =
        {
            .buffer = _uniformBuffers[i],
            .offset = 0,
            .range = sizeof(UniformBufferObject),
        };

        const VkDescriptorImageInfo imageInfo =
        {
            .sampler = _textureSampler,
            .imageView = _textureImageView,
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        };

        const std::array<VkWriteDescriptorSet, 2> descriptorWrites =
        {
            VkWriteDescriptorSet
            {
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .pNext = VK_NULL_HANDLE,
                .dstSet = _vkDescriptorSets[i],
                .dstBinding = 0,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .pImageInfo = VK_NULL_HANDLE,
                .pBufferInfo = &bufferInfo,
                .pTexelBufferView = VK_NULL_HANDLE,
            },
            {
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .pNext = VK_NULL_HANDLE,
                .dstSet = _vkDescriptorSets[i],
                .dstBinding = 1,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                .pImageInfo = &imageInfo,
                .pBufferInfo = VK_NULL_HANDLE,
                .pTexelBufferView = VK_NULL_HANDLE,
            },
        };

        vkUpdateDescriptorSets(_vkDevice, static_cast<std::uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, VK_NULL_HANDLE);
    }
}

void RendererManager::createTextureImage()
{
    int texWidth, texHeight, texChannels;
    void* pixels = stbi_load("textures/texture.jpg", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    const VkDeviceSize imageSize = texWidth * texHeight * 4;

    if (!pixels)
    {
        ERROR_EXIT("Failed to load texture image.");
    } 

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(_vkDevice, stagingBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, pixels, static_cast<size_t>(imageSize));
    vkUnmapMemory(_vkDevice, stagingBufferMemory);
    
    stbi_image_free(pixels);

    createImage(texWidth, texHeight, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _textureImage, _textureImageMemory);

    transitionImageLayout(_textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    copyBufferToImage(stagingBuffer, _textureImage, static_cast<std::uint32_t>(texWidth), static_cast<std::uint32_t>(texHeight));
    transitionImageLayout(_textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    vkDestroyBuffer(_vkDevice, stagingBuffer, VK_NULL_HANDLE);
    vkFreeMemory(_vkDevice, stagingBufferMemory, VK_NULL_HANDLE);
}

void RendererManager::createImage(const std::uint32_t width, const std::uint32_t height, const VkFormat format, const VkImageTiling tiling, const VkImageUsageFlags usage, const VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory)
{
    const VkImageCreateInfo imageInfo =
    {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .pNext = VK_NULL_HANDLE,
        .flags = 0,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = format,
        .extent = 
        {
            .width = width,
            .height = height,
            .depth = 1,
        },
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = tiling,
        .usage = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = VK_NULL_HANDLE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    };

    if (vkCreateImage(_vkDevice, &imageInfo, VK_NULL_HANDLE, &image) != VK_SUCCESS)
    {
        ERROR_EXIT("Failed to create image from texture.");
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(_vkDevice, image, &memRequirements);

    const VkMemoryAllocateInfo allocateInfo =
    {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .pNext = VK_NULL_HANDLE,
        .allocationSize = memRequirements.size,
        .memoryTypeIndex= findMemoryType(memRequirements.memoryTypeBits, properties),
    };

    if (vkAllocateMemory(_vkDevice, &allocateInfo, VK_NULL_HANDLE, &imageMemory) != VK_SUCCESS)
    {
        ERROR_EXIT("Failed to allocate image memory.");
    }

    vkBindImageMemory(_vkDevice, image, imageMemory, 0);
}

VkCommandBuffer RendererManager::beginSingleTimeCommands()
{
    const VkCommandBufferAllocateInfo allocateInfo =
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = VK_NULL_HANDLE,
        .commandPool = _vkCommandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(_vkDevice, &allocateInfo, &commandBuffer);

    const VkCommandBufferBeginInfo beginInfo =
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = VK_NULL_HANDLE,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        .pInheritanceInfo = VK_NULL_HANDLE,
    };

    vkBeginCommandBuffer(commandBuffer, &beginInfo);
    return commandBuffer;
}

void RendererManager::endSingleTimeCommands(const VkCommandBuffer& commandBuffer)
{
    vkEndCommandBuffer(commandBuffer);

    const VkSubmitInfo submitInfo =
    {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pNext = VK_NULL_HANDLE,
        .waitSemaphoreCount = 0,
        .pWaitSemaphores = VK_NULL_HANDLE,
        .pWaitDstStageMask = VK_NULL_HANDLE,
        .commandBufferCount = 1,
        .pCommandBuffers = &commandBuffer,
        .signalSemaphoreCount = 0,
        .pSignalSemaphores = VK_NULL_HANDLE,
    };

    vkQueueSubmit(_vkGraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(_vkGraphicsQueue);

    vkFreeCommandBuffers(_vkDevice, _vkCommandPool, 1, &commandBuffer);
}

void RendererManager::transitionImageLayout(const VkImage image, const VkFormat format, const VkImageLayout oldLayout, const VkImageLayout newLayout)
{
    const VkCommandBuffer commandBuffer = beginSingleTimeCommands();
    VkPipelineStageFlags srcStage;
    VkPipelineStageFlags dstStage;

    VkImageMemoryBarrier barrier;
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.pNext = VK_NULL_HANDLE;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange = 
    {
        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .baseMipLevel = 0,
        .levelCount = 1,
        .baseArrayLayer = 0,
        .layerCount = 1,
    };
    if (oldLayout== VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else
    {
        ERROR_EXIT("Try to perform unsupported layout transition.");
    }

    vkCmdPipelineBarrier(commandBuffer, srcStage, dstStage, 0, 0, VK_NULL_HANDLE, 0, VK_NULL_HANDLE, 1, &barrier);

    endSingleTimeCommands(commandBuffer);
}

void RendererManager::copyBufferToImage(const VkBuffer buffer, const VkImage image, const std::uint32_t width, const std::uint32_t height)
{
    const VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    const VkBufferImageCopy region =
    {
        .bufferOffset = 0,
        .bufferRowLength = 0,
        .bufferImageHeight = 0,
        .imageSubresource = 
        {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .mipLevel = 0,
            .baseArrayLayer = 0,
            .layerCount = 1,
        },
        .imageOffset = {0, 0, 0},
        .imageExtent = {width, height, 1},
    };

    vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    endSingleTimeCommands(commandBuffer);
}

void RendererManager::createTextureImageView()
{
    _textureImageView = createImageView(_textureImage, VK_FORMAT_R8G8B8A8_SRGB);
}

VkImageView RendererManager::createImageView(const VkImage image, const VkFormat format)
{
    const VkImageViewCreateInfo createInfo =
    {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .pNext = VK_NULL_HANDLE,
        .flags = 0,
        .image = image,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = format,
        .components =
        {
            .r = VK_COMPONENT_SWIZZLE_IDENTITY,
            .g = VK_COMPONENT_SWIZZLE_IDENTITY,
            .b = VK_COMPONENT_SWIZZLE_IDENTITY,
            .a = VK_COMPONENT_SWIZZLE_IDENTITY,
        },
        .subresourceRange =
        {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        },
    };

    VkImageView imageView;
    if (vkCreateImageView(_vkDevice, &createInfo, VK_NULL_HANDLE, &imageView) != VK_SUCCESS)
    {
        ERROR_EXIT("Failed to create texture image view.");
    }
    return imageView;
}

void RendererManager::createTextureSampler()
{
    VkPhysicalDeviceProperties properties {};
    vkGetPhysicalDeviceProperties(_vkPhysicalDevice, &properties);

    const VkSamplerCreateInfo samplerInfo =
    {
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .pNext = VK_NULL_HANDLE,
        .flags = 0,
        .magFilter = VK_FILTER_LINEAR,
        .minFilter = VK_FILTER_LINEAR,
        .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
        .addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
        .addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
        .addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
        .mipLodBias = 0.0f,
        .anisotropyEnable = VK_TRUE,
        .maxAnisotropy = properties.limits.maxSamplerAnisotropy,
        .compareEnable = VK_FALSE,
        .compareOp = VK_COMPARE_OP_ALWAYS,
        .minLod = 0.0f,
        .maxLod = 0.0f,
        .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
        .unnormalizedCoordinates = VK_FALSE,
    };

    if (vkCreateSampler(_vkDevice, &samplerInfo, VK_NULL_HANDLE, &_textureSampler) != VK_SUCCESS)
    {
        ERROR_EXIT("Failed to create texture sampler.");
    }
}
