#include "Shader.h"
#include <shaderc/shaderc.hpp>

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

    shaderc::Compiler compiler;
    shaderc::CompileOptions options;
    options.SetOptimizationLevel(shaderc_optimization_level_performance);
    const auto shaderType = [&]()
    {
        switch (_type)
        {
            case SHADER_TYPE_VERTEX:
                return shaderc_glsl_vertex_shader;
            case SHADER_TYPE_FRAGMENT:
                return shaderc_glsl_fragment_shader;
            default:
                ERROR_EXIT("Wrong shader type.");
        }
    }();

    // Preprocess
    shaderc::PreprocessedSourceCompilationResult preprocessed = compiler.PreprocessGlsl(code, shaderType, _filename.c_str(), options);
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
    shaderc::CompilationResult assembly = compiler.CompileGlslToSpv(code, shaderType, _filename.c_str(), options);
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
    INFO("Shader \"" + _filename + "\" successfully compiled.");
}

// Getter to the spir-v bytecode
const std::vector<std::uint32_t>& Shader::code() const
{
    return _shaderCode;
}
