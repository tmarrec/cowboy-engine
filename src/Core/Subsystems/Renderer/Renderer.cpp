#include "Renderer.h"

#include "../Window/Window.h"

#include <memory>

extern Window g_Window;

std::unique_ptr<PhysicalDevice>     g_physicalDevice    = nullptr;
std::unique_ptr<LogicalDevice>      g_logicalDevice     = nullptr;
std::unique_ptr<Swapchain>          g_swapchain         = nullptr;
std::unique_ptr<DescriptorPool>     g_descriptorPool    = nullptr;
std::unique_ptr<GraphicsPipeline>   g_graphicsPipeline  = nullptr;
std::unique_ptr<RenderPass>         g_renderPass        = nullptr;

// Initialize the Renderer manager
Renderer::Renderer()
{
    createInstance();
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
    createSwapchain();
    createImages();
    createImageViews();
    createRenderPass();
    updateGraphicsPipeline();
    createCommandPool();
    createDepthResources();
    createFramebuffers();
    createTextureSampler();
    loadTextures();
    loadModels();
    createVertexBuffer();
    createIndexBuffer();
    createDescriptorPool();
    createDescriptorSets();
    allocateCommandBuffers();
    createSyncObjects();
}

// Clean all the objects related to Vulkan
Renderer::~Renderer()
{
    /*
    for (auto& framebuffer : _vkSwapchainFramebuffers)
    {
        vkDestroyFramebuffer(_vkDevice, framebuffer, VK_NULL_HANDLE);
    }
    vkFreeCommandBuffers(_vkDevice, _vkCommandPool, static_cast<uint32_t>(_vkCommandBuffers.size()), _vkCommandBuffers.data());
    for (auto& imageView : _vkSwapchainImageViews)
    {
        vkDestroyImageView(_vkDevice, imageView, VK_NULL_HANDLE);
    }
    vkDestroySwapchainKHR(_vkDevice, _swapchain->vkSwapchain(), VK_NULL_HANDLE);

    for (uint8_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        vkDestroySemaphore(_vkDevice, _vkImageAvailableSemaphores[i], VK_NULL_HANDLE);
        vkDestroySemaphore(_vkDevice, _vkRenderFinishedSemaphores[i], VK_NULL_HANDLE);
        vkDestroyFence(_vkDevice, _vkInFlightFences[i], VK_NULL_HANDLE);
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
    */
	vkDestroySurfaceKHR(_vkInstance, _vkSurface, VK_NULL_HANDLE);
    vkDestroyInstance(_vkInstance, VK_NULL_HANDLE);
}

// Choose the GPU to use
void Renderer::pickPhysicalDevice()
{
    g_physicalDevice = std::make_unique<PhysicalDevice>(_vkInstance, _vkSurface);
}

// Create a Vulkan surface
void Renderer::createSurface()
{
    g_Window.windowCreateSurface(_vkInstance, &_vkSurface);
}


// Create the Vulkan instance
void Renderer::createInstance()
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

// Create the logical Vulkan device
void Renderer::createLogicalDevice()
{
    g_logicalDevice = std::make_unique<LogicalDevice>();
}

// Create the Vulkan swapchain
void Renderer::createSwapchain()
{
    g_swapchain = std::make_unique<Swapchain>(_vkSurface);
}

// Create the images for the swapchain
void Renderer::createImages()
{
    // Link the swapchain to the vector of images
    uint32_t swapchainImageCount;
    const VkSwapchainKHR vkSwapchain = g_swapchain->vkSwapchain();
    vkGetSwapchainImagesKHR(g_logicalDevice->vkDevice(), vkSwapchain, &swapchainImageCount, VK_NULL_HANDLE);
    _vkSwapchainImages.resize(g_swapchain->imageCount());
    vkGetSwapchainImagesKHR(g_logicalDevice->vkDevice(), vkSwapchain, &swapchainImageCount, _vkSwapchainImages.data());
}

// Create the image view for all swapchain images
void Renderer::createImageViews()
{
    _vkSwapchainImageViews.resize(_vkSwapchainImages.size());
    for (size_t i = 0; i < _vkSwapchainImages.size(); ++i)
    {
        _vkSwapchainImageViews[i] = createImageView(_vkSwapchainImages[i], g_swapchain->format(), VK_IMAGE_ASPECT_COLOR_BIT);
    }
}

// Update the Vulkan graphics pipeline
void Renderer::updateGraphicsPipeline()
{
    g_graphicsPipeline = std::make_unique<GraphicsPipeline>();
}

// Create the Vulkan render pass
void Renderer::createRenderPass()
{
    g_renderPass = std::make_unique<RenderPass>();
}

// Create all the framebuffers needed
void Renderer::createFramebuffers()
{
    _vkSwapchainFramebuffers.resize(_vkSwapchainImageViews.size());
    for (size_t i = 0; i < _vkSwapchainImageViews.size(); ++i)
    {
        const std::array<VkImageView, 2> attachments =
        {
            _vkSwapchainImageViews[i],
            _depthImageView
        };

        const VkExtent2D swapchainExtent = g_swapchain->extent();
        const VkFramebufferCreateInfo framebufferInfo =
        {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .pNext = VK_NULL_HANDLE,
            .flags = 0,
            .renderPass = g_renderPass->vkRenderPass(),
            .attachmentCount = static_cast<uint32_t>(attachments.size()),
            .pAttachments = attachments.data(),
            .width = swapchainExtent.width,
            .height = swapchainExtent.height,
            .layers = 1,
        };

        if (vkCreateFramebuffer(g_logicalDevice->vkDevice(), &framebufferInfo, VK_NULL_HANDLE, &_vkSwapchainFramebuffers[i]) != VK_SUCCESS)
        {
            ERROR_EXIT("Failed to create framebuffer.");
        }
    }
}

// Create the Vulkan command pool
void Renderer::createCommandPool()
{
    const QueueFamilyIndices queueFamilyIndices = g_physicalDevice->findQueueFamilies(g_physicalDevice->vkPhysicalDevice());

    const VkCommandPoolCreateInfo poolInfo =
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = VK_NULL_HANDLE,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = queueFamilyIndices.graphics.value(),
    };

    if (vkCreateCommandPool(g_logicalDevice->vkDevice(), &poolInfo, VK_NULL_HANDLE, &_vkCommandPool) != VK_SUCCESS)
    {
        ERROR_EXIT("Failed to create command pool.");
    }
}

// Create all the Vulkan command buffers and initialize them
void Renderer::allocateCommandBuffers()
{
    _vkCommandBuffers.resize(_vkSwapchainFramebuffers.size());

    const VkCommandBufferAllocateInfo commandBufferAllocateInfo =
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = VK_NULL_HANDLE,
        .commandPool = _vkCommandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = static_cast<uint32_t>(_vkCommandBuffers.size()),
    };

    if (vkAllocateCommandBuffers(g_logicalDevice->vkDevice(), &commandBufferAllocateInfo, _vkCommandBuffers.data()) != VK_SUCCESS)
    {
        ERROR_EXIT("Failed to create command buffers.");
    }
}

// Create all the synchronisation objects for the drawFrame
void Renderer::createSyncObjects()
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

    for (uint8_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        if (
                vkCreateSemaphore(g_logicalDevice->vkDevice(), &semaphoreInfo, VK_NULL_HANDLE, &_vkImageAvailableSemaphores[i]) != VK_SUCCESS
            ||  vkCreateSemaphore(g_logicalDevice->vkDevice(), &semaphoreInfo, VK_NULL_HANDLE, &_vkRenderFinishedSemaphores[i]) != VK_SUCCESS
            ||  vkCreateFence(g_logicalDevice->vkDevice(), &fenceInfo, VK_NULL_HANDLE, &_vkInFlightFences[i]) != VK_SUCCESS
        )
        {
            ERROR_EXIT("Failed to create synchronisation objects.");
        }
    }
}

void Renderer::createCommandBuffer(const uint32_t commandBufferIndex)
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

    const std::array<VkClearValue, 2> clearValues =
    {
        VkClearValue
        {
            .color = 
            VkClearColorValue
            {
                .float32 = {0.0f, 0.0f, 0.0f, 1.0f},
            }
        },
        {
            .depthStencil = 
            VkClearDepthStencilValue
            {
                .depth = 1.0f,
                .stencil = 0,
            }
        }
    };
    const VkRenderPassBeginInfo renderPassInfo =
    {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .pNext = VK_NULL_HANDLE,
        .renderPass = g_renderPass->vkRenderPass(),
        .framebuffer = _vkSwapchainFramebuffers[commandBufferIndex],
        .renderArea = 
        {
            .offset = {0, 0},
            .extent = g_swapchain->extent(),
        },
        .clearValueCount = static_cast<uint32_t>(clearValues.size()),
        .pClearValues = clearValues.data(),
    };

    vkCmdBeginRenderPass(_vkCommandBuffers[commandBufferIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    // Viewport and Scissor
    uint32_t width, height;
    g_Window.windowGetFramebufferSize(width, height);
    const VkViewport viewport =
    {
        .x = 0.0f,
        .y = 0.0f,
        .width = static_cast<float>(width),
        .height = static_cast<float>(height),
        .minDepth = 0.0f,
        .maxDepth = 1.0f,
    };
    vkCmdSetViewport(_vkCommandBuffers[commandBufferIndex], 0, 1, &viewport);

    const VkRect2D scissor =
    {
        .offset = {0, 0},
        .extent = g_swapchain->extent(),
    };
    vkCmdSetScissor(_vkCommandBuffers[commandBufferIndex], 0, 1, &scissor);

    g_graphicsPipeline->bind(_vkCommandBuffers[commandBufferIndex]);

    const std::array<VkBuffer, 1> vertexBuffers = {_vkVertexBuffer};
    const std::array<VkDeviceSize, 1> offsets = {0};

    vkCmdBindVertexBuffers(_vkCommandBuffers[commandBufferIndex], 0, 1, vertexBuffers.data(), offsets.data());
    vkCmdBindIndexBuffer(_vkCommandBuffers[commandBufferIndex], _vkIndexBuffer, 0, VK_INDEX_TYPE_UINT16);


    for (const auto& n : _world.getNodes())
    {
        const glm::mat4& transform = n.getTransform();
        if (n.gotMesh())
        {
            for (const auto& p : n.getPrimitives())
            {
                // Texture
                vkCmdBindDescriptorSets(_vkCommandBuffers[commandBufferIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, g_graphicsPipeline->vkPipelineLayout(), 0, 1, &_vkDescriptorSets[commandBufferIndex], 0, VK_NULL_HANDLE);

                // Transform matrice
                const glm::mat4 t = _projView * transform;
                vkCmdPushConstants(_vkCommandBuffers[commandBufferIndex], g_graphicsPipeline->vkPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &t); 

                // Draw
                vkCmdDrawIndexed(_vkCommandBuffers[commandBufferIndex], p.indexCount, 1, p.firstIndex, p.vertexOffset / 3, 0);
            }
        }
    }

    vkCmdEndRenderPass(_vkCommandBuffers[commandBufferIndex]);

    if (vkEndCommandBuffer(_vkCommandBuffers[commandBufferIndex]) != VK_SUCCESS)
    {
        ERROR_EXIT("Failed to record command buffer.");
    }
}

// Draw the frame by executing the queues while staying synchronised
void Renderer::drawFrame()
{
    VkDevice vkDevice = g_logicalDevice->vkDevice();
    vkWaitForFences(vkDevice, 1, &_vkInFlightFences[_currentFrame], VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    VkResult acquireNextImageResult = vkAcquireNextImageKHR(vkDevice, g_swapchain->vkSwapchain(), UINT64_MAX, _vkImageAvailableSemaphores[_currentFrame], VK_NULL_HANDLE, &imageIndex);
    if (acquireNextImageResult == VK_ERROR_OUT_OF_DATE_KHR)
    {
        // Out of date image, the window has probably been resized
        // Need to update the swapchain, imageviews and framebuffers
        INFO("Detected out-of-date image type, will rebuild the swapchain.");
        vkDeviceWaitIdle(vkDevice);

        vkDestroyImageView(vkDevice, _depthImageView, VK_NULL_HANDLE);
        vkDestroyImage(vkDevice, _depthImage, VK_NULL_HANDLE);
        vkFreeMemory(vkDevice, _depthImageMemory, VK_NULL_HANDLE);

        for (auto& framebuffer : _vkSwapchainFramebuffers)
        {
            vkDestroyFramebuffer(vkDevice, framebuffer, VK_NULL_HANDLE);
        }
        for (auto& imageView : _vkSwapchainImageViews)
        {
            vkDestroyImageView(vkDevice, imageView, VK_NULL_HANDLE);
        }

        createSwapchain();
        createImages();
        createImageViews();
        createDepthResources();
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
        vkWaitForFences(vkDevice, 1, &_vkImagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
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

    vkResetFences(vkDevice, 1, &_vkInFlightFences[_currentFrame]);

    if (vkQueueSubmit(g_logicalDevice->vkGraphicsQueue(), 1, &submitInfo, _vkInFlightFences[_currentFrame]) != VK_SUCCESS)
    {
        ERROR_EXIT("Failed to submit draw command buffer.");
    }

    const std::array<VkSwapchainKHR, 1> swapchains = {g_swapchain->vkSwapchain()};
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

    vkQueuePresentKHR(g_logicalDevice->vkPresentQueue(), &presentInfo);

    vkQueueWaitIdle(g_logicalDevice->vkPresentQueue());

    _currentFrame = (_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

// Wait that the Vulkan device is idle
void Renderer::waitIdle()
{
    vkDeviceWaitIdle(g_logicalDevice->vkDevice());
}

void Renderer::createVertexBuffer()
{
    VkDevice vkDevice = g_logicalDevice->vkDevice();
    VkDeviceSize bufferSize = sizeof(_vertices[0]) * _vertices.size(); 

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(vkDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, _vertices.data(), static_cast<size_t>(bufferSize));
    vkUnmapMemory(vkDevice, stagingBufferMemory);

    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _vkVertexBuffer, _vkVertexBufferMemory);
    copyBuffer(stagingBuffer, _vkVertexBuffer, bufferSize);

    vkDestroyBuffer(vkDevice, stagingBuffer, VK_NULL_HANDLE);
    vkFreeMemory(vkDevice, stagingBufferMemory, VK_NULL_HANDLE);
}

void Renderer::createIndexBuffer()
{
    VkDevice vkDevice = g_logicalDevice->vkDevice();
    VkDeviceSize bufferSize = sizeof(_indices[0]) * _indices.size(); 

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(vkDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, _indices.data(), static_cast<size_t>(bufferSize));
    vkUnmapMemory(vkDevice, stagingBufferMemory);

    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _vkIndexBuffer, _vkIndexBufferMemory);
    copyBuffer(stagingBuffer, _vkIndexBuffer, bufferSize);

    vkDestroyBuffer(vkDevice, stagingBuffer, VK_NULL_HANDLE);
    vkFreeMemory(vkDevice, stagingBufferMemory, VK_NULL_HANDLE);
}

void Renderer::copyBuffer(VkBuffer& srcBuffer, VkBuffer& dstBuffer, VkDeviceSize& size)
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

uint32_t Renderer::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(g_physicalDevice->vkPhysicalDevice(), &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; ++i)
    {
        if (typeFilter & (1 << i)
                && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }

    ERROR_EXIT("Failed to find suitable memory type.");
}

void Renderer::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
{
    VkDevice vkDevice = g_logicalDevice->vkDevice();
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

    if (vkCreateBuffer(vkDevice, &bufferInfo, VK_NULL_HANDLE, &buffer) != VK_SUCCESS)
    {
        ERROR_EXIT("Failed to create buffer.");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(vkDevice, buffer, &memRequirements);

    const VkMemoryAllocateInfo allocateInfo =
    {
        .sType= VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = memRequirements.size,
        .memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties),
    };

    if (vkAllocateMemory(vkDevice, &allocateInfo, VK_NULL_HANDLE, &bufferMemory) != VK_SUCCESS)
    {
        ERROR_EXIT("Failed to allocate buffer memory.");
    }

    vkBindBufferMemory(vkDevice, buffer, bufferMemory, 0);
}

void Renderer::updateUniformBuffer(const uint32_t currentImage)
{
    const UniformBufferObject ubo =
    {
        .view = glm::lookAt(_cameraParameters.position, _cameraParameters.position+_cameraParameters.front, _cameraParameters.up),
        .proj = [&]()
        {
            const VkExtent2D swapchainExtent = g_swapchain->extent();
            glm::mat4 proj = glm::perspective(glm::radians(_cameraParameters.FOV), swapchainExtent.width / static_cast<float>(swapchainExtent.height), 0.01f, 128.0f);
            proj[1][1] *= -1;
            return proj;
        }(),
    };

    _projView = ubo.proj * ubo.view;
}

void Renderer::createDescriptorPool()
{
    g_descriptorPool = std::make_unique<DescriptorPool>(MAX_FRAMES_IN_FLIGHT);
}

void Renderer::createDescriptorSets()
{
    INFO("TODO CREATEDESCRIPTORSETS");
    /*
    const std::array<uint32_t, 1> variableDescCounts = { 256 };
    const VkDescriptorSetVariableDescriptorCountAllocateInfo setCounts = 
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO,
        .descriptorSetCount = 1,
        .pDescriptorCounts = variableDescCounts.data(),
    };

    const std::vector<VkDescriptorSetLayout> layouts(_vkSwapchainImages.size(), _vkDescriptorSetLayout);

    const VkDescriptorSetAllocateInfo allocateInfo =
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext = &setCounts,
        .descriptorPool = _vkDescriptorPool,
        .descriptorSetCount = static_cast<uint32_t>(_vkSwapchainImages.size()),
        .pSetLayouts = layouts.data(),
    };

    _vkDescriptorSets.resize(_vkSwapchainImages.size());
    if (vkAllocateDescriptorSets(_vkDevice, &allocateInfo, _vkDescriptorSets.data()) != VK_SUCCESS)
    {
        ERROR_EXIT("Failed to allocate descriptor sets.");
    }

    INFO("Descriptor sets successfully created.");
    */
}

void Renderer::createImage(const uint32_t width, const uint32_t height, const VkFormat format, const VkImageTiling tiling, const VkImageUsageFlags usage, const VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory)
{
    VkDevice vkDevice = g_logicalDevice->vkDevice();
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

    if (vkCreateImage(vkDevice, &imageInfo, VK_NULL_HANDLE, &image) != VK_SUCCESS)
    {
        ERROR_EXIT("Failed to create image from texture.");
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(vkDevice, image, &memRequirements);

    const VkMemoryAllocateInfo allocateInfo =
    {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .pNext = VK_NULL_HANDLE,
        .allocationSize = memRequirements.size,
        .memoryTypeIndex= findMemoryType(memRequirements.memoryTypeBits, properties),
    };

    if (vkAllocateMemory(vkDevice, &allocateInfo, VK_NULL_HANDLE, &imageMemory) != VK_SUCCESS)
    {
        ERROR_EXIT("Failed to allocate image memory.");
    }

    vkBindImageMemory(vkDevice, image, imageMemory, 0);
}

VkCommandBuffer Renderer::beginSingleTimeCommands()
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
    vkAllocateCommandBuffers(g_logicalDevice->vkDevice(), &allocateInfo, &commandBuffer);

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

void Renderer::endSingleTimeCommands(const VkCommandBuffer& commandBuffer)
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

    vkQueueSubmit(g_logicalDevice->vkGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(g_logicalDevice->vkGraphicsQueue());

    vkFreeCommandBuffers(g_logicalDevice->vkDevice(), _vkCommandPool, 1, &commandBuffer);
}

void Renderer::transitionImageLayout(const VkImage image, const VkFormat format, const VkImageLayout oldLayout, const VkImageLayout newLayout)
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

void Renderer::copyBufferToImage(const VkBuffer buffer, const VkImage image, const uint32_t width, const uint32_t height)
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

VkImageView Renderer::createImageView(const VkImage image, const VkFormat format, const VkImageAspectFlags aspectFlags)
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
            .aspectMask = aspectFlags,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        },
    };

    VkImageView imageView;
    if (vkCreateImageView(g_logicalDevice->vkDevice(), &createInfo, VK_NULL_HANDLE, &imageView) != VK_SUCCESS)
    {
        ERROR_EXIT("Failed to create texture image view.");
    }
    return imageView;
}

void Renderer::createTextureSampler()
{
    VkPhysicalDeviceProperties properties {};
    vkGetPhysicalDeviceProperties(g_physicalDevice->vkPhysicalDevice(), &properties);

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

    if (vkCreateSampler(g_logicalDevice->vkDevice(), &samplerInfo, VK_NULL_HANDLE, &_textureSampler) != VK_SUCCESS)
    {
        ERROR_EXIT("Failed to create texture sampler.");
    }
}

void Renderer::createDepthResources()
{
    const VkFormat depthFormat = VK_FORMAT_D32_SFLOAT;
    const auto extent = g_swapchain->extent();
    createImage(extent.width, extent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _depthImage, _depthImageMemory);
    _depthImageView = createImageView(_depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
}

void Renderer::loadModels()
{
    const auto& badVertices = _world.getVertexBuffer();

    for (size_t i = 0; i < badVertices.size(); i += 3)
    {
        Vertex v = 
        {
            .pos = {badVertices[i], badVertices[i+1], badVertices[i+2]},
            .color = {1.0f, 0.0f, 0.0f},
            .texCoord = {0.0f, 0.0f},
        };
        _vertices.emplace_back(v);
    }

    const auto& badIndices = _world.getIndicesBuffer();
    for (size_t i = 0; i < badIndices.size(); ++i)
    {
        _indices.emplace_back(badIndices[i]);
    }
}

void Renderer::setCameraParameters(const glm::vec3& position, const float FOV, const glm::vec3& front, const glm::vec3& up)
{
    _cameraParameters.position = position;
    _cameraParameters.FOV = FOV;
    _cameraParameters.front = front;
    _cameraParameters.up = up;
}

void Renderer::createTexture(const Image& image)
{
    // TEMP
    VkImage _textureImage;
    VkDeviceMemory _textureImageMemory;
    //VkImageView _textureImageView;

    // Creating texture image
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    const size_t imageSize = image.data.size();

    createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    const VkDevice vkDevice = g_logicalDevice->vkDevice();

    void* data;
    vkMapMemory(vkDevice, stagingBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, image.data.data(), static_cast<size_t>(imageSize));
    vkUnmapMemory(vkDevice, stagingBufferMemory);
    
    createImage(image.width, image.height, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _textureImage, _textureImageMemory);

    transitionImageLayout(_textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    copyBufferToImage(stagingBuffer, _textureImage, static_cast<uint32_t>(image.width), static_cast<uint32_t>(image.height));
    transitionImageLayout(_textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    vkDestroyBuffer(vkDevice, stagingBuffer, VK_NULL_HANDLE);
    vkFreeMemory(vkDevice, stagingBufferMemory, VK_NULL_HANDLE);

    // Creating texture image view
    //_textureImageView = createImageView(_textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
}

void Renderer::loadTextures()
{
    const auto& textures = _world.getTextures();
    for (const auto& texture : textures)
    {
        createTexture(texture.getImage());
    }
}
