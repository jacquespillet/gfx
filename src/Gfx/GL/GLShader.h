#pragma once
#include "../Include/Types.h"

#include "GL/glew.h"
#include <memory>
namespace gfx
{
struct glShader
{
    glShader() = default;
    glShader(char * SourceFile, shaderStageFlags::value Type);
    void SetSource(const char * Source, shaderStageFlags::value Type);
    void Compile();
    
    bool Compiled=false; 
    bool InError=false;
    
    const char* Source;
    

    shaderStageFlags::value Type;
    
    //GL
    GLint ShaderObject;
};

struct glShaderProgram
{
    glShaderProgram() = default;
    glShaderProgram(glShader *VertexShader, glShader *GeometryShader, glShader *FragmentShader);
    void SetSourceFiles(char* VertexShaderSourceFile, char* GeometryShaderSourceFile, char* FragmentShaderSourceFile);
    void SetSource(char* VertexShaderSourceCode, char* GeometryShaderSourceCode, char* FragmentShaderSourceCode);
    void SetCodeForStage(const char *Code, shaderStageFlags::value Stage);
    void Compile();
    
    bool Compiled; 
    
    //shader programs

    std::shared_ptr<glShader> VertexShader = nullptr;
    std::shared_ptr<glShader> GeometryShader = nullptr;
    std::shared_ptr<glShader> FragmentShader = nullptr;
    

    void Bind();
    void Unbind();
    
    //GL
    GLint ProgramShaderObject;
};
}