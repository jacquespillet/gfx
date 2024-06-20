struct normalInfo {
    vec3 GeometricNormal;  
    vec3 Tangent;   
    vec3 Bitangent;   
    vec3 ShadingNormal;   
    vec3 TextureNormal;
};

struct materialInfo
{
    float ior;
    float PerceptualRoughness;      // roughness value, as authored by the model creator (input to shader)
    vec3 f0;                        // full Reflectance color (n incidence angle)

    float AlphaRoughness;           // roughness mapped to a more linear change in the roughness (proposed by [2])
    vec3 CDiff;

    vec3 F90;                       // Reflectance color at grazing angle
    float Metallic;

    vec3 BaseColor;

    // float sheenRoughnessFactor;
    // vec3 sheenColorFactor;

    vec3 ClearcoatF0;
    vec3 ClearcoatF90;
    float ClearcoatFactor;
    vec3 ClearcoatNormal;
    float ClearcoatRoughness;

    // KHR_materials_specular 
    float SpecularWeight; // product of specularFactor and specularTexture.a

    // float transmissionFactor;

    // float thickness;
    // vec3 attenuationColor;
    // float attenuationDistance;

    // KHR_materials_iridescence
    // float iridescenceFactor;
    // float iridescenceIOR;
    // float iridescenceThickness;
};

#ifndef DEFERRED
vec4 GetBaseColor(vec2 FragUV)
{
    vec4 BaseColor = vec4(1,1,1,1);

    BaseColor = vec4(Material.BaseColorFactor, Material.OpacityFactor);

    vec4 SampleCol = SampleTexture(BaseColorTexture, DefaultSampler, FragUV);
    SampleCol.rgb = pow(SampleCol.rgb, vec3(2.2,2.2,2.2));
    SampleCol = mix(vec4(1,1,1,1), SampleCol, Material.UseBaseColor);
    BaseColor.rgb *= SampleCol.rgb;
    BaseColor.a *= SampleCol.a;
    return BaseColor;
}
normalInfo GetNormalInfo(vec3 T, vec3 B, vec3 N, vec2 FragUV)
{
    vec3 ShadingNormal, Tangent, Bitangent, GeometricNormal;
   
    Tangent = normalize(T);
    Bitangent = normalize(B);
    GeometricNormal = normalize(N);

    normalInfo NormalInfo;
    NormalInfo.GeometricNormal = GeometricNormal;
    NormalInfo.ShadingNormal = GeometricNormal;


    vec3 NormalSample = SampleTexture(NormalTexture, DefaultSampler, FragUV).rgb * vec3(2.0,2.0,2.0) - vec3(1.0,1.0,1.0);
    NormalInfo.TextureNormal = normalize(NormalSample);
    NormalInfo.TextureNormal = mix(vec3(0,0,1), NormalInfo.TextureNormal, Material.UseNormalTexture);
    mat3 TBN = mat3(Tangent, Bitangent, GeometricNormal);
#if GRAPHICS_API == D3D11 || GRAPHICS_API == D3D12
    TBN = transpose(TBN);
#endif
    NormalInfo.ShadingNormal = normalize(mul(TBN, NormalInfo.TextureNormal));
    NormalInfo.Tangent = Tangent;
    NormalInfo.Bitangent = Bitangent;
    return NormalInfo;
}

materialInfo GetMetallicRoughnessInfo(materialInfo info, vec2 FragUV)
{
    info.Metallic = Material.MetallicFactor;
    info.PerceptualRoughness = Material.RoughnessFactor;

    vec4 MetallicRoughnessSample = SampleTexture(MetallicRoughnessTexture, DefaultSampler, FragUV);
    MetallicRoughnessSample = mix(vec4(1,1,1,1), MetallicRoughnessSample, Material.UseMetallicRoughnessTexture);
    info.PerceptualRoughness *= MetallicRoughnessSample.g;
    info.Metallic *= MetallicRoughnessSample.b;

    // Achromatic f0 based on IOR.
    info.CDiff = mix(info.BaseColor.rgb,  vec3(0,0,0), info.Metallic);
    info.f0 = mix(info.f0, info.BaseColor.rgb, info.Metallic);


    info.PerceptualRoughness = Clamp01(info.PerceptualRoughness);
    info.Metallic = Clamp01(info.Metallic);

    info.AlphaRoughness = info.PerceptualRoughness * info.PerceptualRoughness;


    return info;
}
#else
materialInfo GetMetallicRoughnessInfo(materialInfo info, float Roughness, float Metallic)
{
    info.Metallic = Metallic;
    info.PerceptualRoughness = Roughness;
    // Achromatic f0 based on IOR.
    info.CDiff = mix(info.BaseColor.rgb,  vec3(0), info.Metallic);
    info.f0 = mix(info.f0, info.BaseColor.rgb, info.Metallic);


    info.PerceptualRoughness = Clamp01(info.PerceptualRoughness);
    info.Metallic = Clamp01(info.Metallic);

    info.AlphaRoughness = info.PerceptualRoughness * info.PerceptualRoughness;


    return info;
}

#endif