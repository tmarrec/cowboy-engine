#pragma once

#include <fstream>
#include <shaderc/shaderc.hpp>
#include <vulkan/vulkan.h>

#include "../../utils.h"

enum ShaderType
{
    SHADER_TYPE_VERTEX,
    SHADER_TYPE_FRAGMENT
};

// Class handling the shaders reading
// Read the GLSL code and compile it to SPIR-V bytecode
class Shader
{
 public:
    Shader(const std::string& filename, const ShaderType type, const VkDevice& device);
    void compile();
    const std::vector<uint32_t>& code() const;
    const VkShaderModule& shaderModule() const;
    void destroyShaderModule();

 private:
    void createShaderModule();

    const std::string       _filename;
    const ShaderType        _type;
    const VkDevice&         _device;
    std::vector<uint32_t>   _shaderCode = {};
    bool                    _lastCompilationOk = false;
    VkShaderModule          _shaderModule;
};
