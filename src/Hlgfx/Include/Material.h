#pragma once
#include "Types.h"
#include "Gfx/Api.h"

namespace hlgfx
{
enum class materialType
{
    Unlit
};

struct defaultTextures
{
    static gfx::imageHandle BlackTexture;
    static gfx::imageHandle BlueTexture;
    static gfx::imageHandle WhiteTexture;
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
    ~unlitMaterial();
  
    virtual void SetCullMode(gfx::cullMode Mode);
    virtual std::vector<u8> Serialize() override;

    void SetDiffuseTexture(std::shared_ptr<gfx::imageHandle> Texture);
    
    std::shared_ptr<gfx::imageHandle> DiffuseTexture;

    struct materialData
    {
        v4f Color;
    } UniformData;

};

}