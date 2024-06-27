#pragma once
#include "Types.h"
#include "Gfx/Api.h"
#include "Texture.h"

#include <string>

namespace hlgfx
{
enum class materialType
{
    PBR
};

struct materialFlags
{
    enum bits : u32
    {
        None                 = 0 << 0,
        BlendEnabled         = 1 << 0,
        PBR                  = 1 << 1,
        CullModeOn           = 1 << 3,
        DepthTestEnabled     = 1 << 4,
        DepthWriteEnabled    = 1 << 5,
        GBuffer              = 1 << 6,
    };
};

struct defaultTextures
{
    static std::shared_ptr<texture> BlackTexture;
    static std::shared_ptr<texture> BlueTexture;
    static std::shared_ptr<texture> WhiteTexture;
    static std::shared_ptr<texture> PurpleTexture;
};

struct material
{
    material(std::string Name);
    
    gfx::pipelineHandle PipelineHandle = gfx::InvalidHandle;
    gfx::bufferHandle UniformBuffer = gfx::InvalidHandle;
    std::shared_ptr<gfx::uniformGroup> Uniforms;
    materialFlags::bits Flags;
    std::string Name;
    u32 ID;

    virtual void DrawGUI() = 0;
    virtual void SetCullMode(gfx::cullMode Mode) = 0;
    virtual void Serialize(const std::string &FileName)=0;
    virtual void RecreatePipeline() = 0;
    virtual std::shared_ptr<material> Clone() = 0;
    static std::shared_ptr<material> Deserialize(const std::string &FileName);
    b8 ShouldRecreate = false;
    b8 ShouldUpdateUniforms = false;
    std::unordered_map<u32, std::shared_ptr<texture>> AllTextures;
};

struct customMaterial : public material
{
    customMaterial(std::string Name, gfx::pipelineHandle Pipeline);
    
    ~customMaterial();
    virtual void DrawGUI() override;
    virtual void SetCullMode(gfx::cullMode Mode) override;
    virtual void Serialize(const std::string &FileName) override;
    virtual void RecreatePipeline() override;
    virtual std::shared_ptr<material> Clone() override;
};

struct pbrMaterial : public material
{
    pbrMaterial(std::string Name);
    pbrMaterial(std::string Name, materialFlags::bits Flags);
    ~pbrMaterial();
  
    virtual void SetCullMode(gfx::cullMode Mode);
    virtual void Serialize(const std::string &FileName) override;
    virtual void RecreatePipeline() override;
    virtual std::shared_ptr<material> Clone() override;

    void SetBaseColorTexture(std::shared_ptr<texture> Texture);
    void SetMetallicRoughnessTexture(std::shared_ptr<texture> Texture);
    void SetOcclusionTexture(std::shared_ptr<texture> Texture);
    void SetNormalTexture(std::shared_ptr<texture> Texture);
    void SetEmissiveTexture(std::shared_ptr<texture> Texture);

    void Update();

    std::shared_ptr<texture> SelectedTexture = nullptr;
    b8 ShowTextureSelection(const char *ID, std::shared_ptr<texture> &Texture);
    bool DrawTexture(const char *Name, std::shared_ptr<texture> &Texture, f32 &Use);
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

        f32 UseNormalTexture;
        f32 UseMetallicRoughnessTexture;
        f32 ColourTextureID;
        f32 Padding;
    } UniformData;

};

}