#include "./DescriptorSet.h"
#include "./../../../utils.h"

#include <array>

DescriptorSet::DescriptorSet(VkDevice& device, const uint32_t maxFramesInFlight)
: _device {device}
{
}

DescriptorSet::~DescriptorSet()
{
}
