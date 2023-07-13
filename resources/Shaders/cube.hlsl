#pragma pack_matrix( column_major )

struct PSInput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};

cbuffer MainConstantBuffer1 : register(b0)
{
    float4 Color0;
    float4 Color1;
};

cbuffer MainConstantBuffer2 : register(b1)
{
    float4 Color2;
    float4 Color3;
};

cbuffer MainConstantBuffer3 : register(b2)
{
    float4 Color4;
    float4 Color5;
};

cbuffer MainConstantBuffer4 : register(b5)
{
    float4x4 ViewMatrix;
    float4x4 ProjectionMatrix;
};

Texture2D color : register(t4);
SamplerState defaultSampler : register(s0);


PSInput VSMain(float4 position : POSITION0, float2 uv : TEXCOORD1)
{
    PSInput result;
    float4x4 VP = mul(ProjectionMatrix, ViewMatrix);
    result.position = mul(VP, position);
    
    result.uv = uv;

    return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    return color.SampleLevel(defaultSampler, input.uv, 1);
}
