#include "Shader.h"

extern std::unique_ptr<LogicalDevice> g_logicalDevice;

static inline shaderc_shader_kind toShadercType(const ShaderType type)
{
    switch (type)
    {
        case SHADER_TYPE_VERTEX:
            return shaderc_glsl_vertex_shader;
        case SHADER_TYPE_FRAGMENT:
            return shaderc_glsl_fragment_shader;
        default:
            ERROR_EXIT("Wrong shader type.");
    }
}

Shader::Shader(const std::string& filename, const ShaderType type)
: _filename{filename}
, _type{type}
{
    compile();
    createShaderModule();
}

// Read the shader code from filename and compile it to spir-v bytecode
void Shader::compile()
{
    std::ifstream file("shaders/" + _filename, std::ios::in);

    if (!file.is_open())
    {
        ERROR_EXIT("Failed to read shader file " + _filename + ".");
    }

    // Get the shader code into the code string
    std::string code ((std::istreambuf_iterator<char>(file)),
                        (std::istreambuf_iterator<char>()));

    file.close();

    const shaderc::Compiler compiler;
    shaderc::CompileOptions options;
    options.SetOptimizationLevel(shaderc_optimization_level_performance);
    const auto shaderType = toShadercType(_type);

    // Preprocess
    const shaderc::PreprocessedSourceCompilationResult preprocessed = compiler.PreprocessGlsl(code, shaderType, _filename.c_str(), options);
    if (preprocessed.GetCompilationStatus() != shaderc_compilation_status_success)
    {
        ERROR(preprocessed.GetErrorMessage());
        if (_lastCompilationOk)
        {
            ERROR("Shader \"" + _filename + "\" preprocess failed. Will keep the last working shader bytecode.");
            return;
        }
        else
        {
            ERROR_EXIT("Shader preprocess failed.");
        }
    }

    // Compile
    const shaderc::CompilationResult assembly = compiler.CompileGlslToSpv(code, shaderType, _filename.c_str(), options);
    if (assembly.GetCompilationStatus() != shaderc_compilation_status_success)
    {
        if (_lastCompilationOk)
        {
            ERROR("Shader \"" + _filename + "\" compilation failed. Will keep the last working shader bytecode.");
            return;
        }
        else
        {
            ERROR_EXIT("Shader compilation failed.");
        }
    }
    
    _shaderCode = {assembly.cbegin(), assembly.cend()};
    _lastCompilationOk = true;
    OK("Shader \"" + _filename + "\" successfully compiled.");
}

// Create Vulkan shader module from shader bytecode
void Shader::createShaderModule()
{
    const VkShaderModuleCreateInfo createInfo =
    {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .pNext = VK_NULL_HANDLE,
        .flags = 0,
        .codeSize = _shaderCode.size() * sizeof(uint32_t),
        .pCode = _shaderCode.data(),
    };

    if (vkCreateShaderModule(g_logicalDevice->vkDevice(), &createInfo, VK_NULL_HANDLE, &_shaderModule) != VK_SUCCESS)
    {
        ERROR_EXIT("Failed to create shader module.");
    }
}

// Destroy the Vulkan shader module
void Shader::destroyShaderModule()
{
    vkDestroyShaderModule(g_logicalDevice->vkDevice(), _shaderModule, VK_NULL_HANDLE);
}

// Getter to the spir-v bytecode
const std::vector<uint32_t>& Shader::code() const
{
    return _shaderCode;
}

// Getter to the Vulkan shader module
const VkShaderModule& Shader::shaderModule() const
{
    return _shaderModule;
}
