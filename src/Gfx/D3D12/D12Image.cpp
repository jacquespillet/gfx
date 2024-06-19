#include "D12Image.h"
#include "../Include/Context.h"
#include "D12Context.h"
#include "D12Mapping.h"
#include "D12CommandBuffer.h"

#include <d3dx12.h>
#include <d3dcompiler.h>
#include <wrl.h>

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include <stb_image_resize2.h>

using namespace Microsoft::WRL;

#include <algorithm>

namespace gfx
{

void image::Init(u32 Width, u32 Height, format Format, imageUsage::value ImageUsage, memoryUsage MemoryUsage, u32 SampleCount)
{
    std::shared_ptr<d3d12Data> D12Data = std::static_pointer_cast<d3d12Data>(context::Get()->ApiContextData);

    Extent.Width = Width;
    Extent.Height = Height;
    Format = Format;
    MipLevelCount = 1;
    this->ChannelCount = ChannelCountFromFormat(Format);
    this->Type = type::BYTE;

    this->Data.resize(Width * Height * FormatSize(Format));

    context *D12Context = context::Get();

    ApiData = std::make_shared<d3d12ImageData>();
    std::shared_ptr<d3d12ImageData> D12Image = std::static_pointer_cast<d3d12ImageData>(ApiData);
        
    D12Image->ResourceState = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

    CD3DX12_RESOURCE_DESC TextureDesc = CD3DX12_RESOURCE_DESC::Tex2D(FormatToNative(Format), Width, Height, 1, MipLevelCount);
    D12Data->Device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
        &TextureDesc, D12Image->ResourceState, nullptr,
        IID_PPV_ARGS(&D12Image->Handle));

    D12Image->OffsetInHeap = D12Data->CurrentHeapOffset;

    // Create the shader resource view (SRV)
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = FormatToNative(Format);
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = MipLevelCount;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    D12Data->Device->CreateShaderResourceView(D12Image->Handle.Get(), &srvDesc, D12Data->GetCPUDescriptorAt(D12Image->OffsetInHeap));    
    
    // Allocate descriptors by incrementing the handles
    D12Data->CurrentHeapOffset++;
}

void ResizeImageData(imageData &ImageData, s32 TargetWidth, s32 TargetHeight)
{
    if(ImageData.Type == type::UNSIGNED_BYTE)
    {
        u8 *ResizedData = (u8*) gfx::AllocateMemory(TargetWidth * TargetHeight * 4);
        stbir_resize_uint8_linear(ImageData.Data , ImageData.Width , ImageData.Height, 0,
                                ResizedData, TargetWidth, TargetHeight, 0, (stbir_pixel_layout)4);
        gfx::DeallocateMemory(ImageData.Data);
        ImageData.Data = ResizedData;
        ImageData.Width = TargetWidth;
        ImageData.Height = TargetHeight;
        ImageData.DataSize = TargetWidth * TargetHeight * 4;
    }
    else assert(false); //TODO
}

void image::Init(imageData &ImageData, const imageCreateInfo &CreateInfo)
{
    std::shared_ptr<d3d12Data> D12Data = std::static_pointer_cast<d3d12Data>(context::Get()->ApiContextData);
    
    if(ImageData.DataSize / ImageData.Height < 256)
    {
        ResizeImageData(ImageData, 256, 256);
    }

    Extent.Width = ImageData.Width;
    Extent.Height = ImageData.Height;
    Format = ImageData.Format;
    ByteSize = ImageData.DataSize;
    MipLevelCount = CreateInfo.GenerateMipmaps ? static_cast<u32>(std::floor(std::log2((std::max)(this->Extent.Width, this->Extent.Height)))) + 1 : 1;
    this->Data.resize(ImageData.DataSize);
    memcpy(this->Data.data(), ImageData.Data, ImageData.DataSize);
    this->ChannelCount = ImageData.ChannelCount;
    this->Type = ImageData.Type;
    this->CreateInfo = CreateInfo;

    context *D12Context = context::Get();

    ApiData = std::make_shared<d3d12ImageData>();
    std::shared_ptr<d3d12ImageData> D12Image = std::static_pointer_cast<d3d12ImageData>(ApiData);
    
    D12Image->ResourceState = D3D12_RESOURCE_STATE_COPY_DEST;

    CD3DX12_RESOURCE_DESC TextureDesc = CD3DX12_RESOURCE_DESC::Tex2D(FormatToNative(ImageData.Format), ImageData.Width, ImageData.Height, 1, MipLevelCount);
    TextureDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
    D12Data->Device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
        &TextureDesc, D12Image->ResourceState, nullptr,
        IID_PPV_ARGS(&D12Image->Handle));

    D12Image->OffsetInHeap = D12Data->CurrentHeapOffset;

    // Create the shader resource view (SRV)
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = FormatToNative(ImageData.Format);
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = MipLevelCount;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    D12Data->Device->CreateShaderResourceView(D12Image->Handle.Get(), &srvDesc, D12Data->GetCPUDescriptorAt(D12Image->OffsetInHeap));    
    
    // Allocate descriptors by incrementing the handles
    D12Data->CurrentHeapOffset++;

    //Fill the stage buffer with data
    auto TextureAllocation = D12Data->StageBuffer.Submit(ImageData.Data, (u32)ImageData.DataSize);
    

    
    //Copy stage buffer into texture
    D12Data->ImmediateCommandBuffer->Begin();
    D12Data->ImmediateCommandBuffer->CopyBufferToImage(
        bufferInfo {D12Data->StageBuffer.GetBuffer(), TextureAllocation.Offset },
        imageInfo {this, imageUsage::UNKNOWN, 0, 0}
    );


    if (!CreateInfo.GenerateMipmaps || MipLevelCount == 1)
    {
        D12Data->ImmediateCommandBuffer->TransferLayout(*this, imageUsage::TRANSFER_DESTINATION, imageUsage::SHADER_READ);

        D12Data->StageBuffer.Flush();
        D12Data->ImmediateCommandBuffer->End();
        context::Get()->SubmitCommandBufferImmediate(D12Data->ImmediateCommandBuffer.get());
        D12Data->StageBuffer.Reset();
    }
    else
    {   
        const D3D_SHADER_MACRO Defines[] =
        {
            nullptr,
            nullptr
        };
        // Compile the compute shader
        ComPtr<ID3DBlob> ShaderBlob = {nullptr};
        ComPtr<ID3DBlob> ErrorBlob = {nullptr};
        HRESULT hr = D3DCompileFromFile(L"resources/Shaders/GenerateMipmaps.hlsl", Defines, nullptr, "GenerateMipMaps", "cs_5_0", 0, 0, &ShaderBlob, &ErrorBlob);
        if (FAILED(hr))
        {
            if (ErrorBlob)
            {
                OutputDebugStringA(static_cast<const char*>(ErrorBlob->GetBufferPointer()));
                // ErrorBlob->Release();
                assert(false);
            }
        }

        // Create the compute shader
        D3D12_SHADER_BYTECODE ShaderBytecode = {};
        ShaderBytecode.pShaderBytecode = ShaderBlob->GetBufferPointer();
        ShaderBytecode.BytecodeLength = ShaderBlob->GetBufferSize();

    
        //Union used for shader constants
        struct DWParam
        {
            DWParam(FLOAT f) : Float(f) {}
            DWParam(UINT u) : Uint(u) {}

            void operator= (FLOAT f) { Float = f; }
            void operator= (UINT u) { Uint = u; }

            union
            {
                FLOAT Float;
                UINT Uint;
            };
        };

        //Calculate heap size
        u32 RequiredHeapSize = 0;
        RequiredHeapSize += MipLevelCount - 1;

        //The compute shader expects 2 floats, the source texture and the destination texture
        CD3DX12_DESCRIPTOR_RANGE SrvCbvRanges[2];
        CD3DX12_ROOT_PARAMETER RootParameters[3];
        SrvCbvRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0);
        SrvCbvRanges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0, 0);
        RootParameters[0].InitAsConstants(2, 0);
        RootParameters[1].InitAsDescriptorTable(1, &SrvCbvRanges[0]);
        RootParameters[2].InitAsDescriptorTable(1, &SrvCbvRanges[1]);

        //Static sampler used to get the linearly interpolated color for the mipmaps
        D3D12_STATIC_SAMPLER_DESC SamplerDesc = {};
        SamplerDesc.Filter = D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;
        SamplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        SamplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        SamplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        SamplerDesc.MipLODBias = 0.0f;
        SamplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
        SamplerDesc.MinLOD = 0.0f;
        SamplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
        SamplerDesc.MaxAnisotropy = 0;
        SamplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
        SamplerDesc.ShaderRegister = 0;
        SamplerDesc.RegisterSpace = 0;
        SamplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

        //Create the root signature for the mipmap compute shader from the parameters and sampler above
        ID3DBlob *Signature;
        CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
        rootSignatureDesc.Init(_countof(RootParameters), RootParameters, 1, &SamplerDesc, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
        D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &Signature, &ErrorBlob);
        ID3D12RootSignature *MipMapRootSignature;
        D12Data->Device->CreateRootSignature(0, Signature->GetBufferPointer(), Signature->GetBufferSize(), IID_PPV_ARGS(&MipMapRootSignature));

        //Create the descriptor heap with layout: source texture - destination texture
        D3D12_DESCRIPTOR_HEAP_DESC HeapDesc = {};
        HeapDesc.NumDescriptors = 2*RequiredHeapSize;
        HeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        HeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        ID3D12DescriptorHeap *DescriptorHeap;
        D12Data->Device->CreateDescriptorHeap(&HeapDesc, IID_PPV_ARGS(&DescriptorHeap));
        UINT DescriptorSize = D12Data->Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);



        //Create pipeline state object for the compute shader using the root signature.
        D3D12_COMPUTE_PIPELINE_STATE_DESC PSODesc = {};
        PSODesc.pRootSignature = MipMapRootSignature;
        PSODesc.CS = ShaderBytecode;
        ID3D12PipelineState *PSOMipMaps;
        D12Data->Device->CreateComputePipelineState(&PSODesc, IID_PPV_ARGS(&PSOMipMaps));


        //Prepare the shader resource view description for the source texture
        D3D12_SHADER_RESOURCE_VIEW_DESC SrcTextureSRVDesc = {};
        SrcTextureSRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        SrcTextureSRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;

        //Prepare the unordered access view description for the destination texture
        D3D12_UNORDERED_ACCESS_VIEW_DESC destTextureUAVDesc = {};
        destTextureUAVDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;

        //Get a new empty command list in recording state
        std::shared_ptr<d3d12CommandBufferData> D12CommandBuffer = std::static_pointer_cast<d3d12CommandBufferData>(D12Data->ImmediateCommandBuffer->ApiData);

        ID3D12GraphicsCommandList *CommandList = D12CommandBuffer->CommandList.Get();

        //Set root signature, pso and descriptor heap
        CommandList->SetComputeRootSignature(MipMapRootSignature);
        CommandList->SetPipelineState(PSOMipMaps);
        CommandList->SetDescriptorHeaps(1, &DescriptorHeap);

        //CPU handle for the first descriptor on the descriptor heap, used to fill the heap
        CD3DX12_CPU_DESCRIPTOR_HANDLE CurrentCPUHandle(DescriptorHeap->GetCPUDescriptorHandleForHeapStart(), 0, DescriptorSize);

        //GPU handle for the first descriptor on the descriptor heap, used to initialize the descriptor tables
        CD3DX12_GPU_DESCRIPTOR_HANDLE CurrentGPUHandle(DescriptorHeap->GetGPUDescriptorHandleForHeapStart(), 0, DescriptorSize);

        //Transition from pixel shader resource to unordered access
        CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(D12Image->Handle.Get(), D12Image->ResourceState, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));

        //Loop through the mipmaps copying from the bigger mipmap to the smaller one with downsampling in a compute shader
        for(uint32_t TopMip = 0; TopMip < MipLevelCount-1; TopMip++)
        {
            //Get mipmap dimensions
            uint32_t DstWidth = std::max(ImageData.Width >> (TopMip+1), 1U);
            uint32_t DstHeight = std::max(ImageData.Height >> (TopMip+1), 1U);

            //Create shader resource view for the source texture in the descriptor heap
            SrcTextureSRVDesc.Format = FormatToNative(ImageData.Format);
            SrcTextureSRVDesc.Texture2D.MipLevels = 1;
            SrcTextureSRVDesc.Texture2D.MostDetailedMip = TopMip;
            D12Data->Device->CreateShaderResourceView(D12Image->Handle.Get(), &SrcTextureSRVDesc, CurrentCPUHandle);
            CurrentCPUHandle.Offset(1, DescriptorSize);

            //Create unordered access view for the destination texture in the descriptor heap
            destTextureUAVDesc.Format = FormatToNative(ImageData.Format);
            destTextureUAVDesc.Texture2D.MipSlice = TopMip+1;
            D12Data->Device->CreateUnorderedAccessView(D12Image->Handle.Get(), nullptr, &destTextureUAVDesc, CurrentCPUHandle);
            CurrentCPUHandle.Offset(1, DescriptorSize);

            //Pass the destination texture pixel size to the shader as constants
            CommandList->SetComputeRoot32BitConstant(0, DWParam(1.0f/DstWidth).Uint, 0);
            CommandList->SetComputeRoot32BitConstant(0, DWParam(1.0f/DstHeight).Uint, 1);
            
            //Pass the source and destination texture views to the shader via descriptor tables
            CommandList->SetComputeRootDescriptorTable(1, CurrentGPUHandle);
            CurrentGPUHandle.Offset(1, DescriptorSize);
            CommandList->SetComputeRootDescriptorTable(2, CurrentGPUHandle);
            CurrentGPUHandle.Offset(1, DescriptorSize);

            //Dispatch the compute shader with one thread per 8x8 pixels
            CommandList->Dispatch(std::max(DstWidth / 8, 1u), std::max(DstHeight / 8, 1u), 1);

            //Wait for all accesses to the destination texture UAV to be finished before generating the next mipmap, as it will be the source texture for the next mipmap
            CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::UAV(D12Image->Handle.Get()));
        }

        //When done with the texture, transition it's state back to be a pixel shader resource
        CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(D12Image->Handle.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));


        //Close and submit the command list
        D12Data->ImmediateCommandBuffer->End();
        context::Get()->SubmitCommandBufferImmediate(D12Data->ImmediateCommandBuffer.get());    
        
        D12Data->StageBuffer.Flush();
        D12Data->StageBuffer.Reset();
    }
}


void image::InitAsCubemap(const imageData &Left, const imageData &Right, const imageData &Top, const imageData &Bottom, const imageData &Back, const imageData &Front, const imageCreateInfo &CreateInfo)
{
    std::shared_ptr<d3d12Data> D12Data = std::static_pointer_cast<d3d12Data>(context::Get()->ApiContextData);

    Extent.Width = Left.Width;
    Extent.Height = Left.Height;
    Format = Left.Format;
    ByteSize = Left.DataSize;
    MipLevelCount = CreateInfo.GenerateMipmaps ? static_cast<u32>(std::floor(std::log2((std::max)(this->Extent.Width, this->Extent.Height)))) + 1 : 1;
    this->Data.resize(Left.DataSize + Right.DataSize + Top.DataSize + Bottom.DataSize + Back.DataSize + Front.DataSize);
    sz Offset=0;
    memcpy(this->Data.data() + Offset, Left.Data, Left.DataSize);
    Offset += Left.DataSize;
    memcpy(this->Data.data() + Offset, Right.Data, Right.DataSize);
    Offset += Right.DataSize;
    memcpy(this->Data.data() + Offset, Top.Data, Top.DataSize);
    Offset += Top.DataSize;
    memcpy(this->Data.data() + Offset, Bottom.Data, Bottom.DataSize);
    Offset += Bottom.DataSize;
    memcpy(this->Data.data() + Offset, Back.Data, Back.DataSize);
    Offset += Back.DataSize;
    memcpy(this->Data.data() + Offset, Front.Data, Front.DataSize);
    Offset += Front.DataSize;
    this->ChannelCount = Left.ChannelCount;
    this->Type = Left.Type;

    context *D12Context = context::Get();

    ApiData = std::make_shared<d3d12ImageData>();
    std::shared_ptr<d3d12ImageData> D12Image = std::static_pointer_cast<d3d12ImageData>(ApiData);
        
    D12Image->ResourceState = D3D12_RESOURCE_STATE_COPY_DEST;

    CD3DX12_RESOURCE_DESC TextureDesc = CD3DX12_RESOURCE_DESC::Tex2D(FormatToNative(Format), Extent.Width, Extent.Height, 6, MipLevelCount);
    D12Data->Device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
        &TextureDesc, D12Image->ResourceState, nullptr,
        IID_PPV_ARGS(&D12Image->Handle));

    D12Image->OffsetInHeap = D12Data->CurrentHeapOffset;

    // Create the shader resource view (SRV)
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = FormatToNative(Format);
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
    srvDesc.TextureCube.MipLevels = MipLevelCount;
    srvDesc.TextureCube.MostDetailedMip = 0;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    D12Data->Device->CreateShaderResourceView(D12Image->Handle.Get(), &srvDesc, D12Data->GetCPUDescriptorAt(D12Image->OffsetInHeap));    
    // Allocate descriptors by incrementing the handles
    D12Data->CurrentHeapOffset++;

    const std::vector<std::reference_wrapper<const imageData>> Images = {Right, Left, Top, Bottom, Front, Back};

    //Fill the stage buffer with data
    for(u32 i=0; i<6; i++)
    {
        auto TextureAllocation = D12Data->StageBuffer.Submit(Images[i].get().Data, (u32)Images[i].get().DataSize);
        
        //Copy stage buffer into texture
        D12Data->ImmediateCommandBuffer->Begin();
        D12Data->ImmediateCommandBuffer->CopyBufferToImage(
            bufferInfo {D12Data->StageBuffer.GetBuffer(), TextureAllocation.Offset },
            imageInfo {this, imageUsage::UNKNOWN, 0, i, 1}
        );

        D12Data->StageBuffer.Flush();
        D12Data->ImmediateCommandBuffer->End();
        context::Get()->SubmitCommandBufferImmediate(D12Data->ImmediateCommandBuffer.get());
        D12Data->StageBuffer.Reset();
    }

    if (CreateInfo.GenerateMipmaps)
    {
        //TODO        
    }
}

void image::InitAsArray(u32 Width, u32 Height, u32 Depth, format Format, imageUsage::value ImageUsage, memoryUsage MemoryUsage, u32 SampleCount)
{
    std::shared_ptr<d3d12Data> D12Data = std::static_pointer_cast<d3d12Data>(context::Get()->ApiContextData);

    this->Extent.Width = Width;
    this->Extent.Height = Height;
    this->LayerCount = Depth;
    this->Format = Format;
    this->MipLevelCount = 1;
    this->ChannelCount = ChannelCountFromFormat(Format);
    this->Type = type::BYTE;

    this->Data.resize(Width * Height * Depth * FormatSize(Format));

    context *D12Context = context::Get();

    ApiData = std::make_shared<d3d12ImageData>();
    std::shared_ptr<d3d12ImageData> D12Image = std::static_pointer_cast<d3d12ImageData>(ApiData);
        
    D12Image->ResourceState = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

    CD3DX12_RESOURCE_DESC TextureDesc = CD3DX12_RESOURCE_DESC::Tex2D(FormatToNative(Format), Width, Height, Depth, MipLevelCount);
    D12Data->Device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
        &TextureDesc, D12Image->ResourceState, nullptr,
        IID_PPV_ARGS(&D12Image->Handle));

    D12Image->OffsetInHeap = D12Data->CurrentHeapOffset;


    // Create the shader resource view (SRV)
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = FormatToNative(Format);
    //If depth buffer, use R instead of D
    if(Format == format::D16_UNORM) srvDesc.Format = DXGI_FORMAT_R16_UNORM;
    if(Format == format::D32_SFLOAT) srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
    srvDesc.Texture2DArray.MipLevels = MipLevelCount;
    srvDesc.Texture2DArray.MostDetailedMip = 0;
    srvDesc.Texture2DArray.ArraySize = Depth;
    srvDesc.Texture2DArray.FirstArraySlice = 0;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    D12Data->Device->CreateShaderResourceView(D12Image->Handle.Get(), &srvDesc, D12Data->GetCPUDescriptorAt(D12Image->OffsetInHeap));    
    
    // Allocate descriptors by incrementing the handles
    D12Data->CurrentHeapOffset++;
}

ImTextureID image::GetImGuiID()
{
    GET_CONTEXT(D12Context, context::Get());
    GET_API_DATA(D12Image, d3d12ImageData, this);
    return (ImTextureID) D12Context->GetGPUDescriptorAt(D12Image->OffsetInHeap).ptr;
}

}