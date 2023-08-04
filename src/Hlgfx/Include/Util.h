#pragma once
#include "Types.h"
#include <vector>
#include <string>

namespace hlgfx
{
void AddItem(std::vector<u8> &Blob, void *Item, sz Size);

std::string FileNameFromPath(const std::string& fullPath);

}