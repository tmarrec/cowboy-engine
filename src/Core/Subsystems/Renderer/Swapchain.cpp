#include "./Swapchain.h"

#include <vulkan/vulkan_core.h>
#include "PhysicalDevice.h"
#include "LogicalDevice.h"
#include "Instance.h"
#include "GraphicsPipeline.h"

extern Window                               g_Window;
extern std::unique_ptr<PhysicalDevice>      g_physicalDevice;
extern std::unique_ptr<LogicalDevice>       g_logicalDevice;
extern std::unique_ptr<Instance>            g_instance;
extern std::unique_ptr<GraphicsPipeline>    g_graphicsPipeline;

// Create the Vulkan swapchain
Swapchain::Swapchain()
{
    // Get the informations needed for the swapchain
    const SwapchainSupportDetails swapchainSupport = querySupport(g_physicalDevice->vkPhysicalDevice());
    const VkSurfaceFormatKHR surfaceFormat = chooseSurfaceFormat(swapchainSupport.formats);
    const VkPresentModeKHR presentMode = choosePresentMode(swapchainSupport.presentModes);
    _imageCount = [&]()
    {
        uint32_t imageCount = swapchainSupport.capabilities.minImageCount + 1;
        if (swapchainSupport.capabilities.maxImageCount > 0 && imageCount > swapchainSupport.capabilities.maxImageCount)
        {
            imageCount = swapchainSupport.capabilities.maxImageCount;
        }
        return imageCount;
    }();
    _vkSurfaceFormat = surfaceFormat.format;
    _vkExtent = chooseExtent(swapchainSupport.capabilities);

    // Fill the swapchain informations
    const VkSwapchainCreateInfoKHR createInfo = [&]()
    {
        VkSwapchainCreateInfoKHR createInfo =
        {
            .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
            .pNext = VK_NULL_HANDLE,
            .flags = 0,
            .surface = g_instance->vkSurface(),
            .minImageCount = _imageCount,
            .imageFormat = _vkSurfaceFormat,
            .imageColorSpace = surfaceFormat.colorSpace,
            .imageExtent = _vkExtent,
            .imageArrayLayers = 1,
            .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            .preTransform = swapchainSupport.capabilities.currentTransform,
            .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
            .presentMode = presentMode,
            .clipped = VK_TRUE,
            .oldSwapchain = _vkSwapchain,
        };
        auto indices = g_physicalDevice->findQueueFamilies(g_physicalDevice->vkPhysicalDevice());
        if (indices.graphics != indices.present)
        {
            const std::array<uint32_t, 2> queueFamilyIndices = {indices.graphics.value(), indices.present.value()};
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
        return createInfo;
    }();


    // Create the swapchain
    CHECK("Swapchain", vkCreateSwapchainKHR(g_logicalDevice->vkDevice(), &createInfo, VK_NULL_HANDLE, &_vkSwapchain));

    createImages();
    createImagesViews();
    createDepthResources();
}

// Get the swapchain surface format, aiming for RGBA 32bits sRGB 
VkSurfaceFormatKHR Swapchain::chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
    for (const auto& availableFormat : availableFormats)
    {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB
            && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return availableFormat;
        }
    }
    WARNING("Unable to find desired swapchain surface formats. Will use the first found");
    return availableFormats[0];
}

// Get the swapchain present mode, aiming for MAILBOX mode
VkPresentModeKHR Swapchain::choosePresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
    for (const auto& availablePresentMode : availablePresentModes)
    {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            return availablePresentMode;
        }
    }
    WARNING("Unable to find VK_PRESENT_MODE_MAILBOX_KHR swapchain present mode. Will use VK_PRESENT_MODE_IMMEDIATE_KHR.");
    return VK_PRESENT_MODE_IMMEDIATE_KHR;
}

// Choose the swap extent based on the window manager informations
VkExtent2D Swapchain::chooseExtent(const VkSurfaceCapabilitiesKHR capabilities)
{
    if (capabilities.currentExtent.width != UINT32_MAX)
    {
        return capabilities.currentExtent;
    }
    else
    {
        return [&]()
        {
            uint32_t width, height;
            g_Window.windowGetFramebufferSize(width, height);
            width = std::clamp(width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            height = std::clamp(height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
            return VkExtent2D{width, height};
        }();
    }
}

// Get the details of the device swapchain support
const SwapchainSupportDetails Swapchain::querySupport(const VkPhysicalDevice physicalDevice) const
{
    const auto surface = g_instance->vkSurface();
    SwapchainSupportDetails details;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, VK_NULL_HANDLE);

    if (formatCount != 0)
    {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, VK_NULL_HANDLE);
    if (presentModeCount != 0)
    {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, details.presentModes.data());
    }
    return details;
}

const uint32_t Swapchain::imageCount() const
{
    return _imageCount;
}

const VkFormat Swapchain::format() const
{
    return _vkSurfaceFormat;
}

const VkExtent2D Swapchain::extent() const
{
    return _vkExtent;
}

// Getter to the vulkan swapchain object
VkSwapchainKHR& Swapchain::vkSwapchain()
{
    return _vkSwapchain;
}

const std::vector<VkImage> Swapchain::vkSwapchainImages() const
{
    return _vkSwapchainImages;
}

void Swapchain::createImages()
{
    // Link the swapchain to the vector of images
    uint32_t swapchainImageCount;
    vkGetSwapchainImagesKHR(g_logicalDevice->vkDevice(), _vkSwapchain, &swapchainImageCount, VK_NULL_HANDLE);
    _vkSwapchainImages.resize(_imageCount);
    vkGetSwapchainImagesKHR(g_logicalDevice->vkDevice(), _vkSwapchain, &swapchainImageCount, _vkSwapchainImages.data());
    OK("Swapchain images");
}

void Swapchain::createImagesViews()
{
    _vkSwapchainImageViews.resize(_vkSwapchainImages.size());
    for (size_t i = 0; i < _vkSwapchainImages.size(); ++i)
    {
        _vkSwapchainImageViews[i] = createImageView(_vkSwapchainImages[i], _vkSurfaceFormat, VK_IMAGE_ASPECT_COLOR_BIT);
    }
    OK("Swapchain images views");
}

VkImageView Swapchain::createImageView(const VkImage image, const VkFormat format, const VkImageAspectFlags aspectFlags)
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
    CHECK("Swapchain image view", vkCreateImageView(g_logicalDevice->vkDevice(), &createInfo, VK_NULL_HANDLE, &imageView));
    return imageView;
}

void Swapchain::createImage(const uint32_t width, const uint32_t height, const VkFormat format, const VkImageTiling tiling, const VkImageUsageFlags usage, const VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory)
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

void Swapchain::createDepthResources()
{
    const VkFormat depthFormat = VK_FORMAT_D32_SFLOAT;
    createImage(_vkExtent.width, _vkExtent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _depthImage, _depthImageMemory);
    _depthImageView = createImageView(_depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
}

void Swapchain::createFramebuffers()
{
    ASSERT(g_graphicsPipeline, "g_graphicsPipeline should not be nullptr");
    for (size_t i = 0; i < _vkSwapchainImageViews.size(); ++i)
    {
        const std::vector<VkImageView> attachments =
        {
            _vkSwapchainImageViews[i],
            _depthImageView
        };

        auto framebuffer = std::make_unique<Framebuffer>(g_graphicsPipeline->renderPass().vkRenderPass(), attachments, _vkExtent.width, _vkExtent.height, 1);
        _swapchainFramebuffers.emplace_back(std::move(framebuffer));
    }
    OK("Swapchain framebuffers");
}

uint32_t Swapchain::findMemoryType(const uint32_t typeFilter, const VkMemoryPropertyFlags properties)
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
