#include "./GraphicsPipeline.h"
#include "./../../utils.h"

#include <array>
#include <glm/glm.hpp>

#include "Swapchain.h"

extern std::unique_ptr<LogicalDevice>   g_logicalDevice;
extern std::unique_ptr<Swapchain>       g_swapchain;

GraphicsPipeline::GraphicsPipeline()
{
    createDescriptorSetLayout();

    _vertShader = std::make_shared<Shader>("vert.vert", SHADER_TYPE_VERTEX);
    _fragShader = std::make_shared<Shader>("frag.frag", SHADER_TYPE_FRAGMENT);

    const VkPipelineShaderStageCreateInfo vertShaderStageInfo =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .pNext = VK_NULL_HANDLE,
        .flags = 0,
        .stage = VK_SHADER_STAGE_VERTEX_BIT,
        .module = _vertShader->shaderModule(),
        .pName = "main",
        .pSpecializationInfo = VK_NULL_HANDLE,
    };

    const VkPipelineShaderStageCreateInfo fragShaderStageInfo =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .pNext = VK_NULL_HANDLE,
        .flags = 0,
        .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
        .module = _fragShader->shaderModule(),
        .pName = "main",
        .pSpecializationInfo = VK_NULL_HANDLE,
    };

    const std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages = {vertShaderStageInfo, fragShaderStageInfo};

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
        .vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescription.size()),
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
    const VkExtent2D vkExtent = g_swapchain->extent();
    const VkViewport viewport =
    {
        .x = 0.0f,
        .y = 0.0f,
        .width = static_cast<float>(vkExtent.width),
        .height = static_cast<float>(vkExtent.height),
        .minDepth = 0.0f,
        .maxDepth = 1.0f,
    };

    const VkRect2D scissor =
    {
        .offset = {0, 0},
        .extent = vkExtent,
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

    // Depth buffer
    const VkPipelineDepthStencilStateCreateInfo depthStencil =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .pNext = VK_NULL_HANDLE,
        .flags = 0,
        .depthTestEnable = VK_TRUE,
        .depthWriteEnable = VK_TRUE,
        .depthCompareOp = VK_COMPARE_OP_LESS,
        .depthBoundsTestEnable = VK_FALSE,
        .stencilTestEnable = VK_FALSE,
        .front = {},
        .back = {},
        .minDepthBounds = 0.0f,
        .maxDepthBounds = 1.0f,
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

    // Push Constant
    const VkPushConstantRange pushConstantRange =
    {
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
        .offset = 0,
        .size = sizeof(glm::mat4),
    };

    // Pipeline layout
    const VkPipelineLayoutCreateInfo pipelineLayoutInfo =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pNext = VK_NULL_HANDLE,
        .flags = 0,
        .setLayoutCount = 2,
        .pSetLayouts = &_vkDescriptorSetLayout[0],
        .pushConstantRangeCount = 1,
        .pPushConstantRanges = &pushConstantRange,
    };

    if (vkCreatePipelineLayout(g_logicalDevice->vkDevice(), &pipelineLayoutInfo, VK_NULL_HANDLE, &_vkPipelineLayout) != VK_SUCCESS)
    {
        ERROR("Failed to create pipeline layout.");
    }

    const VkGraphicsPipelineCreateInfo pipelineInfo =
    {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext = VK_NULL_HANDLE,
        .flags = 0,
        .stageCount = 2,
        .pStages = shaderStages.data(),
        .pVertexInputState = &vertexInputInfo,
        .pInputAssemblyState = &inputAssemblyInfo,
        .pTessellationState = VK_NULL_HANDLE,
        .pViewportState = &viewportStateInfo,
        .pRasterizationState = &rasterizerInfo,
        .pMultisampleState = &multisamplingInfo,
        .pDepthStencilState = &depthStencil,
        .pColorBlendState = &colorBlendingInfo,
        .pDynamicState = &dynamicStateInfo,
        .layout = _vkPipelineLayout,
        .renderPass = _renderPass.vkRenderPass(),
        .subpass = 0,
        .basePipelineHandle = VK_NULL_HANDLE,
        .basePipelineIndex = -1,
    };

    CHECK("Graphics pipeline", vkCreateGraphicsPipelines(g_logicalDevice->vkDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, VK_NULL_HANDLE, &_vkGraphicsPipeline));

    _vertShader->destroyShaderModule();
    _fragShader->destroyShaderModule();
}

GraphicsPipeline::~GraphicsPipeline()
{
    const VkDevice vkDevice = g_logicalDevice->vkDevice();
    vkDestroyPipelineLayout(vkDevice, _vkPipelineLayout, VK_NULL_HANDLE);
    vkDestroyPipeline(vkDevice, _vkGraphicsPipeline, VK_NULL_HANDLE);

    vkDestroyDescriptorSetLayout(vkDevice, _vkDescriptorSetLayout[0], VK_NULL_HANDLE);
    vkDestroyDescriptorSetLayout(vkDevice, _vkDescriptorSetLayout[1], VK_NULL_HANDLE);
}

void GraphicsPipeline::bind(const VkCommandBuffer& vkCommandBuffer) const
{
    vkCmdBindPipeline(vkCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _vkGraphicsPipeline);
}

void GraphicsPipeline::createDescriptorSetLayout()
{
    _vkDescriptorSetLayout.resize(MAX_FRAMES_IN_FLIGHT);
    for (uint8_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        const std::array<VkDescriptorSetLayoutBinding, 2> bindlessLayout =
        {
            VkDescriptorSetLayoutBinding
            {
                .binding = 0,
                .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .descriptorCount = 1,
                .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
                .pImmutableSamplers = VK_NULL_HANDLE,
            },
            {
                .binding = 1,
                .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                .descriptorCount = 32 * 1024,
                .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
                .pImmutableSamplers = VK_NULL_HANDLE,
            }
        };

        const std::array<VkDescriptorBindingFlags, 2> descriptorBindingFlags =
        {
            0,
            VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT,
        };
    
        const VkDescriptorSetLayoutBindingFlagsCreateInfo setLayoutBindingFlags =
        {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
            .pNext = VK_NULL_HANDLE,
            .bindingCount = 2,
            .pBindingFlags = &descriptorBindingFlags[0],
        };

        const VkDescriptorSetLayoutCreateInfo layoutInfo =
        {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .pNext = &setLayoutBindingFlags,
            .flags = 0,
            .bindingCount = 2,
            .pBindings = &bindlessLayout[0],
        };

        CHECK("Descriptor set layout", vkCreateDescriptorSetLayout(g_logicalDevice->vkDevice(), &layoutInfo, VK_NULL_HANDLE, &_vkDescriptorSetLayout[i]))
    }
}

const VkPipelineLayout& GraphicsPipeline::vkPipelineLayout() const
{
    return _vkPipelineLayout;
}

const std::vector<VkDescriptorSetLayout>& GraphicsPipeline::vkDescriptorSetLayouts() const
{
    return _vkDescriptorSetLayout;
}

const RenderPass& GraphicsPipeline::renderPass() const
{
    return _renderPass;
}

