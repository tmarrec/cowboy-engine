#include "./Node.h"
#include "./Mesh.h"

#include <cstdint>
#include <iostream>

#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/quaternion.hpp>
#include <memory>

Node::Node(const int idx, const tinygltf::Model& model, std::vector<uint16_t>& indicesBuffer, std::vector<float>& vertexBuffer, std::vector<Primitive>& primitives, std::vector<Node>& nodes, const glm::mat4& parentTransform)
{
    const auto& node = model.nodes[idx];
    INFO("Loading node \"" << node.name << "\""); 
    
    // If local transform matrix specified
    if (!node.matrix.empty())
    {
        for (uint8_t j = 0; j < 4; ++j)
        {
            for (uint8_t i = 0; i < 4; ++i)
            {
                _transform[j][i] = node.matrix[i+j*4];
            }
        }
    }
    // If need to build the local transform matrix
    else
    {
        _transform = glm::mat4(1.0f);
        if (!node.translation.empty())
        {
            glm::mat4 translation {1};
            translation[3][0] = node.translation[0];
            translation[3][1] = node.translation[1];
            translation[3][2] = node.translation[2];
            _transform *= translation;
        }
        if (!node.rotation.empty())
        {
            const glm::mat4 rotation = glm::toMat4(glm::quat(node.rotation[3],node.rotation[0],node.rotation[1],node.rotation[2]));
            _transform *= rotation;
        }
        if (!node.scale.empty())
        {
            glm::mat4 scale {1};
            scale[0][0] = node.scale[0];
            scale[1][1] = node.scale[1];
            scale[2][2] = node.scale[2];
            _transform *= scale;
        }
    }

    _transform = parentTransform * _transform;

    for (const auto childrenIdx : node.children)
    {
        const Node node {childrenIdx, model, indicesBuffer, vertexBuffer, primitives, nodes, _transform};
        nodes.emplace_back(node);
    }

    if (node.mesh >= 0)
    {
        _mesh = std::make_shared<Mesh>(node.mesh, model, indicesBuffer, vertexBuffer, primitives);
    }
}

const std::vector<Primitive>& Node::getPrimitives() const
{
    return _mesh->getPrimitives();
}

const glm::mat4& Node::getTransform() const
{
    return _transform;
}

const bool Node::gotMesh() const
{
    return _mesh != nullptr;
}
