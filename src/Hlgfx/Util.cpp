#include "Include/Util.h"
#include <algorithm>
#include <filesystem>

namespace hlgfx
{
void AddItem(std::vector<u8> &Blob, void *Item, sz Size)
{
    sz Cursor = Blob.size();
    Blob.resize(Cursor + Size);
    memcpy(Blob.data() + Cursor, Item, Size);
}

std::string FileNameFromPath(const std::string& FullPath) {
    // Find the last occurrence of the directory separator
    size_t lastSeparator = FullPath.find_last_of("/\\");
    if (lastSeparator == std::string::npos)
        lastSeparator = 0; // If no directory separator found, set it to 0 to start from the beginning of the string

    // Find the last occurrence of the extension separator
    size_t extensionStart = FullPath.find_last_of(".");
    if (extensionStart == std::string::npos || extensionStart < lastSeparator)
        extensionStart = FullPath.length(); // If no extension separator found or it's before the last separator, set it to the end of the string

    // Extract the file name without the extension
    return FullPath.substr(lastSeparator + 1, extensionStart - lastSeparator - 1);
}

std::string PathFromFile(const std::string &FullPath)
{
    std::filesystem::path filePath = FullPath;
    std::filesystem::path directoryPath = filePath.parent_path();
    return directoryPath.string();    
}

}