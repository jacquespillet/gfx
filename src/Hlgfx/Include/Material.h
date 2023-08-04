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
        None                 = 0 << 0,
        BlendEnabled         = 1 << 0,
        Unlit                = 1 << 1,
        CullModeOn           = 1 << 3,
        DepthTestEnabled     = 1 << 4,
        DepthWriteEnabled    = 1 << 5,
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
    material(std::string Name);
    
    gfx::pipelineHandle PipelineHandle;
    gfx::bufferHandle UniformBuffer;
    std::shared_ptr<gfx::uniformGroup> Uniforms;
    materialFlags::bits Flags;
    std::string Name;
    std::string UUID;

    virtual void DrawGUI() = 0;
    virtual void SetCullMode(gfx::cullMode Mode) = 0;
    virtual std::vector<u8> Serialize()=0;
    virtual void RecreatePipeline() = 0;
    virtual std::shared_ptr<material> Clone() = 0;
    b8 ShouldRecreate = false;
    std::unordered_map<std::string, std::shared_ptr<texture>> AllTextures;

};

struct unlitMaterial : public material
{
    unlitMaterial(std::string Name);
    unlitMaterial(std::string Name, materialFlags::bits Flags);
    ~unlitMaterial();
  
    virtual void SetCullMode(gfx::cullMode Mode);
    virtual std::vector<u8> Serialize() override;
    virtual void RecreatePipeline() override;
    virtual std::shared_ptr<material> Clone() override;

    void SetBaseColorTexture(std::shared_ptr<texture> Texture);
    void SetMetallicRoughnessTexture(std::shared_ptr<texture> Texture);
    void SetOcclusionTexture(std::shared_ptr<texture> Texture);
    void SetNormalTexture(std::shared_ptr<texture> Texture);
    void SetEmissiveTexture(std::shared_ptr<texture> Texture);

    void Update();

    virtual void DrawGUI() override;
 
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

        v3f Emission;
        f32 OcclusionStrength;

        f32 DebugChannel; 
        f32 UseBaseColor;
        f32 UseEmissionTexture;
        f32 UseOcclusionTexture;
    } UniformData;

};

}