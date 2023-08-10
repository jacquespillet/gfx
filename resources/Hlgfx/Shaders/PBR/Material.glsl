
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


vec4 GetBaseColor()
{
    vec4 BaseColor = vec4(1);

    BaseColor = vec4(Material.BaseColorFactor, Material.OpacityFactor);

    vec4 SampleCol = texture(BaseColorTexture, Input.FragUV);
    SampleCol.rgb = pow(SampleCol.rgb, vec3(2.2));
    SampleCol = mix(vec4(1,1,1,1), SampleCol, Material.UseBaseColor);
    BaseColor.rgb *= SampleCol.rgb;
    BaseColor.a *= SampleCol.a;

    return BaseColor;
}

struct normalInfo {
    vec3 GeometricNormal;  
    vec3 Tangent;   
    vec3 Bitangent;   
    vec3 ShadingNormal;   
    vec3 TextureNormal;
};

normalInfo GetNormalInfo()
{
    vec3 ShadingNormal, Tangent, Bitangent, GeometricNormal;
   
    Tangent = normalize(Input.TBN[0]);
    Bitangent = normalize(Input.TBN[1]);
    GeometricNormal = normalize(Input.TBN[2]);

    normalInfo NormalInfo;
    NormalInfo.GeometricNormal = GeometricNormal;
    NormalInfo.ShadingNormal = GeometricNormal;


    NormalInfo.TextureNormal = texture(NormalTexture, Input.FragUV).rgb * 2.0 - vec3(1.0);
    NormalInfo.TextureNormal = normalize(NormalInfo.TextureNormal);
    NormalInfo.TextureNormal = mix(vec3(0,0,1), NormalInfo.TextureNormal, Material.UseNormalTexture);
    NormalInfo.ShadingNormal = normalize(mat3(Tangent, Bitangent, GeometricNormal) * NormalInfo.TextureNormal);


    NormalInfo.Tangent = Tangent;
    NormalInfo.Bitangent = Bitangent;
    return NormalInfo;
}

materialInfo GetMetallicRoughnessInfo(materialInfo info)
{
    info.Metallic = Material.MetallicFactor;
    info.PerceptualRoughness = Material.RoughnessFactor;

    vec4 MetallicRoughnessSample = texture(MetallicRoughnessTexture, Input.FragUV);
    MetallicRoughnessSample = mix(vec4(1), MetallicRoughnessSample, Material.UseMetallicRoughnessTexture);
    info.PerceptualRoughness *= MetallicRoughnessSample.g;
    info.Metallic *= MetallicRoughnessSample.b;

    // Achromatic f0 based on IOR.
    info.CDiff = mix(info.BaseColor.rgb,  vec3(0), info.Metallic);
    info.f0 = mix(info.f0, info.BaseColor.rgb, info.Metallic);


    info.PerceptualRoughness = Clamp01(info.PerceptualRoughness);
    info.Metallic = Clamp01(info.Metallic);

    info.AlphaRoughness = info.PerceptualRoughness * info.PerceptualRoughness;


    return info;
}