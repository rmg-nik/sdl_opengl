#ifndef GL_PROGRAM_HPP
#define GL_PROGRAM_HPP

#include <string>
#ifdef WIN32
#include <GL/glew.h>
#endif

class GLProgram
{
public:
    
    GLProgram();

    ~GLProgram();

    bool InitWithFiles(const std::string& vertexShaderFileName, const std::string& fragmentShaderFileName);

    bool InitWithData(const std::string& vertexShaderData, const std::string& fragmentShaderData);

    bool IsInitialized() const;

    const std::string& GetError() const;

    void Use();

    GLuint GetProgram() const;

private:

    bool CheckShaderCompilationStatus(GLuint shader);

    bool CheckProgramLinkageStatus(GLuint program);

private:

    bool m_initialized;

    GLuint m_program;

    std::string m_error;
};
#endif