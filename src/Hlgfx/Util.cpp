#include "Include/Util.h"
#include <algorithm>
namespace hlgfx
{
void AddItem(std::vector<u8> &Blob, void *Item, sz Size)
{
    sz Cursor = Blob.size();
    Blob.resize(Cursor + Size);
    memcpy(Blob.data() + Cursor, Item, Size);
}

std::string FileNameFromPath(const std::string& fullPath) {
    // Find the last occurrence of the directory separator
    size_t lastSeparator = fullPath.find_last_of("/\\");
    if (lastSeparator == std::string::npos)
        lastSeparator = 0; // If no directory separator found, set it to 0 to start from the beginning of the string

    // Find the last occurrence of the extension separator
    size_t extensionStart = fullPath.find_last_of(".");
    if (extensionStart == std::string::npos || extensionStart < lastSeparator)
        extensionStart = fullPath.length(); // If no extension separator found or it's before the last separator, set it to the end of the string

    // Extract the file name without the extension
    return fullPath.substr(lastSeparator + 1, extensionStart - lastSeparator - 1);
}

}