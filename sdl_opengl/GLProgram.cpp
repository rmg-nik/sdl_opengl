#include "GLProgram.hpp"
#include <stdexcept>
#include <fstream>
#include <sstream>

GLProgram::GLProgram():m_initialized{false}
{

}

GLProgram::~GLProgram()
{
    if (m_initialized)
        glDeleteProgram(m_program);
}

bool GLProgram::InitWithFiles(const std::string& vertexShaderFileName, const std::string& fragmentShaderFileName)
{
    try
    {
        std::ostringstream vertexShaderData;
        std::ostringstream fragmentShaderData;
        std::ifstream vertexShaderFile(vertexShaderFileName);
        std::ifstream fragmentShaderFile(fragmentShaderFileName);
        
        vertexShaderData << vertexShaderFile.rdbuf();
        fragmentShaderData << fragmentShaderFile.rdbuf();
        return InitWithData(vertexShaderData.str(), fragmentShaderData.str());
    }
    catch (const std::exception& ex)
    {
        m_error = "Error while GLProgram::InitWithFiles: " + std::string(ex.what());
    }    
    return m_initialized;
}

bool GLProgram::InitWithData(const std::string& vertexShaderData, const std::string& fragmentShaderData)
{
    try
    {
        GLuint vertexShader;
        vertexShader = glCreateShader(GL_VERTEX_SHADER);
        const GLchar* vdata = vertexShaderData.c_str();
        glShaderSource(vertexShader, 1, &vdata, nullptr);
        glCompileShader(vertexShader);
        if (!CheckShaderCompilationStatus(vertexShader))
            return false;

        GLuint fragmentShader;
        fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        const GLchar* fdata = fragmentShaderData.c_str();
        glShaderSource(fragmentShader, 1, &fdata, nullptr);
        glCompileShader(fragmentShader);
        if (!CheckShaderCompilationStatus(fragmentShader))
        {
            glDeleteShader(vertexShader);
            return false;
        }

        m_program = glCreateProgram();

        glAttachShader(m_program, vertexShader);
        glAttachShader(m_program, fragmentShader);
        glLinkProgram(m_program);

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        if (!CheckProgramLinkageStatus(m_program))
            return false;

        m_initialized = true;
    }
    catch (const std::exception& ex)
    {
        m_error = "Error while GLProgram::InitWithData: " + std::string(ex.what());
    }
    return m_initialized;
}

bool GLProgram::IsInitialized() const
{
    return m_initialized;
}

const std::string& GLProgram::GetError() const
{
    return m_error;
}

void GLProgram::Use()
{
    if (!m_initialized)
        throw std::runtime_error("GLProgram is not initialized");

    glUseProgram(m_program);
}

GLuint GLProgram::GetProgram() const
{
    return m_program;
}

bool GLProgram::CheckShaderCompilationStatus(GLuint shader)
{
    GLint success;
    GLchar buffer[255];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(shader, sizeof(buffer), nullptr, buffer);
        m_error = buffer;
        return false;
    }
    return true;
}

bool GLProgram::CheckProgramLinkageStatus(GLuint program)
{
    GLint success;
    GLchar buffer[255];
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(program, sizeof(buffer), nullptr, buffer);
        m_error = buffer;
        return false;
    }
    return true;
}

