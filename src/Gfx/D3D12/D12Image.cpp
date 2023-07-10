#include "D12Image.h"
#include "../Include/GfxContext.h"
#include "D12GfxContext.h"
#include "D12Mapping.h"
#include "D12CommandBuffer.h"

#include <d3dx12.h>

namespace gfx
{

image::image(imageData *ImageData, textureCreateInfo &CreateInfo)
{
    std::shared_ptr<d3d12Data> D12Data = std::static_pointer_cast<d3d12Data>(context::Get()->ApiContextData);

    Extent.Width = ImageData->Width;
    Extent.Height = ImageData->Height;
    Format = ImageData->Format;
    ByteSize = ImageData->DataSize;

    context *VulkanContext = context::Get();

    ApiData = std::make_shared<d3d12ImageData>();
    std::shared_ptr<d3d12ImageData> D12Image = std::static_pointer_cast<d3d12ImageData>(ApiData);
    
    uint32_t MipLevels = 1;
    
    D12Image->ResourceState = D3D12_RESOURCE_STATE_COPY_DEST;

    
    D12Data->Device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Tex2D(FormatToNative(ImageData->Format), ImageData->Width, ImageData->Height, 1, MipLevels), D12Image->ResourceState, nullptr,
        IID_PPV_ARGS(&D12Image->Handle));

    //Fill the stage buffer with data
    auto TextureAllocation = D12Data->StageBuffer.Submit(ImageData->Data, (u32)ImageData->DataSize);
    

    
    //Copy stage buffer into texture
    D12Data->ImmediateCommandBuffer->Begin();
    D12Data->ImmediateCommandBuffer->CopyBufferToImage(
        bufferInfo {D12Data->StageBuffer.GetBuffer(), TextureAllocation.Offset },
        imageInfo {this, imageUsage::UNKNOWN, 0, 0}
    );
    D12Data->ImmediateCommandBuffer->TransferLayout(*this, imageUsage::TRANSFER_DESTINATION, imageUsage::SHADER_READ);
    D12Data->StageBuffer.Flush();
    D12Data->ImmediateCommandBuffer->End();
    context::Get()->SubmitCommandBufferImmediate(D12Data->ImmediateCommandBuffer.get());
    D12Data->StageBuffer.Reset();
    
    //Create the view 

    //TODO: Do we use these handles ?
    D12Image->CPUHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(D12Data->CommonDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
    D12Image->GPUHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(D12Data->CommonDescriptorHeap->GetGPUDescriptorHandleForHeapStart());

    D12Image->OffsetInHeap = D12Data->CurrentHeapOffset;

    // Create the shader resource view (SRV)
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = FormatToNative(ImageData->Format);
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = MipLevels;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    D12Data->Device->CreateShaderResourceView(D12Image->Handle.Get(), &srvDesc, D12Data->GetCPUDescriptorAt(D12Image->OffsetInHeap));    
    
    // Allocate descriptors by incrementing the handles
    D12Image->CPUHandle.Offset(1, D12Data->Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)); // index is the index of the descriptor to allocate
    D12Image->GPUHandle.Offset(1, D12Data->Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
    D12Data->CurrentHeapOffset++;

}

}