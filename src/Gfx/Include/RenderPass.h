#pragma once
#include <memory>
namespace gfx
{
struct renderPass
{  
    std::shared_ptr<void> ApiData;
    std::string Name;
};
}