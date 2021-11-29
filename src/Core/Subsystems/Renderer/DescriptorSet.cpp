#include "./DescriptorSet.h"
#include "./../../utils.h"

#include <array>
#include <memory>

#include "GraphicsPipeline.h"
#include "DescriptorPool.h"

extern std::unique_ptr<GraphicsPipeline>    g_graphicsPipeline;
extern std::unique_ptr<LogicalDevice>       g_logicalDevice;
extern std::unique_ptr<DescriptorPool>      g_descriptorPool;

DescriptorSet::DescriptorSet()
{
    const std::array<uint32_t, 1> variableDescCounts = { 32 * 1024 };
    const VkDescriptorSetVariableDescriptorCountAllocateInfo variableDescCountAllocInfo = 
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO,
        .descriptorSetCount = 1,
        .pDescriptorCounts = variableDescCounts.data(),
    };

    const VkDescriptorSetAllocateInfo allocateInfo =
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext = &variableDescCountAllocInfo,
        .descriptorPool = g_descriptorPool->vkDescriptorPool(),
        .descriptorSetCount = 1,
        .pSetLayouts = g_graphicsPipeline->vkDescriptorSetLayouts().data()
    };

    _vkDescriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
    CHECK("Descriptor sets", vkAllocateDescriptorSets(g_logicalDevice->vkDevice(), &allocateInfo, _vkDescriptorSets.data()));
}

DescriptorSet::~DescriptorSet()
{
}
