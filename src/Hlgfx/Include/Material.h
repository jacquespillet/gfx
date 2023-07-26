#pragma once
#include "Types.h"
#include "Gfx/Api.h"

namespace hlgfx
{
enum class materialType
{
    Unlit
};

struct material
{
    gfx::pipelineHandle PipelineHandle;
    
    gfx::bufferHandle UniformBuffer;
    std::shared_ptr<gfx::uniformGroup> Uniforms;

    virtual void SetCullMode(gfx::cullMode Mode) = 0;
    virtual std::vector<u8> Serialize()=0;

};

struct unlitMaterial : public material
{
    unlitMaterial();

    virtual void SetCullMode(gfx::cullMode Mode);
    virtual std::vector<u8> Serialize() override;
    struct materialData
    {
        v4f Color;
    } UniformData;

};

}