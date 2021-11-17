#pragma once

#include <memory>

#include "PhysicalDevice.h"
#include "LogicalDevice.h"
#include "DescriptorPool.h"
#include "GraphicsPipeline.h"
#include "RenderPass.h"
#include "Swapchain.h"

inline std::shared_ptr<PhysicalDevice>     g_physicalDevice     = nullptr;
inline std::shared_ptr<LogicalDevice>      g_logicalDevice      = nullptr;
inline std::shared_ptr<DescriptorPool>     g_descriptorPool     = nullptr;
inline std::shared_ptr<GraphicsPipeline>   g_graphicsPipeline   = nullptr;
inline std::shared_ptr<RenderPass>         g_renderPass         = nullptr;
inline std::shared_ptr<Swapchain>          g_swapchain          = nullptr;
