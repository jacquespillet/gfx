#pragma once
#include <string>
#include "../Include/Types.h"
#include <Windows.h>

namespace gfx
{

std::string ReadFileString(const char *FileName);

void WriteFileString(const char *FileName, std::string Content);

std::string ReadShaderFile(const char * FileName);

b8 FileExists(const char *FileName);


//TODO: Allocate using heap allocator in these functions to keep track of memory
LPCWSTR ConstCharToLPCWSTR(const char* narrowString);
std::wstring LPCSTRToWString(LPCSTR str);


struct fileContent
{
    sz Size;
    u8 *Data;
};


fileContent ReadFileBinary(const char *FileName);
void WriteFileBinary(const char *FileName, u8 *Data, sz Size);

}