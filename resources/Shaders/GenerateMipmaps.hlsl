Texture2D sourceTexture : register(t0); // Highest mip level texture
RWTexture2D<float4> intermediateTexture : register(u0); // Intermediate texture for downsampling
SamplerState defaultSampler : register(s0);


// Constants for the current mip level
cbuffer Constants : register(b0)
{
    uint mipWidth;
    uint mipHeight;
};

float2 GetUVFromTexelCoord(int2 TexelCoord, uint Width, uint Height)
{
    return float2(float(TexelCoord.x) / float(Width), float(TexelCoord.y) / float(Height));
}

[numthreads(8, 8, 1)]
void CSMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // Calculate the source texel coordinates for the current thread
    float2 sourceTexCoords = float2(dispatchThreadID.xy) * float2(2, 2);
    
    // Calculate the destination texel coordinates for the current thread
    float2 destTexCoords = float2(dispatchThreadID.xy);
    
    // Calculate the source and destination texel offsets for bilinear interpolation
    float2 texOffset = float2(0.5, 0.5);

    uint upperLevelWidth = mipWidth * 2;
    uint upperLevelHeight = mipHeight * 2;
    
    // Perform bilinear interpolation to downsample the texture
    float4 texel0 = sourceTexture.SampleLevel(defaultSampler, GetUVFromTexelCoord(int2(sourceTexCoords + texOffset), upperLevelWidth, upperLevelHeight), 0);
    float4 texel1 = sourceTexture.SampleLevel(defaultSampler, GetUVFromTexelCoord(int2(sourceTexCoords.x + 1 + texOffset.x, sourceTexCoords.y + texOffset.y), upperLevelWidth, upperLevelHeight), 0);
    float4 texel2 = sourceTexture.SampleLevel(defaultSampler, GetUVFromTexelCoord(int2(sourceTexCoords.x + texOffset.x, sourceTexCoords.y + 1 + texOffset.y), upperLevelWidth, upperLevelHeight), 0);
    float4 texel3 = sourceTexture.SampleLevel(defaultSampler, GetUVFromTexelCoord(int2(sourceTexCoords + 1 + texOffset), upperLevelWidth, upperLevelHeight), 0);
    float4 destTexel = (texel0 + texel1 + texel2 + texel3) * 0.25;
    
    // Write the downscaled texel to the intermediate texture
    intermediateTexture[int2(destTexCoords)] = destTexel;
}