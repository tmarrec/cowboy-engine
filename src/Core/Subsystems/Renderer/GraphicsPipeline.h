#pragma once

#include <vulkan/vulkan.h>
#include "glm/glm.hpp"

#include "Shader.h"

struct Vertex
{
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec2 texCoord;

    static VkVertexInputBindingDescription getBindingDescription()
    {
        return 
        {
            .binding = 0,
            .stride = sizeof(Vertex),
            .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
        };
    }

    static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions()
    {
        return
        {
            VkVertexInputAttributeDescription
            {   // Position
                .location = 0,
                .binding = 0,
                .format = VK_FORMAT_R32G32B32_SFLOAT,
                .offset = offsetof(Vertex, pos),
            },
            {   // Color
                .location = 1,
                .binding = 0,
                .format = VK_FORMAT_R32G32B32_SFLOAT,
                .offset = offsetof(Vertex, color),
            },
            {   // Texture Coordinates
                .location = 2,
                .binding = 0,
                .format = VK_FORMAT_R32G32_SFLOAT,
                .offset = offsetof(Vertex, texCoord),
            }
        };
    }
};

class GraphicsPipeline
{
 public:
    GraphicsPipeline(const VkDevice& vkDevice, const VkExtent2D& vkExtent);
    ~GraphicsPipeline();
    void bind(const VkCommandBuffer& vkCommandBuffer) const;

 private:
    const VkDevice&         _vkDevice;
    VkPipeline              _vkGraphicsPipeline     = VK_NULL_HANDLE;
    VkPipelineLayout        _vkPipelineLayout       = VK_NULL_HANDLE;
    std::shared_ptr<Shader> _vertShader             = nullptr;
    std::shared_ptr<Shader> _fragShader             = nullptr;
};
