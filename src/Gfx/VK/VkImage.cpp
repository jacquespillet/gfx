#if GFX_API == GFX_VK
#include "VkImage.h"
#include "../Include/Context.h"
#include "../Include/Memory.h"
#include "../Include/ImguiHelper.h"
#include "VkContext.h"
#include "VkCommon.h"
#include "VkMemoryAllocation.h"
#include "VkCommandBuffer.h"

#include <algorithm>

#include "imgui_impl_vulkan.h"
namespace gfx
{


void GenerateImageMipmaps(vk::Image Image, u32 Width, u32 Height, u32 MipLevels, vk::CommandBuffer &CommandBuffer, int LayerIndex = 0)
{
    // VkImageMemoryBarrier Barrier {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
    vk::ImageMemoryBarrier Barrier;
    Barrier.image = Image;
    Barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    Barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    Barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    Barrier.subresourceRange.baseArrayLayer=LayerIndex;
    Barrier.subresourceRange.layerCount=1;
    Barrier.subresourceRange.levelCount=1;

    int32_t MipWidth = Width;
    int32_t MipHeight = Height;
    for(u32 i=1; i<MipLevels; i++)
    {
        Barrier.subresourceRange.baseMipLevel=i-1;
        Barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
        Barrier.newLayout = vk::ImageLayout::eTransferSrcOptimal;
        Barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
        Barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;

        CommandBuffer.pipelineBarrier(
            vk::PipelineStageFlagBits::eTransfer, 
            vk::PipelineStageFlagBits::eTransfer, (vk::DependencyFlagBits)0,
            0, nullptr, 
            0, nullptr, 
            1, &Barrier);
        
        vk::ImageBlit Blit {};
        Blit.srcOffsets[0] = vk::Offset3D(0,0,0);
        Blit.srcOffsets[1] = vk::Offset3D(MipWidth, MipHeight, 1);
        Blit.srcSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
        Blit.srcSubresource.mipLevel = i-1;
        Blit.srcSubresource.baseArrayLayer = LayerIndex;
        Blit.srcSubresource.layerCount=1;
        Blit.dstOffsets[0] = vk::Offset3D(0,0,0);
        Blit.dstOffsets[1] = vk::Offset3D(MipWidth > 1 ? MipWidth / 2 : 1, MipHeight > 1 ? MipHeight/2 : 1, 1);
        Blit.dstSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
        Blit.dstSubresource.mipLevel = i;
        Blit.dstSubresource.baseArrayLayer = LayerIndex;
        Blit.dstSubresource.layerCount=1;

        CommandBuffer.blitImage(Image, vk::ImageLayout::eTransferSrcOptimal,
                                Image, vk::ImageLayout::eTransferDstOptimal,
                                1, &Blit, vk::Filter::eLinear);
        
        Barrier.oldLayout = vk::ImageLayout::eTransferSrcOptimal;
        Barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
        Barrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
        Barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

        CommandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, (vk::DependencyFlagBits)0,
                                     0, nullptr,
                                     0, nullptr,
                                     1, &Barrier);
        if (MipWidth > 1) MipWidth /= 2;
        if (MipHeight > 1) MipHeight /= 2;                     
    }

    Barrier.subresourceRange.baseMipLevel = MipLevels - 1;
    Barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
    Barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
    Barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
    Barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
    
    CommandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, (vk::DependencyFlagBits)0,
                                    0, nullptr,
                                    0, nullptr,
                                    1, &Barrier);
}

void vkImageData::Init(const image &Image, imageUsage::value ImageUsage, memoryUsage MemoryUsage, vk::ImageCreateFlags Flags)
{
    vk::ImageCreateInfo ImageCreateInfo;
    ImageCreateInfo.setImageType(vk::ImageType::e2D)
                    .setFormat(FormatToNative(Image.Format))
                    .setExtent(vk::Extent3D(Image.Extent.Width, Image.Extent.Height, 1))
                    .setSamples(vk::SampleCountFlagBits::e1)
                    .setMipLevels(Image.MipLevelCount)
                    .setArrayLayers(Image.LayerCount)
                    .setTiling(vk::ImageTiling::eOptimal)
                    .setUsage((vk::ImageUsageFlags)ImageUsage)
                    .setSharingMode(vk::SharingMode::eExclusive)
                    .setFlags(Flags)
                    .setInitialLayout(vk::ImageLayout::eUndefined);


    Allocation = gfx::AllocateImage(ImageCreateInfo, MemoryUsage, &Handle);
    this->CurrentLayout = imageLayout::Undefined;
}

void image::Init(imageData &ImageData, const imageCreateInfo &CreateInfo)
{
    Extent.Width = ImageData.Width;
    Extent.Height = ImageData.Height;
    Format = ImageData.Format;
    Data.resize(ImageData.Width * ImageData.Height * FormatSize(Format));
    memcpy(Data.data(), ImageData.Data, Data.size());
    ChannelCount = ChannelCountFromFormat(Format);
    this->Type = ImageData.Type;
    this->CreateInfo = CreateInfo;


    MipLevelCount = CreateInfo.GenerateMipmaps ? static_cast<u32>(std::floor(std::log2((std::max)(this->Extent.Width, this->Extent.Height)))) + 1 : 1;

    context *VulkanContext = context::Get();
    ApiData = std::make_shared<vkImageData>();
    GET_API_DATA(VKImage, vkImageData, this);

    imageUsage::value ImageUsage = imageUsage::TRANSFER_DESTINATION | imageUsage::SHADER_READ; 
    if(CreateInfo.GenerateMipmaps) ImageUsage |= imageUsage::TRANSFER_SOURCE;
    VKImage->Init(
        *this,
        ImageUsage,
        memoryUsage::GpuOnly
    );
    VKImage->InitViews(*this, VKImage->Handle, Format);

    // CommandBuffer.Begin();
    GET_CONTEXT(VkData, context::Get());
    auto TextureAllocation = VkData->StageBuffer.Submit(ImageData.Data, (u32)ImageData.DataSize);
    
    VkData->ImmediateCommandBuffer->Begin();

    
    VkData->ImmediateCommandBuffer->CopyBufferToImage(
        bufferInfo {VkData->StageBuffer.GetBuffer(), TextureAllocation.Offset },
        imageInfo {this, imageUsage::UNKNOWN, 0, 0}
    );

    if(!CreateInfo.GenerateMipmaps)
        VkData->ImmediateCommandBuffer->TransferLayout(*this, imageUsage::TRANSFER_DESTINATION, imageUsage::SHADER_READ);
    else
    {
        GenerateImageMipmaps(VKImage->Handle, this->Extent.Width, this->Extent.Height, MipLevelCount, std::static_pointer_cast<vkCommandBufferData>(VkData->ImmediateCommandBuffer->ApiData)->Handle);
    }

    

    VkData->StageBuffer.Flush();
    VkData->ImmediateCommandBuffer->End();
    context::Get()->SubmitCommandBufferImmediate(VkData->ImmediateCommandBuffer.get());
    VkData->StageBuffer.Reset();

    VKImage->InitSampler(CreateInfo, MipLevelCount, Format);


    if(imgui::IsInitialized())
        VKImage->ImGuiDescriptorSet = ImGui_ImplVulkan_AddTexture(VKImage->Sampler, VKImage->DefaultImageViews.NativeView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}


void image::InitAsCubemap(const imageData &Left, const imageData &Right, const imageData &Top, const imageData &Bottom, const imageData &Back, const imageData &Front, const imageCreateInfo &CreateInfo)
{
    //Assume all images have same size and format
    this->Extent.Width = Left.Width;
    this->Extent.Height = Left.Height;
    this->Format = Left.Format;
    this->MipLevelCount = CreateInfo.GenerateMipmaps ? static_cast<u32>(std::floor(std::log2((std::max)(this->Extent.Width, this->Extent.Height)))) + 1 : 1;
    this->LayerCount = 6;
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


    context *VulkanContext = context::Get();
    ApiData = std::make_shared<vkImageData>();
    GET_API_DATA(VKImage, vkImageData, this);

    imageUsage::value ImageUsage = imageUsage::TRANSFER_DESTINATION | imageUsage::SHADER_READ; 
    if(CreateInfo.GenerateMipmaps) ImageUsage |= imageUsage::TRANSFER_SOURCE;
    VKImage->Init(
        *this,
        ImageUsage,
        memoryUsage::GpuOnly,
        vk::ImageCreateFlagBits::eCubeCompatible
    );
    VKImage->InitViews(*this, VKImage->Handle, this->Format, vk::ImageViewType::eCube);
    VKImage->InitSampler(CreateInfo, this->MipLevelCount, Format);
    
    GET_CONTEXT(VkData, context::Get());
    
    const std::vector<std::reference_wrapper<const imageData>> Images = {Right, Left, Top, Bottom, Front, Back};

    for (sz i = 0; i < 6; i++)
    {
        auto TextureAllocation = VkData->StageBuffer.Submit(Images[i].get().Data, (u32)Images[i].get().DataSize);
        VkData->ImmediateCommandBuffer->Begin();
        
        if(i==0)
            VkData->ImmediateCommandBuffer->TransferLayout(*this, imageUsage::UNKNOWN, imageUsage::TRANSFER_DESTINATION);
        VkData->ImmediateCommandBuffer->CopyBufferToImage(
            bufferInfo {VkData->StageBuffer.GetBuffer(), 0 },
            imageInfo {this, imageUsage::TRANSFER_DESTINATION, 0, (u32)i, (u32)1}
        );
        
        
        if(i==5 && !CreateInfo.GenerateMipmaps)
            VkData->ImmediateCommandBuffer->TransferLayout(*this, imageUsage::TRANSFER_DESTINATION, imageUsage::SHADER_READ);

        VkData->ImmediateCommandBuffer->End();
        context::Get()->SubmitCommandBufferImmediate(VkData->ImmediateCommandBuffer.get());
        VkData->StageBuffer.Flush();
        VkData->StageBuffer.Reset();
    }

    if(CreateInfo.GenerateMipmaps)
    {
        VkData->ImmediateCommandBuffer->Begin();

        for(sz i=0; i<6; i++)
        {
            GenerateImageMipmaps(VKImage->Handle, this->Extent.Width, this->Extent.Height, this->MipLevelCount, std::static_pointer_cast<vkCommandBufferData>(VkData->ImmediateCommandBuffer->ApiData)->Handle, i);
        }
        VkData->ImmediateCommandBuffer->End();
        context::Get()->SubmitCommandBufferImmediate(VkData->ImmediateCommandBuffer.get());
    }
    

}

image::image(vk::Image VkImage, u32 Width, u32 Height, format Format)
{
    this->ApiData = std::make_shared<vkImageData>();

    this->Extent.Width = Width;
    this->Extent.Height = Height;
    this->Format = Format;
    this->Type = type::BYTE;

    GET_API_DATA(ImageData, vkImageData, this);

    ImageData->Allocation = {};
    ImageData->InitViews(*this, VkImage, Format);
    ImageData->CurrentLayout = imageLayout::Undefined;
}

void image::GenerateMipmaps(std::shared_ptr<gfx::commandBuffer> CommandBuffer)
{
    GET_API_DATA(VkImageData, vkImageData, this);
    GET_API_DATA(VkCommandBufferData, vkCommandBufferData, CommandBuffer);
    u32 MipLevels = static_cast<u32>(std::floor(std::log2((std::max)(this->Extent.Width, this->Extent.Height)))) + 1;
    GenerateImageMipmaps(VkImageData->Handle, this->Extent.Width, this->Extent.Height, MipLevels, VkCommandBufferData->Handle, 0);
}

void image::InitAsArray(u32 Width, u32 Height, u32 Depth, format Format, imageUsage::value ImageUsage, memoryUsage MemoryUsage, u32 SampleCount)
{
    ApiData = std::make_shared<vkImageData>();
    GET_API_DATA(VkImageData, vkImageData, this);
    
    this->MipLevelCount = 1;
    this->Format = Format;
    this->Extent.Width = Width;
    this->Extent.Height = Height;
    this->LayerCount = Depth;
    this->Data.resize(Width * Height  * Depth * FormatSize(Format));
    this->Type = type::BYTE;
    
    imageUsage::value CreateUsage = ImageUsage;
    if(ImageUsage == imageUsage::COLOR_ATTACHMENT || ImageUsage == imageUsage::DEPTH_STENCIL_ATTACHMENT) CreateUsage |= imageUsage::SHADER_READ;
    CreateUsage |= imageUsage::TRANSFER_DESTINATION;
    vk::ImageCreateInfo ImageCreateInfo;
    ImageCreateInfo.setImageType(vk::ImageType::e2D)
                    .setFormat(FormatToNative(Format))
                    .setExtent(vk::Extent3D(Width, Height, 1))
                    .setSamples(SampleCountToNative(SampleCount))
                    .setMipLevels(MipLevelCount)
                    .setArrayLayers(LayerCount)
                    .setTiling(vk::ImageTiling::eOptimal)
                    .setUsage((vk::ImageUsageFlags)CreateUsage)
                    .setSharingMode(vk::SharingMode::eExclusive)
                    .setInitialLayout(vk::ImageLayout::eUndefined);
        
    // if(Options & imageOptions::CUBEMAP)
    // {
    //     ImageCreateInfo.setFlags(vk::ImageCreateFlagBits::eCubeCompatible);
    // }                       

    VkImageData->Allocation = gfx::AllocateImage(ImageCreateInfo, MemoryUsage, &VkImageData->Handle);
    VkImageData->InitViews(*this, VkImageData->Handle,  Format, vk::ImageViewType::e2DArray);
    VkImageData->InitSamplerDefault(Format, MipLevelCount); 
    VkImageData->CurrentLayout = imageLayout::Undefined;   

    commandBuffer *CommandBuffer = gfx::context::Get()->GetImmediateCommandBuffer();
    CommandBuffer->Begin();
    CommandBuffer->TransferLayout(*this, VkImageData->CurrentLayout, ImageUsageToImageLayout((imageUsage::bits)ImageUsage));
    CommandBuffer->End();
    context::Get()->SubmitCommandBufferImmediate(CommandBuffer);
    
}

void image::Init(u32 Width, u32 Height, format Format, imageUsage::value ImageUsage, memoryUsage MemoryUsage, u32 SampleCount, b8 GenerateMips)
{
    ApiData = std::make_shared<vkImageData>();
    GET_API_DATA(VkImageData, vkImageData, this);
    
    this->Extent.Width = Width;
    this->Extent.Height = Height;

    u32 MipLevelCount = GenerateMips ? static_cast<u32>(std::floor(std::log2((std::max)(this->Extent.Width, this->Extent.Height)))) + 1 : 1;

    this->MipLevelCount = MipLevelCount;
    this->LayerCount = 1;
    this->Format = Format;
    this->Data.resize(Width * Height * FormatSize(Format));
    this->Type = type::BYTE;
    
    if(ImageUsage & imageUsage::COLOR_ATTACHMENT || ImageUsage & imageUsage::DEPTH_STENCIL_ATTACHMENT) ImageUsage |= imageUsage::SHADER_READ | imageUsage::TRANSFER_SOURCE;
    
    
    vk::ImageCreateInfo ImageCreateInfo;
    ImageCreateInfo.setImageType(vk::ImageType::e2D)
                    .setFormat(FormatToNative(Format))
                    .setExtent(vk::Extent3D(Width, Height, 1))
                    .setSamples(SampleCountToNative(SampleCount))
                    .setMipLevels(MipLevelCount)
                    .setArrayLayers(LayerCount)
                    .setTiling(vk::ImageTiling::eOptimal)
                    .setUsage((vk::ImageUsageFlags)ImageUsage)
                    .setSharingMode(vk::SharingMode::eExclusive)
                    .setInitialLayout(vk::ImageLayout::eUndefined);
        
    // if(Options & imageOptions::CUBEMAP)
    // {
    //     ImageCreateInfo.setFlags(vk::ImageCreateFlagBits::eCubeCompatible);
    // }              

    
         

    VkImageData->Allocation = gfx::AllocateImage(ImageCreateInfo, MemoryUsage, &VkImageData->Handle);
    VkImageData->InitViews(*this, VkImageData->Handle,  Format);
    VkImageData->InitSamplerDefault(Format);
    VkImageData->CurrentLayout = imageLayout::Undefined;

    if(ImageUsage & imageUsage::STORAGE)
    {
        commandBuffer *CommandBuffer = gfx::context::Get()->GetImmediateCommandBuffer();
        CommandBuffer->Begin();
        CommandBuffer->TransferLayout(*this, VkImageData->CurrentLayout, ImageUsageToImageLayout(imageUsage::STORAGE));
        CommandBuffer->End();
        context::Get()->SubmitCommandBufferImmediate(CommandBuffer);
    }
}

vk::ImageSubresourceRange GetDefaultImageSubresourceRange(const image &Image)
{
    return vk::ImageSubresourceRange(
        ImageFormatToImageAspect(Image.Format),
        0,
        Image.MipLevelCount,
        0,
        Image.LayerCount
    );    
}

vk::ImageSubresourceLayers GetDefaultImageSubresourceLayers(const image &Image, u32 MipLevel, u32 Layer, u32 LayerCount)
{
    return vk::ImageSubresourceLayers(
        ImageFormatToImageAspect(Image.Format),
        MipLevel,
        Layer,
        LayerCount
    );
}


vk::ImageMemoryBarrier GetImageMemoryBarrier(const image &Texture, imageUsage::bits OldLayout, imageUsage::bits NewLayout)
{
    GET_API_DATA(VkImageData, vkImageData, (&Texture));

    auto SubResourceRange = GetDefaultImageSubresourceRange(Texture);
    vk::ImageMemoryBarrier Barrier;
    Barrier.setSrcAccessMask(ImageUsageToAccessFlags(OldLayout))
           .setDstAccessMask(ImageUsageToAccessFlags(NewLayout))
           .setOldLayout(ImageUsageToImageLayoutNative(OldLayout))
           .setNewLayout(ImageUsageToImageLayoutNative(NewLayout))
           .setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
           .setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
           .setImage(VkImageData->Handle)
           .setSubresourceRange(SubResourceRange);

    return Barrier;
}

vk::ImageMemoryBarrier GetImageMemoryBarrier(const image &Texture, imageLayout OldLayout, imageLayout NewLayout)
{
    GET_API_DATA(VkImageData, vkImageData, (&Texture));

    auto SubResourceRange = GetDefaultImageSubresourceRange(Texture);
    vk::ImageMemoryBarrier Barrier;
    Barrier.setSrcAccessMask(ImageLayoutToAccessFlags(OldLayout))
           .setDstAccessMask(ImageLayoutToAccessFlags(NewLayout))
           .setOldLayout(ImageLayoutToNative(OldLayout))
           .setNewLayout(ImageLayoutToNative(NewLayout))
           .setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
           .setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
           .setImage(VkImageData->Handle)
           .setSubresourceRange(SubResourceRange);

    return Barrier;
}


void vkImageData::InitViews(const image &Image, const vk::Image &VkImage, format Format, vk::ImageViewType ViewType)
{
    auto Vulkan = context::Get();
    GET_CONTEXT(VkData, Vulkan);

    this->Handle = VkImage;
    
    auto SubresourceRange = GetDefaultImageSubresourceRange(Image);
    
    //Create image view
    vk::ImageViewCreateInfo ImageViewCreateInfo;
    ImageViewCreateInfo.setImage(this->Handle)
                        .setViewType(ViewType)
                        .setFormat(FormatToNative(Format))
                        .setComponents(vk::ComponentMapping(vk::ComponentSwizzle::eIdentity,
                                                            vk::ComponentSwizzle::eIdentity,
                                                            vk::ComponentSwizzle::eIdentity,
                                                            vk::ComponentSwizzle::eIdentity))
                        .setSubresourceRange(SubresourceRange);

    auto NativeSubresourceRange = GetDefaultImageSubresourceRange(Image);
    ImageViewCreateInfo.setSubresourceRange(NativeSubresourceRange);
    this->DefaultImageViews.NativeView = VkData->Device.createImageView(ImageViewCreateInfo);

    //If depth image,
    auto DepthSubresourceRange = GetDefaultImageSubresourceRange(Image);
    DepthSubresourceRange.setAspectMask(DepthSubresourceRange.aspectMask & vk::ImageAspectFlagBits::eDepth);
    if(DepthSubresourceRange.aspectMask != vk::ImageAspectFlags())
    {
        ImageViewCreateInfo.setSubresourceRange(DepthSubresourceRange);
        this->DefaultImageViews.DepthOnlyView = VkData->Device.createImageView(ImageViewCreateInfo);
        this->DefaultImageViews.DepthOnlyViewSet=true;
    }

    //If stencil image,
    auto StencilSubresourceRange = GetDefaultImageSubresourceRange(Image);
    StencilSubresourceRange.setAspectMask(StencilSubresourceRange.aspectMask & vk::ImageAspectFlagBits::eStencil);
    if(StencilSubresourceRange.aspectMask != vk::ImageAspectFlags())
    {
        ImageViewCreateInfo.setSubresourceRange(StencilSubresourceRange);
        this->DefaultImageViews.StencilOnlyView = VkData->Device.createImageView(ImageViewCreateInfo);
        this->DefaultImageViews.StencilOnlyViewSet=true;
    }    
}

void vkImageData::InitSampler(const imageCreateInfo &CreateInfo, u32 MipLevelCount, format Format)
{
    vk::SamplerCreateInfo SamplerCreateInfo;
    SamplerCreateInfo.setMagFilter(SamplerFilterToNative(CreateInfo.MinFilter))
                     .setMinFilter(SamplerFilterToNative(CreateInfo.MagFilter))
                     .setMipmapMode(SamplerFilterToNativeMip(CreateInfo.MinFilter))
                     .setAddressModeU(SamplerWrapModeToNative(CreateInfo.WrapR))
                     .setAddressModeV(SamplerWrapModeToNative(CreateInfo.WrapS))
                     .setAddressModeW(SamplerWrapModeToNative(CreateInfo.WrapT))
                     .setMinLod(0)
                     .setMaxLod((f32)MipLevelCount)
                     .setBorderColor(vk::BorderColor::eFloatOpaqueWhite);
    
    if(IsDepthFormat(Format))
    {
        SamplerCreateInfo.setCompareEnable(true).setCompareOp(vk::CompareOp::eLess);
    }                     
    
    auto Vulkan = context::Get();
    GET_CONTEXT(VkData, context::Get());

    this->Sampler = VkData->Device.createSampler(SamplerCreateInfo);
}

void vkImageData::InitSamplerDefault(format Format, u32 MipLevelCount)
{

    vk::SamplerCreateInfo SamplerCreateInfo;
    SamplerCreateInfo.setMagFilter(vk::Filter::eLinear)
                     .setMinFilter(vk::Filter::eLinear)
                     .setMipmapMode(vk::SamplerMipmapMode::eNearest)
                     .setAddressModeU(vk::SamplerAddressMode::eClampToBorder)
                     .setAddressModeV(vk::SamplerAddressMode::eClampToBorder)
                     .setAddressModeW(vk::SamplerAddressMode::eClampToBorder)
                     .setMinLod(0)
                     .setMaxLod(f32(MipLevelCount))
                     .setBorderColor(vk::BorderColor::eFloatOpaqueWhite);
    
    if(IsDepthFormat(Format))
    {
        SamplerCreateInfo.setCompareEnable(true).setCompareOp(vk::CompareOp::eLess);
    }
    if(Format == format::R32G32B32A32_UINT) //TODO: Add all the formats that do not support compare
    {
        SamplerCreateInfo.setMagFilter(vk::Filter::eNearest).setMinFilter(vk::Filter::eNearest);
        // SamplerCreateInfo.setCompareEnable(true);
    }
    auto Vulkan = context::Get();
    GET_CONTEXT(VkData, context::Get());

    this->Sampler = VkData->Device.createSampler(SamplerCreateInfo);
}

void image::Destroy()
{
    GET_CONTEXT(VkData, context::Get());
    GET_API_DATA(VkImageData, vkImageData, this);

    VkData->Device.destroyImageView(VkImageData->DefaultImageViews.NativeView);
    if(VkImageData->DefaultImageViews.DepthOnlyViewSet)
        VkData->Device.destroyImageView(VkImageData->DefaultImageViews.DepthOnlyView);
    if(VkImageData->DefaultImageViews.StencilOnlyViewSet)
        VkData->Device.destroyImageView(VkImageData->DefaultImageViews.StencilOnlyView);

    if((void*)VkImageData->Sampler != nullptr)
    {
        VkData->Device.destroySampler(VkImageData->Sampler);
    }
    
    vmaDestroyImage(VkData->Allocator, VkImageData->Handle, VkImageData->Allocation);

}

ImTextureID image::GetImGuiID()
{
    GET_API_DATA(VkImageData, vkImageData, this);
    return (ImTextureID) VkImageData->ImGuiDescriptorSet;
}

}

#endif