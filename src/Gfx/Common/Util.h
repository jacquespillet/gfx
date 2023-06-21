#pragma once
#include <string>

namespace gfx
{

std::string ReadFileString(const char *FileName);

void WriteFileString(const char *FileName, std::string Content);

std::string ReadShaderFile(const char * FileName);


}