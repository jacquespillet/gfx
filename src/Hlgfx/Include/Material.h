#pragma once
#include "Types.h"
#include "Gfx/Api.h"
#include "Texture.h"

namespace hlgfx
{
enum class materialType
{
    Unlit
};

struct materialFlags
{
    enum bits : u32
    {
        None          = 0 << 0,
        BlendEnabled  = 1 << 0,
        BlendDisabled = 1 << 1,
        CullModeOn    = 1 << 2,
        CullModeOff   = 1 << 3,
        Custom        = 1 << 4,
        Unlit         = 1 << 5,
    };
};

struct defaultTextures
{
    static std::shared_ptr<texture> BlackTexture;
    static std::shared_ptr<texture> BlueTexture;
    static std::shared_ptr<texture> WhiteTexture;
};

struct material
{
    gfx::pipelineHandle PipelineHandle;
    
    gfx::bufferHandle UniformBuffer;
    std::shared_ptr<gfx::uniformGroup> Uniforms;
    materialFlags::bits Flags;
    
    virtual void SetCullMode(gfx::cullMode Mode) = 0;
    virtual std::vector<u8> Serialize()=0;
    
};

struct unlitMaterial : public material
{
    unlitMaterial();
    unlitMaterial(materialFlags::bits Flags);
    ~unlitMaterial();
  
    virtual void SetCullMode(gfx::cullMode Mode);
    virtual std::vector<u8> Serialize() override;

    void SetDiffuseTexture(std::shared_ptr<texture> Texture);
    void SetMetallicRoughnessTexture(std::shared_ptr<texture> Texture);
    void SetOcclusionTexture(std::shared_ptr<texture> Texture);
    void SetNormalTexture(std::shared_ptr<texture> Texture);
    void SetEmissiveTexture(std::shared_ptr<texture> Texture);

    void Update();

    std::shared_ptr<texture> BaseColorTexture;
    std::shared_ptr<texture> MetallicRoughnessTexture;
    std::shared_ptr<texture> OcclusionTexture;
    std::shared_ptr<texture> NormalTexture;
    std::shared_ptr<texture> EmissiveTexture;

    struct materialData
    {
        f32 RoughnessFactor;
        f32 MetallicFactor;
        f32 EmissiveFactor;
        f32 AlphaCutoff;
        
        v3f BaseColorFactor;
        f32 OpacityFactor;

        s32 DebugChannel;
        v3i Padding0;

        f32 OcclusionStrength;
        v3f Emission;
    } UniformData;

};

}