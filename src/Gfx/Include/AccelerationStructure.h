#pragma once

namespace gfx
{
struct accelerationStructure
{
    std::shared_ptr<void> ApiData = nullptr;

    b8 NeedsDescriptorUpdate=false;
};
}