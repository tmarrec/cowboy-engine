#include "Shader.h"

#include <sstream>
#include <fstream>

Shader::Shader(const std::string& vertexFilePath, const std::string& fragmentFilePath)
{
    std::string vertexCode;
    std::string fragmentCode;
    std::ifstream vShaderFile;
    std::ifstream fShaderFile;

    vShaderFile.open(vertexFilePath);
    fShaderFile.open(fragmentFilePath);

    std::stringstream vShaderStream, fShaderStream;
    vShaderStream << vShaderFile.rdbuf();
    fShaderStream << fShaderFile.rdbuf();		
    vShaderFile.close();
    fShaderFile.close();
    vertexCode   = vShaderStream.str();
    fragmentCode = fShaderStream.str();

    if (vertexCode.size() == 0)
    {
        ERROR("Unable to find the vertex shader \"" << vertexFilePath << "\"");
        return;
    }
    if (fragmentCode.size() == 0)
    {
        ERROR("Unable to find the fragment shader \"" << fragmentFilePath << "\"");
        return;
    }

    // Compile shaders
    // Vertex shader
    GLuint vertex = 0;
    const char* vShaderCode = vertexCode.c_str();
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, nullptr);
    glCompileShader(vertex);
    checkCompilation(vertex, GL_VERTEX_SHADER);

    // Fragment Shader
    GLuint fragment = 0;
    const char* fShaderCode = fragmentCode.c_str();
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, nullptr);
    glCompileShader(fragment);
    checkCompilation(fragment, GL_FRAGMENT_SHADER);

    // Shader Program linking
    _id = glCreateProgram();
    glAttachShader(_id, vertex);
    glAttachShader(_id, fragment);

    glLinkProgram(_id);
    checkCompilation(_id, GL_PROGRAM);

    // Delete shaders as they are linked into our program
    glDeleteShader(vertex);
    glDeleteShader(fragment);

    OK("Shader");
}

void Shader::setMat4f(const std::string& name, const glm::mat4& mat) const
{
    glUniformMatrix4fv(glGetUniformLocation(_id, name.c_str()), 1, GL_FALSE, glm::value_ptr(mat));
}

void Shader::use() const
{
    glUseProgram(_id);
}

void Shader::checkCompilation(const GLuint shader, const GLenum type) const
{
    GLint success;
    GLsizei infoLogLength = 0;
    switch (type)
    {
        case GL_PROGRAM:
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if (!success)
            {
                char* infoLog = new char[infoLogLength];
                glGetShaderInfoLog(shader, infoLogLength, NULL, infoLog);
                ERROR("Shader compilation error of type : " <<
                        type << "\n" << infoLog << "\n");
            }
            break;
        case GL_FRAGMENT_SHADER:
        case GL_VERTEX_SHADER:
            glGetProgramiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);
            glGetProgramiv(shader, GL_LINK_STATUS, &success);
            if (!success)
            {
                char* infoLog = new char[infoLogLength];
                glGetProgramInfoLog(shader, infoLogLength, NULL, infoLog);
                ERROR("Program linking error of type : " <<
                        type << "\n" << infoLog << "\n");
            }
            break;
    }
}
