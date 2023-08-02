#include "D12Image.h"
#include "../Include/Context.h"
#include "D12Context.h"
#include "D12Mapping.h"
#include "D12CommandBuffer.h"

#include <d3dx12.h>
#include <d3dcompiler.h>
#include <wrl.h>
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

void image::Init(const imageData &ImageData, const imageCreateInfo &CreateInfo)
{
    std::shared_ptr<d3d12Data> D12Data = std::static_pointer_cast<d3d12Data>(context::Get()->ApiContextData);

    Extent.Width = ImageData.Width;
    Extent.Height = ImageData.Height;
    Format = ImageData.Format;
    ByteSize = ImageData.DataSize;
    MipLevelCount = CreateInfo.GenerateMipmaps ? static_cast<u32>(std::floor(std::log2((std::max)(this->Extent.Width, this->Extent.Height)))) + 1 : 1;
    this->Data.resize(ImageData.DataSize);
    memcpy(this->Data.data(), ImageData.Data, ImageData.DataSize);
    this->ChannelCount = ImageData.ChannelCount;
    this->Type = ImageData.Type;

    context *D12Context = context::Get();

    ApiData = std::make_shared<d3d12ImageData>();
    std::shared_ptr<d3d12ImageData> D12Image = std::static_pointer_cast<d3d12ImageData>(ApiData);
        
    D12Image->ResourceState = D3D12_RESOURCE_STATE_COPY_DEST;

    CD3DX12_RESOURCE_DESC TextureDesc = CD3DX12_RESOURCE_DESC::Tex2D(FormatToNative(ImageData.Format), ImageData.Width, ImageData.Height, 1, MipLevelCount);
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

    if (!CreateInfo.GenerateMipmaps)
    {
        D12Data->ImmediateCommandBuffer->TransferLayout(*this, imageUsage::TRANSFER_DESTINATION, imageUsage::SHADER_READ);

        D12Data->StageBuffer.Flush();
        D12Data->ImmediateCommandBuffer->End();
        context::Get()->SubmitCommandBufferImmediate(D12Data->ImmediateCommandBuffer.get());
        D12Data->StageBuffer.Reset();
    }
    else
    {  
        // Create an intermediate texture with a full mip chain
        TextureDesc.MipLevels = MipLevelCount;
        TextureDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
        ComPtr<ID3D12Resource> IntermediateTexture;
        D12Data->Device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
                                        &TextureDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr,
                                        IID_PPV_ARGS(&IntermediateTexture));        


        u32 IntermediateOffsetInHeapStart = D12Data->CurrentHeapOffset;
        std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> IntermediateUAVs(MipLevelCount);
        for(u32 i=0; i<MipLevelCount; i++)
        {
            IntermediateUAVs[i] = D12Data->GetCPUDescriptorAt(IntermediateOffsetInHeapStart + i);
            // Describe and create the UAV for the intermediate texture
            D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
            uavDesc.Format = IntermediateTexture->GetDesc().Format; // Use the same format as the texture
            uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D; // Assuming the intermediate texture is 2D
            uavDesc.Texture2D.MipSlice = i; // Use the first mip level
            uavDesc.Texture2D.PlaneSlice = 0; // Use the first plane (for non-planar formats)
            D12Data->Device->CreateUnorderedAccessView(IntermediateTexture.Get(), nullptr, &uavDesc, IntermediateUAVs[i]);
        }
                  
        //Create Root signature for compute
        CD3DX12_ROOT_PARAMETER1 rootParameters[3];

        // Define a descriptor table parameter for the source texture
        CD3DX12_DESCRIPTOR_RANGE1 descriptorRangeSRV;
        descriptorRangeSRV.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0); // Assumes source texture is a shader resource view (SRV)
        rootParameters[0].InitAsDescriptorTable(1, &descriptorRangeSRV);
        CD3DX12_DESCRIPTOR_RANGE1 descriptorRangeUAV;
        descriptorRangeUAV.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0); // Assumes intermediate texture is a unordered access view (UAV)
        rootParameters[1].InitAsDescriptorTable(1, &descriptorRangeUAV);
        rootParameters[2].InitAsConstants(2, 0); // Assumes 2 32-bit constants starting at register 0
        
        //Static sampler used to get the linearly interpolated color for the mipmaps
        D3D12_STATIC_SAMPLER_DESC samplerDesc = {};
        samplerDesc.Filter = D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;
        samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        samplerDesc.MipLODBias = 0.0f;
        samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
        samplerDesc.MinLOD = 0.0f;
        samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
        samplerDesc.MaxAnisotropy = 0;
        samplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
        samplerDesc.ShaderRegister = 0;
        samplerDesc.RegisterSpace = 0;
        samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;        
        // Create a root signature descriptor
        CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
        rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 1, &samplerDesc, D3D12_ROOT_SIGNATURE_FLAG_NONE);

        // Serialize the root signature
        ComPtr<ID3DBlob> signatureBlob = {nullptr};
        ComPtr<ID3DBlob> errorBlob = {nullptr};
        HRESULT hr = D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_1,
                                                        &signatureBlob, &errorBlob);
        if (FAILED(hr))
        { 
            if (errorBlob)
            {
                OutputDebugStringA(static_cast<const char*>(errorBlob->GetBufferPointer()));
                // errorBlob->Release();
            }
        }
        // Create the root signature
        ID3D12RootSignature* rootSignature = nullptr;
        hr = D12Data->Device->CreateRootSignature(0, signatureBlob->GetBufferPointer(),
                                        signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
        if (FAILED(hr))
        {
            assert(false);
        }

        // Release the signature blob
        // signatureBlob->Release();        
        //Create compute shader PSO
        const D3D_SHADER_MACRO defines[] =
        {
            nullptr,
            nullptr
        };
        // Compile the compute shader
        ComPtr<ID3DBlob> shaderBlob = {nullptr};
        hr = D3DCompileFromFile(L"resources/Shaders/GenerateMipmaps.hlsl", defines, nullptr, "CSMain", "cs_5_0", 0, 0, &shaderBlob, &errorBlob);
        if (FAILED(hr))
        {
            if (errorBlob)
            {
                OutputDebugStringA(static_cast<const char*>(errorBlob->GetBufferPointer()));
                // errorBlob->Release();
                assert(false);
            }
        }

        // Create the compute shader
        D3D12_SHADER_BYTECODE shaderBytecode = {};
        shaderBytecode.pShaderBytecode = shaderBlob->GetBufferPointer();
        shaderBytecode.BytecodeLength = shaderBlob->GetBufferSize();

        // Describe the compute pipeline state
        D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc = {};
        psoDesc.pRootSignature = rootSignature; // The root signature for the compute shader
        psoDesc.CS = shaderBytecode;

        // Create the compute pipeline state object
        ComPtr<ID3D12PipelineState> computePSO = nullptr;
        hr = D12Data->Device->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&computePSO));
        if (FAILED(hr))
        {
            assert(false);
        }
        // Release the shader blob
        // shaderBlob->Release();



        std::shared_ptr<d3d12CommandBufferData> D12CommandBuffer = std::static_pointer_cast<d3d12CommandBufferData>(D12Data->ImmediateCommandBuffer->ApiData);
        
        // Transition the intermediate texture to the COPY_SOURCE state
        // D12CommandBuffer->CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(IntermediateTexture.Get(),
        //                                     D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_DEST));


        
        // Copy the texture to the highest mipmap level in the intermediary
        D12CommandBuffer->CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(D12Image->Handle.Get(),
                                            D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_COPY_SOURCE));
        D3D12_TEXTURE_COPY_LOCATION srcLocation = {};
        srcLocation.pResource = D12Image->Handle.Get();
        srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
        srcLocation.SubresourceIndex = 0; // Highest mip level

        D3D12_TEXTURE_COPY_LOCATION dstLocation = {};
        dstLocation.pResource = IntermediateTexture.Get();
        dstLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
        dstLocation.SubresourceIndex = 0; // Highest mip level
        D12CommandBuffer->CommandList->CopyTextureRegion(&dstLocation, 0, 0, 0, &srcLocation, nullptr);


        // Transition the source texture to the COPY_DEST state so we can read it in the shader
        D12CommandBuffer->CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(D12Image->Handle.Get(),
            D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
        

        D12CommandBuffer->CommandList->SetPipelineState(computePSO.Get());
        D12CommandBuffer->CommandList->SetComputeRootSignature(rootSignature);
        D12CommandBuffer->CommandList->SetComputeRootDescriptorTable(0, D12Data->GetGPUDescriptorAt(D12Image->OffsetInHeap));
        

        // Dispatch the compute shader
        struct mipSize {u32 Width; u32 Height;};
        mipSize MipSize;
        MipSize.Width = Extent.Width;
        MipSize.Height = Extent.Height;
        for (UINT mipLevel = 1; mipLevel < MipLevelCount; ++mipLevel)
        {
            //bind the mipmap level
            CD3DX12_GPU_DESCRIPTOR_HANDLE IntermediateHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(D12Data->GetGPUDescriptorAt(IntermediateOffsetInHeapStart + mipLevel));
            D12CommandBuffer->CommandList->SetComputeRootDescriptorTable(1, IntermediateHandle);
        
            // Calculate the dimensions of the current mip level
            MipSize.Width = (std::max)(1u, MipSize.Width / 2);
            MipSize.Height = (std::max)(1u, MipSize.Height / 2);

            // Set the dimensions of the current mip level
            D12CommandBuffer->CommandList->SetComputeRoot32BitConstants(2, 2, &MipSize, 0);

            // Dispatch the compute shader
            D12CommandBuffer->CommandList->Dispatch((std::max)(1u, MipSize.Width / 8), (std::max)(1u, MipSize.Height / 8), 1);
        }

        //Transition the texture to copy dest
        D12CommandBuffer->CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(D12Image->Handle.Get(),
            D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_DEST));
            
        //Transition the intermediate to copy source
        D12CommandBuffer->CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(IntermediateTexture.Get(),
                                            D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_COPY_SOURCE));

        // Copy each level of the intermediary into our texture
        for (UINT mipLevel = 1; mipLevel < MipLevelCount; ++mipLevel)
        {
            // Create a subresource footprint for the current mip level
            D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint;
            UINT numRows;
            UINT64 rowSizeInBytes;
            UINT64 totalBytes;
            D12Data->Device->GetCopyableFootprints(&IntermediateTexture->GetDesc(), mipLevel, 1, 0, &footprint, &numRows, &rowSizeInBytes, &totalBytes);

            // Create a copy operation from the intermediate texture to the source texture
            D3D12_TEXTURE_COPY_LOCATION srcLocation = {};
            srcLocation.pResource = IntermediateTexture.Get();
            srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
            srcLocation.SubresourceIndex = mipLevel;

            D3D12_TEXTURE_COPY_LOCATION dstLocation = {};
            dstLocation.pResource = D12Image->Handle.Get();
            dstLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
            dstLocation.SubresourceIndex = mipLevel;

            // Issue the copy command
            D12CommandBuffer->CommandList->CopyTextureRegion(&dstLocation, 0, 0, 0, &srcLocation, nullptr);
        }
        // Transition the source texture to the shader resource state
        D12CommandBuffer->CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(D12Image->Handle.Get(),
                                            D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
        
        

        D12Data->ImmediateCommandBuffer->End();
        context::Get()->SubmitCommandBufferImmediate(D12Data->ImmediateCommandBuffer.get());
         
        D12Data->StageBuffer.Flush();
        D12Data->StageBuffer.Reset();
    }
    
    //Create the view 
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

ImTextureID image::GetImGuiID()
{
    GET_CONTEXT(D12Context, context::Get());
    GET_API_DATA(D12Image, d3d12ImageData, this);
    return (ImTextureID) D12Context->GetGPUDescriptorAt(D12Image->OffsetInHeap).ptr;
}

}