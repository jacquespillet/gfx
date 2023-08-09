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

#if GFX_API != GFX_D3D12 && GFX_API != GFX_D3D11 //hlsl has its own include handler
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

//TODO
u8 ChannelCountFromFormat(format Format)
{
    return 4;
}

//TODO
sz FormatSize(format Format)
{
    return 4;
}

const char *FormatStringMapping[] = 
{
    "UNDEFINED",
    "R4G4_UNORM_PACK_8",
    "R4G4B4A4_UNORM_PACK_16",
    "B4G4R4A4_UNORM_PACK_16",
    "R5G6B5_UNORM_PACK_16",
    "B5G6R5_UNORM_PACK_16",
    "R5G5B5A1_UNORM_PACK_16",
    "B5G5R5A1_UNORM_PACK_16",
    "A1R5G5B5_UNORM_PACK_16",
    "R8_UNORM",
    "R8_SNORM",
    "R8_USCALED",
    "R8_SSCALED",
    "R8_UINT",
    "R8_SINT",
    "R8_SRGB",
    "R8G8_UNORM",
    "R8G8_SNORM",
    "R8G8_USCALED",
    "R8G8_SSCALED",
    "R8G8_UINT",
    "R8G8_SINT",
    "R8G8_SRGB",
    "R8G8B8_UNORM",
    "R8G8B8_SNORM",
    "R8G8B8_USCALED",
    "R8G8B8_SSCALED",
    "R8G8B8_UINT",
    "R8G8B8_SINT",
    "R8G8B8_SRGB",
    "B8G8R8_UNORM",
    "B8G8R8_SNORM",
    "B8G8R8_USCALED",
    "B8G8R8_SSCALED",
    "B8G8R8_UINT",
    "B8G8R8_SINT",
    "B8G8R8_SRGB",
    "R8G8B8A8_UNORM",
    "R8G8B8A8_SNORM",
    "R8G8B8A8_USCALED",
    "R8G8B8A8_SSCALED",
    "R8G8B8A8_UINT",
    "R8G8B8A8_SINT",
    "R8G8B8A8_SRGB",
    "B8G8R8A8_UNORM",
    "B8G8R8A8_SNORM",
    "B8G8R8A8_USCALED",
    "B8G8R8A8_SSCALED",
    "B8G8R8A8_UINT",
    "B8G8R8A8_SINT",
    "B8G8R8A8_SRGB",
    "A8B8G8R8_UNORM_PACK_32",
    "A8B8G8R8_SNORM_PACK_32",
    "A8B8G8R8_USCALED_PACK_32",
    "A8B8G8R8_SSCALED_PACK_32",
    "A8B8G8R8_UINT_PACK_32",
    "A8B8G8R8_SINT_PACK_32",
    "A8B8G8R8_SRGB_PACK_32",
    "A2R10G10B10_UNORM_PACK_32",
    "A2R10G10B10_SNORM_PACK_32",
    "A2R10G10B10_USCALED_PACK_32",
    "A2R10G10B10_SSCALED_PACK_32",
    "A2R10G10B10_UINT_PACK_32",
    "A2R10G10B10_SINT_PACK_32",
    "A2B10G10R10_UNORM_PACK_32",
    "A2B10G10R10_SNORM_PACK_32",
    "A2B10G10R10_USCALED_PACK_32",
    "A2B10G10R10_SSCALED_PACK_32",
    "A2B10G10R10_UINT_PACK_32",
    "A2B10G10R10_SINT_PACK_32",
    "R16_UNORM",
    "R16_SNORM",
    "R16_USCALED",
    "R16_SSCALED",
    "R16_UINT",
    "R16_SINT",
    "R16_SFLOAT",
    "R16G16_UNORM",
    "R16G16_SNORM",
    "R16G16_USCALED",
    "R16G16_SSCALED",
    "R16G16_UINT",
    "R16G16_SINT",
    "R16G16_SFLOAT",
    "R16G16B16_UNORM",
    "R16G16B16_SNORM",
    "R16G16B16_USCALED",
    "R16G16B16_SSCALED",
    "R16G16B16_UINT",
    "R16G16B16_SINT",
    "R16G16B16_SFLOAT",
    "R16G16B16A16_UNORM",
    "R16G16B16A16_SNORM",
    "R16G16B16A16_USCALED",
    "R16G16B16A16_SSCALED",
    "R16G16B16A16_UINT",
    "R16G16B16A16_SINT",
    "R16G16B16A16_SFLOAT",
    "R32_UINT",
    "R32_SINT",
    "R32_SFLOAT",
    "R32G32_UINT",
    "R32G32_SINT",
    "R32G32_SFLOAT",
    "R32G32B32_UINT",
    "R32G32B32_SINT",
    "R32G32B32_SFLOAT",
    "R32G32B32A32_UINT",
    "R32G32B32A32_SINT",
    "R32G32B32A32_SFLOAT",
    "R64_UINT",
    "R64_SINT",
    "R64_SFLOAT",
    "R64G64_UINT",
    "R64G64_SINT",
    "R64G64_SFLOAT",
    "R64G64B64_UINT",
    "R64G64B64_SINT",
    "R64G64B64_SFLOAT",
    "R64G64B64A64_UINT",
    "R64G64B64A64_SINT",
    "R64G64B64A64_SFLOAT",
    "B10G11R11_UFLOAT_PACK_32",
    "E5B9G9R9_UFLOAT_PACK_32",
    "D16_UNORM",
    "X8D24_UNORM_PACK_32",
    "D32_SFLOAT",
    "S8_UINT",
    "D16_UNORM_S8_UINT",
    "D24_UNORM_S8_UINT",
    "D32_SFLOAT_S8_UINT",
};

const char *FormatToString(format Format)
{
    return FormatStringMapping[(u32)Format];
}


}