//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

struct PSInput
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
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

cbuffer MainConstantBuffer4 : register(b3)
{
    float4 Color6;
    float4 Color7;
};

PSInput VSMain(float4 position : POSITION0, float4 color : POSITION1)
{
    PSInput result;

    result.position = position;
    result.color = color;

    return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    return input.color + Color0;
}
