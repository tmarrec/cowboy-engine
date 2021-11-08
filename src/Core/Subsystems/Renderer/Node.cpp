#include "Node.h"
#include "Mesh.h"

#include <cstdint>
#include <iostream>

#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/quaternion.hpp>
#include <memory>

Node::Node(const int idx, const tinygltf::Model& model, std::vector<std::uint16_t>& indicesBuffer, std::vector<float>& vertexBuffer, std::vector<Primitive>& primitives)
{
    const auto& node = model.nodes[idx];
    for (const auto childrenIdx : node.children)
    {
        const Node node {childrenIdx, model, indicesBuffer, vertexBuffer, primitives};
        _nodes.emplace_back(node);
    }

    std::cout << node.name << std::endl; 

    // If local transform matrix specified
    if (!node.matrix.empty())
    {
        for (std::uint8_t j = 0; j < 4; ++j)
        {
            for (std::uint8_t i = 0; i < 4; ++i)
            {
                _transform[j][i] = node.matrix[i+j*4];
            }
        }
    }
    // If need to build the local transform matrix
    else
    {
        glm::mat4 translation {0} ;
        if (!node.translation.empty())
        {
            translation = glm::translate(glm::mat4(), glm::vec3(node.translation[0], node.translation[1], node.translation[2]));
        }
        glm::mat4 rotation {0};
        if (!node.rotation.empty())
        {
            rotation = glm::toMat4(glm::quat(node.rotation[0],node.rotation[1],node.rotation[2],node.rotation[3]));
        }
        glm::mat4 scale {0};
        if (!node.scale.empty())
        {
            scale = glm::scale(glm::mat4(), glm::vec3(node.scale[0], node.scale[1], node.scale[2]));
        }
        _transform = translation * rotation * scale;
    }

    std::cout << glm::to_string(_transform) << std::endl;

    _mesh = std::make_shared<Mesh>(node.mesh, model, indicesBuffer, vertexBuffer, primitives);
}

