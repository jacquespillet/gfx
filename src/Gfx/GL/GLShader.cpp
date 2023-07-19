#include "GLShader.h"

#include <string.h>
#include <stdio.h>
#include <fstream>
#include <sstream>
#include <algorithm>

#include "../Common/Util.h"
#include "GLMapping.h"

namespace gfx
{




glShader::glShader(char* SourceFile, shaderStageFlags::value Type)
{
    this->Type = Type;
    this->Source = ReadFileString(SourceFile).c_str();
    Compile();
}

void glShader::SetSource(const char* Source, shaderStageFlags::value Type)
{
    this->Type = Type;
    this->Source = Source;
}


void glShader::Compile()
{
	ShaderObject = glCreateShader(ShaderStageToNative(Type));
	glShaderSource(ShaderObject, 1, &Source, NULL);
	glCompileShader(ShaderObject);

    GLint CompileStatus;
    glGetShaderiv(ShaderObject, GL_COMPILE_STATUS, &CompileStatus);
	if (CompileStatus != GL_TRUE) 
	{
        char Log[5000];
        int LogLength; 
        glGetShaderInfoLog(ShaderObject, 5000, &LogLength, Log);
        printf("shader log: %s\n", Log);
    }	
	Compiled = true;
}


//
glShaderProgram::glShaderProgram(glShader *VertexShader, glShader *GeometryShader, glShader *FragmentShader) :
                                VertexShader(VertexShader),
                                GeometryShader(GeometryShader),
                                FragmentShader(FragmentShader)
                                {}

void glShaderProgram::SetSourceFiles(char* VertexShaderSourceFile, char* GeometryShaderSourceFile, char* FragmentShaderSourceFile)
{
    assert(!this->VertexShader); assert(!this->FragmentShader); assert(!this->GeometryShader);
    this->VertexShader = std::make_shared<glShader>(VertexShaderSourceFile, shaderStageFlags::Vertex);
    this->GeometryShader = std::make_shared<glShader>(GeometryShaderSourceFile, shaderStageFlags::Geometry);
    this->FragmentShader = std::make_shared<glShader>(FragmentShaderSourceFile, shaderStageFlags::Fragment);
}

void glShaderProgram::SetSource(char* VertexShaderSource, char* GeometryShaderSource, char* FragmentShaderSource)
{
    assert(!this->VertexShader); assert(!this->FragmentShader); assert(!this->GeometryShader);
    this->VertexShader = std::make_shared<glShader>();
    this->FragmentShader = std::make_shared<glShader>();
    this->GeometryShader = std::make_shared<glShader>();

    this->VertexShader->SetSource(VertexShaderSource, shaderStageFlags::Vertex);
    this->GeometryShader->SetSource(GeometryShaderSource, shaderStageFlags::Geometry);
    this->FragmentShader->SetSource(FragmentShaderSource, shaderStageFlags::Fragment);
}

void glShaderProgram::SetCodeForStage(const char *Code, shaderStageFlags::value Stage)
{
    switch (Stage)
    {
    case shaderStageFlags::Vertex:
        this->VertexShader = std::make_shared<glShader>();
        this->VertexShader->SetSource(Code, Stage);
        break;
    case shaderStageFlags::Fragment:
        this->FragmentShader = std::make_shared<glShader>();
        this->FragmentShader->SetSource(Code, Stage);
        break;
    case shaderStageFlags::Compute:
        this->ComputeShader = std::make_shared<glShader>();
        this->ComputeShader->SetSource(Code, Stage);
        break;
    }
}


void glShaderProgram::Compile()
{
    if(ComputeShader != nullptr && !ComputeShader->Compiled)
    {
        ComputeShader->Compile();
        ProgramShaderObject = glCreateProgram();
	    glAttachShader(ProgramShaderObject, ComputeShader->ShaderObject);
		glLinkProgram(ProgramShaderObject);
    }
    else
    {
        if(FragmentShader != nullptr && !FragmentShader->Compiled) FragmentShader->Compile();
        if(GeometryShader != nullptr && !GeometryShader->Compiled) GeometryShader->Compile();
        if(VertexShader != nullptr && !VertexShader->Compiled) VertexShader->Compile();

        bool HasGeometry= (GeometryShader != nullptr && GeometryShader->Compiled); 
        //link vertex and fragment shader to create shader program object
        ProgramShaderObject = glCreateProgram();
        glAttachShader(ProgramShaderObject, VertexShader->ShaderObject);
        glAttachShader(ProgramShaderObject, FragmentShader->ShaderObject);
        if(HasGeometry) glAttachShader(ProgramShaderObject, GeometryShader->ShaderObject);
        glLinkProgram(ProgramShaderObject);
    }
	
	//Check status of shader and log any compile time errors
    printf("Shader:Compile: checking shader status\n");
	int LinkStatus;
	glGetProgramiv(ProgramShaderObject, GL_LINK_STATUS, &LinkStatus);
	if (LinkStatus != GL_TRUE) 
	{
		char Log[5000];
		int LogLentgh; 
		glGetProgramInfoLog(ProgramShaderObject, 5000, &LogLentgh, Log);
		printf("Shader:Compile: Could not link program:\n %s \n", Log);
		glDeleteProgram(ProgramShaderObject);
		ProgramShaderObject = 0;
	}
	else
	{
		printf("Shader:Compile: compile success\n");
		Compiled = true; 
	}
}

void glShaderProgram::Bind()
{
    glUseProgram(ProgramShaderObject);
}

void glShaderProgram::Unbind()
{
    glUseProgram(0);
}






}