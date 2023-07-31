#include "Util.h"
#include <fstream>
#include "../Include/Types.h"
#include "../Include/Memory.h"

namespace gfx
{
    
std::string ReadFileString(const char *FileName)
{
    std::ifstream InputFile(FileName);
    std::string contents((std::istreambuf_iterator<char>(InputFile)), std::istreambuf_iterator<char>());
    return contents;
}


void WriteFileString(const char *FileName, std::string Content)
{
    std::ofstream OutputFile(FileName);
    OutputFile << Content;
    OutputFile.close();
}


void GetFilePath(const std::string& FullPath, std::string& PathWithoutFileName)
{
    size_t found = FullPath.find_last_of("/\\");
    PathWithoutFileName = FullPath.substr(0, found + 1);
} 

std::string RemoveQuotationMarks(std::string Input)
{
    Input.erase(std::remove(Input.begin(), Input.end(), '\"'), Input.end());
    Input.erase(std::remove(Input.begin(), Input.end(), '\''), Input.end());

    return Input;
}


std::string ReadShaderFile(const char * FileName)
{
    std::string IncludeIdentifier = "#include ";
    static b8 RecursiveCall=false;

    std::string FullSource = "";
    std::ifstream File(FileName);

    if(!File.is_open())
    {
        printf("Could not open shader File %s", FileName);
        return FullSource.c_str();
    }

    std::string LineBuffer;
    while(std::getline(File, LineBuffer))
    {

#if GFX_API != GFX_D3D12 //hlsl has its own include handler
        if(LineBuffer.find(IncludeIdentifier) != LineBuffer.npos)
        {
            LineBuffer.erase(0, IncludeIdentifier.size());

            std::string PathOfThisFile;
            GetFilePath(FileName, PathOfThisFile);
            LineBuffer.insert(0, PathOfThisFile);
            LineBuffer = RemoveQuotationMarks(LineBuffer);

            RecursiveCall = true;
            FullSource += ReadShaderFile(LineBuffer.c_str());

            continue;
        }
#endif

        FullSource += LineBuffer + "\n";
    }

    if(RecursiveCall)
    {
        FullSource += "\0";
    }

    File.close();

    return FullSource;
}    

b8 FileExists(const char *FileName)
{
	if (FileName == nullptr) return false;
    std::ifstream File(FileName);
    if(!File.is_open())
    {
        return false;
    }

    return true;
}

LPCWSTR ConstCharToLPCWSTR(const char* narrowString) {
    int bufferSize = MultiByteToWideChar(CP_UTF8, 0, narrowString, -1, nullptr, 0);
    wchar_t* wideString = new wchar_t[bufferSize];
    MultiByteToWideChar(CP_UTF8, 0, narrowString, -1, wideString, bufferSize);
    return wideString;
}

std::wstring LPCSTRToWString(LPCSTR str)
{
    // Determine the required length of the wide string
    int wideStrLength = MultiByteToWideChar(CP_ACP, 0, str, -1, nullptr, 0);

    // Allocate memory for the wide string
    wchar_t* wideStrBuffer = new wchar_t[wideStrLength];

    // Convert the narrow string to wide string
    MultiByteToWideChar(CP_ACP, 0, str, -1, wideStrBuffer, wideStrLength);

    // Create a wstring from the wide string
    std::wstring wideStr(wideStrBuffer);

    // Clean up the allocated memory
    delete[] wideStrBuffer;

    return wideStr;
}



fileContent ReadFileBinary(const char *FileName)
{
    fileContent Result = {};
    std::ifstream File(FileName, std::ios::binary);
    if(!File.is_open())
    {
        return Result;
    }
    File.seekg(0, std::ios::end);
    Result.Size = File.tellg();
    File.seekg(0, std::ios::beg);
    Result.Data = (u8*)AllocateMemory(Result.Size);
    File.read((char*)Result.Data, Result.Size);
    File.close();
    return Result;
}

void WriteFileBinary(const char *FileName, u8 *Data, sz Size)
{
    fileContent Result = {};
    std::ofstream File(FileName, std::ios::binary);
    if(!File.is_open())
    {
        return;
    }
    File.write((char*)Data, Size);
    File.close();
}


u8 ChannelCountFromFormat(format Format)
{
    return 4;
}

sz FormatSize(format Format)
{
    return 4;
}




}