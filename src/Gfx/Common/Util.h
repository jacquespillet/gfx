#pragma once
#include <string>
#include "../Include/Types.h"


namespace gfx
{

std::string ReadFileString(const char *FileName);

void WriteFileString(const char *FileName, std::string Content);

std::string ReadShaderFile(const char * FileName);

b8 FileExists(const char *FileName);

struct fileContent
{
    sz Size;
    u8 *Data;
};


fileContent ReadFileBinary(const char *FileName);
void WriteFileBinary(const char *FileName, u8 *Data, sz Size);

}