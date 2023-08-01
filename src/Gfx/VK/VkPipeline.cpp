#if GFX_API==GFX_VK
#include "../Include/Pipeline.h"
#include "../Include/Context.h"
#include "../Include/Types.h"

#include "VkResourceManager.h"
#include "VkRenderPass.h"
#include "VkShader.h"
#include "VkCommon.h"
#include "VkPipeline.h"

#include <algorithm>
namespace gfx
{
    
renderPassOutput &renderPassOutput::Depth(format Format, imageLayout Layout)
{
    DepthStencilFormat = Format;
    DepthStencilFinalLayout = Layout;
    return *this;
}
renderPassOutput &renderPassOutput::Color(format Format, imageLayout Layout, renderPassOperation::values Operation)
{
    ColorFinalLayouts[NumColorFormats] = Layout;
    ColorOperations[NumColorFormats] = Operation;
    ColorFormats[NumColorFormats++] = Format;
    return *this;
}
renderPassOutput &renderPassOutput::SetDepthStencilOperation(renderPassOperation::values DepthOperation, renderPassOperation::values StencilOperation)
{
    this->DepthOperation = DepthOperation;
    this->StencilOperation = StencilOperation;
    return *this;
}
void renderPassOutput::Reset()
{
    NumColorFormats = 0;
    for (u32 i = 0; i < commonConstants::MaxImageOutputs; i++)
    {
        ColorFormats[i] = format::UNDEFINED;
        ColorOperations[i] = renderPassOperation::DontCare;
        ColorFinalLayouts[i] = imageLayout::Undefined;
    }

    DepthStencilFormat = format::UNDEFINED;
    DepthStencilFinalLayout = imageLayout::Undefined;
} 


descriptorSetLayoutHandle CreateDescriptorSetLayout2(const descriptorSetLayoutCreation &Creation)
{
    context *Context = context::Get();
    GET_CONTEXT(VkData, Context);

    std::shared_ptr<vkResourceManagerData> VkResourceManager = std::static_pointer_cast<vkResourceManagerData>(Context->ResourceManager.ApiData);

    descriptorSetLayoutHandle Handle = VkResourceManager->DescriptorSetLayouts.ObtainResource(); 
    if(Handle == InvalidHandle)
    {
        return Handle;
    }

    descriptorSetLayout *DescriptorSetLayout = (descriptorSetLayout *)VkResourceManager->DescriptorSetLayouts.GetResource(Handle);

    memset(&DescriptorSetLayout->UsedBindings[0], 0, vkConstants::MaxDescriptorsPerSet * sizeof(b8));

    u16 MaxBinding=0;
    for(u32 i=0; i<Creation.NumBindings; i++)
    {
        const descriptorSetLayoutCreation::binding &Binding = Creation.Bindings[i];
        MaxBinding = (std::max)(MaxBinding, Binding.Start);
    }    
    MaxBinding +=1;

    DescriptorSetLayout->BindingCount = (u16)Creation.NumBindings;
    u8 *Memory = (u8*)AllocateMemory((sizeof(vk::DescriptorSetLayoutBinding) + sizeof(descriptorBinding)) * Creation.NumBindings + (sizeof(u8) * MaxBinding));
    DescriptorSetLayout->Bindings = (descriptorBinding*)Memory;
    DescriptorSetLayout->BindingNativeHandle = (vk::DescriptorSetLayoutBinding*)(Memory + sizeof(descriptorBinding) * Creation.NumBindings);
    DescriptorSetLayout->IndexToBinding = (u8*)(DescriptorSetLayout->BindingNativeHandle + Creation.NumBindings); 
    DescriptorSetLayout->Handle = Handle;
    DescriptorSetLayout->SetIndex = u16(Creation.SetIndex);
    DescriptorSetLayout->Bindless = Creation.Bindless ? 1 : 0;



    u32 UsedBindings = 0;
    for(u32 R = 0; R < Creation.NumBindings; ++R)
    {
        descriptorBinding &Binding = DescriptorSetLayout->Bindings[R];
        const descriptorSetLayoutCreation::binding &InputBinding = Creation.Bindings[R];
        Binding.Start = InputBinding.Start == UINT16_MAX ? (u16)R : InputBinding.Start;
        Binding.Count = InputBinding.Count;
        Binding.Type = InputBinding.Type;
        Binding.Name = InputBinding.Name;

        DescriptorSetLayout->IndexToBinding[Binding.Start] = R;

        vk::DescriptorSetLayoutBinding &VkBinding = DescriptorSetLayout->BindingNativeHandle[UsedBindings];
        ++UsedBindings;

        VkBinding.binding = Binding.Start;
        VkBinding.descriptorType = InputBinding.Type;
        VkBinding.descriptorCount = InputBinding.Count;
        VkBinding.stageFlags = vk::ShaderStageFlagBits::eAll;
        VkBinding.pImmutableSamplers = nullptr; 

        DescriptorSetLayout->UsedBindings[Binding.Start] = true;
    }

    vk::DescriptorSetLayoutCreateInfo DescriptorSetLayoutCreateInfo;
    DescriptorSetLayoutCreateInfo.setBindingCount(UsedBindings)
                                 .setPBindings(DescriptorSetLayout->BindingNativeHandle);

    
    DescriptorSetLayout->NativeHandle = VkData->Device.createDescriptorSetLayout(DescriptorSetLayoutCreateInfo);
    

    return Handle;
}

void vkPipelineData::Create(const pipelineCreation &PipelineCreation)
{
    GET_CONTEXT(VkData, context::Get());

    shader *ShaderStateData = context::Get()->GetShader(this->ShaderState);
    GET_API_DATA(VkShaderData, vkShaderData, ShaderStateData);


    renderPass *RenderPass = gfx::context::Get()->GetRenderPass(PipelineCreation.RenderPassHandle);

    vk::DescriptorSetLayout Layouts[vkConstants::MaxDescriptorSetLayouts];

    u32 NumActiveLayouts = VkShaderData->SpirvParseResults.SetCount;
    for(sz i=0; i<NumActiveLayouts; i++)
    {
        this->DescriptorSetLayoutHandles[i] = CreateDescriptorSetLayout2(VkShaderData->SpirvParseResults.Sets[i]);
        
        std::shared_ptr<vkResourceManagerData> VkResourceManagerData = std::static_pointer_cast<vkResourceManagerData>(context::Get()->ResourceManager.ApiData);
        this->DescriptorSetLayouts[i] = (descriptorSetLayout*)VkResourceManagerData->DescriptorSetLayouts.GetResource(this->DescriptorSetLayoutHandles[i]);
        Layouts[i] = this->DescriptorSetLayouts[i]->NativeHandle;
    }

    vk::PipelineLayoutCreateInfo PipelineLayoutCreate;
    PipelineLayoutCreate.setPSetLayouts(Layouts)
                        .setSetLayoutCount(NumActiveLayouts);

    this->PipelineLayout = VkData->Device.createPipelineLayout(PipelineLayoutCreate);
    this->NumActiveLayouts = NumActiveLayouts;

    if(ShaderStateData->GraphicsPipeline)
    {
        vk::GraphicsPipelineCreateInfo PipelineCreateInfo;

        PipelineCreateInfo.setPStages(VkShaderData->ShaderStageCreateInfo)
                          .setStageCount(ShaderStateData->ActiveShaders)
                          .setLayout(this->PipelineLayout);

        vk::PipelineVertexInputStateCreateInfo VertexInputInfo;
        
        vk::VertexInputAttributeDescription VertexAttributes[8];
        if(PipelineCreation.VertexInput.NumVertexAttributes)
        {
            for(u32 i=0; i<PipelineCreation.VertexInput.NumVertexAttributes; i++)
            {
                const vertexAttribute &VertexAttribute = PipelineCreation.VertexInput.VertexAttributes[i];
                VertexAttributes[i] = vk::VertexInputAttributeDescription(VertexAttribute.Location, VertexAttribute.Binding, ToVkVertexFormat(VertexAttribute.Format), VertexAttribute.Offset);
            }
            VertexInputInfo.setVertexAttributeDescriptionCount(PipelineCreation.VertexInput.NumVertexAttributes)
                           .setPVertexAttributeDescriptions(VertexAttributes);
        }
        else VertexInputInfo.setVertexAttributeDescriptionCount(0) .setPVertexAttributeDescriptions(nullptr);
        
        vk::VertexInputBindingDescription VertexBindings[8];

        if(PipelineCreation.VertexInput.NumVertexStreams)
        {
            VertexInputInfo.setVertexBindingDescriptionCount(PipelineCreation.VertexInput.NumVertexStreams);

            for(u32 i=0; i<PipelineCreation.VertexInput.NumVertexStreams; i++)
            {
                const vertexStream &VertexStream = PipelineCreation.VertexInput.VertexStreams[i];
                vk::VertexInputRate VertexRate = VertexStream.InputRate == vertexInputRate::PerVertex ? vk::VertexInputRate::eVertex : vk::VertexInputRate::eInstance;
                VertexBindings[i] = vk::VertexInputBindingDescription(VertexStream.Binding, VertexStream.Stride, VertexRate);
            }
            VertexInputInfo.setPVertexBindingDescriptions(VertexBindings);
        }
        else VertexInputInfo.setVertexBindingDescriptionCount(0).setVertexBindingDescriptions(nullptr);
        
        PipelineCreateInfo.setPVertexInputState(&VertexInputInfo);

        vk::PipelineInputAssemblyStateCreateInfo InputAssemblyCreateInfo;
        InputAssemblyCreateInfo.setTopology(vk::PrimitiveTopology::eTriangleList)
                               .setPrimitiveRestartEnable(VK_FALSE);
        PipelineCreateInfo.setPInputAssemblyState(&InputAssemblyCreateInfo);

        vk::PipelineColorBlendAttachmentState ColorBlendAttachment[8];
        if(PipelineCreation.BlendState.ActiveStates)
        {
            for(sz i=0; i<PipelineCreation.BlendState.ActiveStates; i++)
            {
                const blendState &BlendState = PipelineCreation.BlendState.BlendStates[i];

                ColorBlendAttachment[i].colorWriteMask = vk::ColorComponentFlagBits::eR |  vk::ColorComponentFlagBits::eG |  vk::ColorComponentFlagBits::eB |  vk::ColorComponentFlagBits::eA;
                ColorBlendAttachment[i].blendEnable = BlendState.BlendEnabled ? VK_TRUE : VK_FALSE;
                ColorBlendAttachment[i].srcColorBlendFactor = BlendFactorToNative(BlendState.SourceColor);
                ColorBlendAttachment[i].dstColorBlendFactor = BlendFactorToNative(BlendState.DestinationColor);
                ColorBlendAttachment[i].colorBlendOp = BlendOpToNative(BlendState.ColorOp);

                if(BlendState.SeparateBlend)
                {
                    ColorBlendAttachment[i].srcAlphaBlendFactor = BlendFactorToNative(BlendState.SourceAlpha);
                    ColorBlendAttachment[i].dstAlphaBlendFactor = BlendFactorToNative(BlendState.DestinationAlpha);
                    ColorBlendAttachment[i].alphaBlendOp = BlendOpToNative(BlendState.AlphaOp);
                }
                else{
                    ColorBlendAttachment[i].srcAlphaBlendFactor = BlendFactorToNative(BlendState.SourceColor);
                    ColorBlendAttachment[i].dstAlphaBlendFactor = BlendFactorToNative(BlendState.DestinationColor);
                    ColorBlendAttachment[i].alphaBlendOp = BlendOpToNative(BlendState.ColorOp);
                }
            }
        }
        else
        {
            for(u32 i=0; i<RenderPass->Output.NumColorFormats; i++)
            {
                ColorBlendAttachment[i] = vk::PipelineColorBlendAttachmentState();
                ColorBlendAttachment[i].blendEnable = VK_FALSE;
                ColorBlendAttachment[i].srcColorBlendFactor = vk::BlendFactor::eOne;
                ColorBlendAttachment[i].dstColorBlendFactor = vk::BlendFactor::eZero;
                ColorBlendAttachment[i].colorWriteMask = vk::ColorComponentFlagBits::eR |  vk::ColorComponentFlagBits::eG |  vk::ColorComponentFlagBits::eB |  vk::ColorComponentFlagBits::eA;
            }
        }

        vk::PipelineColorBlendStateCreateInfo ColorBlending;
        ColorBlending.setLogicOp(vk::LogicOp::eCopy)
                     //.setAttachmentCount(PipelineCreation.BlendState.ActiveStates)
                      .setAttachmentCount(PipelineCreation.BlendState.ActiveStates ? PipelineCreation.BlendState.ActiveStates : RenderPass->Output.NumColorFormats)
                     .setPAttachments(ColorBlendAttachment)
                     .setBlendConstants({0,0,0,0});
        PipelineCreateInfo.setPColorBlendState(&ColorBlending);


        vk::PipelineDepthStencilStateCreateInfo DepthStencil;
        DepthStencil.setDepthWriteEnable(PipelineCreation.DepthStencil.DepthWriteEnable ? true : false)
                    .setStencilTestEnable(PipelineCreation.DepthStencil.StencilEnable ? true : false)
                    .setDepthTestEnable(PipelineCreation.DepthStencil.DepthEnable ? true : false)
                    .setDepthCompareOp(CompareOpToNative(PipelineCreation.DepthStencil.DepthComparison));
        PipelineCreateInfo.setPDepthStencilState(&DepthStencil);


        vk::PipelineMultisampleStateCreateInfo MultisampleState;
        MultisampleState.setSampleShadingEnable(VK_FALSE)
                        .setRasterizationSamples(SampleCountToNative(RenderPass->Output.SampleCount))
                        .setMinSampleShading(1.0f)
                        .setPSampleMask(nullptr)
                        .setAlphaToCoverageEnable(VK_FALSE)
                        .setAlphaToOneEnable(VK_FALSE);
        PipelineCreateInfo.setPMultisampleState(&MultisampleState);

        vk::PipelineRasterizationStateCreateInfo Rasterizer;
        Rasterizer.setDepthClampEnable(VK_FALSE)
                  .setRasterizerDiscardEnable(VK_FALSE)
                  .setPolygonMode(vk::PolygonMode::eFill)
                  .setLineWidth(1)
                  .setCullMode(CullModeToNative(PipelineCreation.Rasterization.CullMode))
                  .setFrontFace(FrontFaceToNative(PipelineCreation.Rasterization.FrontFace))
                  .setDepthBiasEnable(VK_FALSE)
                  .setDepthBiasConstantFactor(0.0f)
                  .setDepthBiasClamp(0.0f)
                  .setDepthBiasSlopeFactor(0.0f);
        PipelineCreateInfo.setPRasterizationState(&Rasterizer);

        vk::Viewport Viewport;
        Viewport.x = 0;
        Viewport.y = 0;
        Viewport.width = (float)VkData->SurfaceExtent.width;
        Viewport.height = (float)VkData->SurfaceExtent.height;
        Viewport.minDepth = 0;
        Viewport.maxDepth = 1;

        vk::Rect2D Scissor;
        Scissor.offset = vk::Offset2D(0,0);
        Scissor.extent = vk::Extent2D(VkData->SurfaceExtent.width, VkData->SurfaceExtent.height);

        vk::PipelineViewportStateCreateInfo ViewportState;
        ViewportState.setViewportCount(1)
                     .setPViewports(&Viewport)
                     .setScissorCount(1)
                     .setPScissors(&Scissor);
        PipelineCreateInfo.setPViewportState(&ViewportState);

        GET_API_DATA(VkRenderPassData, vkRenderPassData, RenderPass);
        PipelineCreateInfo.setRenderPass(VkRenderPassData->NativeHandle);

        vk::DynamicState DynamicStates[] = {vk::DynamicState::eViewport, vk::DynamicState::eScissor};
        vk::PipelineDynamicStateCreateInfo DynamicState;
        DynamicState.setDynamicStateCount(2)
                    .setPDynamicStates(DynamicStates);
        PipelineCreateInfo.setPDynamicState(&DynamicState);

        this->NativeHandle = VkData->Device.createGraphicsPipeline(VK_NULL_HANDLE, PipelineCreateInfo).value;
        this->BindPoint = vk::PipelineBindPoint::eGraphics;
    }
    else
    {
        vk::ComputePipelineCreateInfo ComputePipelineCreateInfo;
        ComputePipelineCreateInfo.setStage(VkShaderData->ShaderStageCreateInfo[0])
                                 .setLayout(this->PipelineLayout);
        this->NativeHandle = VkData->Device.createComputePipeline(VK_NULL_HANDLE, ComputePipelineCreateInfo).value;
        this->BindPoint = vk::PipelineBindPoint::eCompute;
    }



    for (size_t j = 0; j < PipelineCreation.Shaders.StagesCount; j++)
    {
        DeallocateMemory((void*)PipelineCreation.Shaders.Stages[j].Code);
        DeallocateMemory((void*)PipelineCreation.Shaders.Stages[j].FileName);
    }
    DeallocateMemory((void*)PipelineCreation.Name);
}

void vkPipelineData::DestroyVkResources()
{   
    GET_CONTEXT(VkData, context::Get());

    shader *Shader = context::Get()->GetShader(this->ShaderState);
    GET_API_DATA(VkShaderData, vkShaderData, Shader);
    for (size_t i = 0; i < Shader->ActiveShaders; i++)
    {
        VkData->Device.destroyShaderModule(VkShaderData->ShaderStageCreateInfo[i].module);
    }
    
    VkData->Device.destroyPipeline(this->NativeHandle);
    VkData->Device.destroyPipelineLayout(this->PipelineLayout);

    std::shared_ptr<vkResourceManagerData> VKResourceManager = std::static_pointer_cast<vkResourceManagerData>(context::Get()->ResourceManager.ApiData);
    for(sz i=0; i<this->NumActiveLayouts; i++)
    {
        VkData->Device.destroyDescriptorSetLayout(this->DescriptorSetLayouts[i]->NativeHandle);
        VKResourceManager->DescriptorSetLayouts.ReleaseResource(this->DescriptorSetLayoutHandles[i]);
        DeallocateMemory(this->DescriptorSetLayouts[i]->Bindings);
    }
}

//

}
#endif