#pragma once
#include <memory>
#include <string>

namespace gfx
{
struct renderPass
{  
    std::shared_ptr<void> ApiData;
    std::string Name;
};
}