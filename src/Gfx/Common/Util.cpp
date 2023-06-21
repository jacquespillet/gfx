#include "Util.h"
#include <fstream>
#include "../Include/Types.h"

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

        FullSource += LineBuffer + "\n";
    }

    if(RecursiveCall)
    {
        FullSource += "\0";
    }

    File.close();

    return FullSource;
}    


}