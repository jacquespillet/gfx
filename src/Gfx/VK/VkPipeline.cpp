#if GFX_API==GFX_VK
#include "../Include/Pipeline.h"
#include "../Include/Context.h"
#include "../Include/Types.h"

#include "VkResourceManager.h"
#include "VkRenderPass.h"
#include "VkShader.h"
#include "VkCommon.h"
#include "VkPipeline.h"
#include "VkBuffer.h"

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
uint32_t AlignedSize(uint32_t Value, uint32_t Alignment)
{
    return (Value + Alignment-1) & ~(Alignment-1);
}


void vkPipelineData::Create(const pipelineCreation &PipelineCreation)
{
    context *Context = context::Get(); 

    GET_CONTEXT(VkData, Context);
    shader *ShaderStateData = Context->GetShader(this->ShaderState);
    GET_API_DATA(VkShaderData, vkShaderData, ShaderStateData);


    renderPass *RenderPass = Context->GetRenderPass(PipelineCreation.RenderPassHandle);

    vk::DescriptorSetLayout Layouts[vkConstants::MaxDescriptorSetLayouts];

    u32 NumActiveLayouts = VkShaderData->SpirvParseResults.SetCount;
    for(sz i=0; i<NumActiveLayouts; i++)
    {
        this->DescriptorSetLayoutHandles[i] = CreateDescriptorSetLayout2(VkShaderData->SpirvParseResults.Sets[i]);
        
        std::shared_ptr<vkResourceManagerData> VkResourceManagerData = std::static_pointer_cast<vkResourceManagerData>(Context->ResourceManager.ApiData);
        this->DescriptorSetLayouts[i] = (descriptorSetLayout*)VkResourceManagerData->DescriptorSetLayouts.GetResource(this->DescriptorSetLayoutHandles[i]);
        Layouts[i] = this->DescriptorSetLayouts[i]->NativeHandle;
    }

    vk::PipelineLayoutCreateInfo PipelineLayoutCreate;
    PipelineLayoutCreate.setPSetLayouts(Layouts)
                        .setSetLayoutCount(NumActiveLayouts);

    this->PipelineLayout = VkData->Device.createPipelineLayout(PipelineLayoutCreate);
    this->NumActiveLayouts = NumActiveLayouts;

    bool GraphicsPipeline = !ShaderStateData->ComputePipeline && !ShaderStateData->RTXPipeline;
    if(GraphicsPipeline)
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
    else if(ShaderStateData->ComputePipeline)
    {
        vk::ComputePipelineCreateInfo ComputePipelineCreateInfo;
        ComputePipelineCreateInfo.setStage(VkShaderData->ShaderStageCreateInfo[0])
                                 .setLayout(this->PipelineLayout);
        this->NativeHandle = VkData->Device.createComputePipeline(VK_NULL_HANDLE, ComputePipelineCreateInfo).value;
        this->BindPoint = vk::PipelineBindPoint::eCompute;
    }
    else if(ShaderStateData->RTXPipeline)
    {
#if 0
        // For each group
        int RayGenGroup=-1;
        int MissGroup=-1;
        int IsectGroup=-1;
        // Gathers all the shaders of the same group in a same vector
        std::unordered_map<int, std::vector<int>> GroupMapping = {};
        for(int i=0; i<PipelineCreation.Shaders.StagesCount; i++)
        {
            int Group = PipelineCreation.Shaders.Stages[i].Group;
            GroupMapping[Group].push_back(i);
        }

        // Create the shader groups
        VkShaderData->ShaderGroups.resize(GroupMapping.size());
        for(u32 i=0; i<VkShaderData->ShaderGroups.size(); i++)
        {
            
            // Default
            VkRayTracingShaderGroupCreateInfoKHR &ShaderGroup = VkShaderData->ShaderGroups[i];
            ShaderGroup = {VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR};
            ShaderGroup.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
            ShaderGroup.closestHitShader = VK_SHADER_UNUSED_KHR;
            ShaderGroup.anyHitShader = VK_SHADER_UNUSED_KHR;
            ShaderGroup.intersectionShader = VK_SHADER_UNUSED_KHR;   
            ShaderGroup.generalShader = VK_SHADER_UNUSED_KHR;
            
            // Go through all the shaders in that group
            for(u32 j=0; j<GroupMapping[i].size(); j++)
            {
                // Get its index in the stages
                u32 ShaderInx = GroupMapping[i][j];

                // Set the type
                if(PipelineCreation.Shaders.Stages[ShaderInx].Stage == shaderStageFlags::bits::ClosestHitKHR)
                {
                    ShaderGroup.type =VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
                    ShaderGroup.closestHitShader = ShaderInx;
                    IsectGroup = i;
                }                            
                if(PipelineCreation.Shaders.Stages[ShaderInx].Stage == shaderStageFlags::bits::AnyHitKHR)
                {
                    ShaderGroup.type =VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
                    ShaderGroup.anyHitShader = ShaderInx;
                }                     
                if(PipelineCreation.Shaders.Stages[ShaderInx].Stage == shaderStageFlags::bits::MissKHR | PipelineCreation.Shaders.Stages[ShaderInx].Stage == shaderStageFlags::bits::RaygenKHR)
                {
                    if(PipelineCreation.Shaders.Stages[ShaderInx].Stage == shaderStageFlags::bits::MissKHR )
                        MissGroup = i;
                    if(PipelineCreation.Shaders.Stages[ShaderInx].Stage == shaderStageFlags::bits::RaygenKHR)
                        RayGenGroup = i;
                    ShaderGroup.generalShader = ShaderInx;
                }                     
            }
        }

        if(RayGenGroup != -1 && IsectGroup != -1 && MissGroup != -1)
        {
            VkRayTracingPipelineCreateInfoKHR RayTracingPipelineCreateInfo = {VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR};
            RayTracingPipelineCreateInfo.stageCount= PipelineCreation.Shaders.StagesCount;
            RayTracingPipelineCreateInfo.pStages = (VkPipelineShaderStageCreateInfo*)VkShaderData->ShaderStageCreateInfo;
            RayTracingPipelineCreateInfo.groupCount = static_cast<uint32_t>(VkShaderData->ShaderGroups.size());
            RayTracingPipelineCreateInfo.pGroups = VkShaderData->ShaderGroups.data();
            RayTracingPipelineCreateInfo.maxPipelineRayRecursionDepth=2;
            RayTracingPipelineCreateInfo.layout = PipelineLayout;
            VK_CALL(VkData->_vkCreateRayTracingPipelinesKHR((VkDevice)VkData->Device, VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &RayTracingPipelineCreateInfo, nullptr, (VkPipeline*)&this->NativeHandle));
            this->BindPoint = vk::PipelineBindPoint::eRayTracingKHR;


            u32 TotalShaders = PipelineCreation.Shaders.StagesCount;
            u32 GroupCount = VkShaderData->ShaderGroups.size();
            u32 HandleSize = VkData->RayTracingPipelineProperties.shaderGroupHandleSize;
            u32 HandleSizeAligned = AlignedSize(HandleSize, VkData->RayTracingPipelineProperties.shaderGroupHandleAlignment);

            // Get the shader group handles
            u32 DataSize = TotalShaders * HandleSize;
            std::vector<uint8_t> ShaderHandleStorage(DataSize);
            VK_CALL(VkData->_vkGetRayTracingShaderGroupHandlesKHR((VkDevice)VkData->Device, (VkPipeline)this->NativeHandle, 0, GroupCount, DataSize, ShaderHandleStorage.data()));

            u64 RayGenSize = AlignedSize(HandleSizeAligned, VkData->RayTracingPipelineProperties.shaderGroupBaseAlignment);
            u64 MissSize = AlignedSize(HandleSizeAligned * GroupMapping[MissGroup].size(), VkData->RayTracingPipelineProperties.shaderGroupBaseAlignment);
            u64 IsectSize = AlignedSize(HandleSizeAligned * GroupMapping[IsectGroup].size(), VkData->RayTracingPipelineProperties.shaderGroupBaseAlignment);
            u64 CallableSize = 0;

            u64 SBTSize = RayGenSize + MissSize + IsectSize + CallableSize;
            
            SBT = Context->CreateBuffer(SBTSize, bufferUsage::ShaderBindingTable | bufferUsage::TransferSource | bufferUsage::ShaderDeviceAddress, memoryUsage::CpuToGpu);
                        
            u64 BaseAddress = VkData->GetBufferDeviceAddress(SBT);

            u64 GroupOffset = 0;
            auto GetHandle = [&](int i) { return ShaderHandleStorage.data() + i * HandleSize; };
            for(u32 i =0; i<GroupCount; i++)
            {
                u64 CurrentSize = 0;
                if(i==RayGenGroup)
                {
                    RayGenSBTAddress.deviceAddress = BaseAddress;
                    RayGenSBTAddress.stride = RayGenSize;
                    RayGenSBTAddress.size = RayGenSBTAddress.stride; // THat's because there can only be 1 raygen shader.
                    BaseAddress += RayGenSBTAddress.size;
                    CurrentSize = RayGenSBTAddress.size;
                }
                if(i==MissGroup)
                {
                    MissSBTAddress.deviceAddress = BaseAddress;
                    MissSBTAddress.stride = HandleSizeAligned;
                    MissSBTAddress.size = MissSize;
                    BaseAddress += MissSBTAddress.size;
                    CurrentSize = MissSBTAddress.size;
                    
                }
                if(i==IsectGroup)
                {
                    IsectSBTAddress.deviceAddress = BaseAddress;
                    IsectSBTAddress.stride = HandleSizeAligned;
                    IsectSBTAddress.size = IsectSize;
                    BaseAddress += IsectSBTAddress.size;
                    CurrentSize = IsectSBTAddress.size;
                }
                // Copy handle to buffer
                u32 k=0;
                u32 HandleOffset=0;
                for(auto &Handle : GroupMapping[i])
                {
                    Context->CopyDataToBuffer(SBT, GetHandle(GroupMapping[i][k]), HandleSize, GroupOffset + HandleOffset);
                    k++;
                    HandleOffset += HandleSizeAligned;
                }  
                GroupOffset += CurrentSize;                  
            }


            // TODO
            CallableSBTAddress.deviceAddress = 0;
            CallableSBTAddress.stride = 0;
            CallableSBTAddress.size = 0;
        }

#else
        // create Hit Groups
        VkRayTracingShaderGroupCreateInfoKHR RayGenGroup;
        std::vector<VkRayTracingShaderGroupCreateInfoKHR> HitGroups;
        std::vector<VkRayTracingShaderGroupCreateInfoKHR> MissGroups;

        int RayGenIndex = -1;
        std::vector<int> MissIndices;
        for(int i=0; i<PipelineCreation.Shaders.StagesCount; i++)
        {

            if(PipelineCreation.Shaders.Stages[i].Stage == shaderStageFlags::RaygenKHR)
            {
                RayGenGroup = {VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR};
                RayGenGroup.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
                RayGenGroup.closestHitShader = VK_SHADER_UNUSED_KHR;
                RayGenGroup.anyHitShader = VK_SHADER_UNUSED_KHR;
                RayGenGroup.intersectionShader = VK_SHADER_UNUSED_KHR;   
                RayGenGroup.generalShader = i;
                RayGenIndex = i;
            }
            
            if(PipelineCreation.Shaders.Stages[i].Stage == shaderStageFlags::MissKHR)
            {
                VkRayTracingShaderGroupCreateInfoKHR MissGroup = {VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR};
                MissGroup.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
                MissGroup.closestHitShader = VK_SHADER_UNUSED_KHR;
                MissGroup.anyHitShader = VK_SHADER_UNUSED_KHR;
                MissGroup.intersectionShader = VK_SHADER_UNUSED_KHR;   
                MissGroup.generalShader = i;
                MissGroups.push_back(MissGroup);
                MissIndices.push_back(i);
            }
        }
        for(int i=0; i<PipelineCreation.HitGroups.size(); i++)
        {
            VkRayTracingShaderGroupCreateInfoKHR HitGroup = {VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR};
            HitGroup.type = PipelineCreation.HitGroups[i].IsectInx >= 0 ? VK_RAY_TRACING_SHADER_GROUP_TYPE_PROCEDURAL_HIT_GROUP_KHR : VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
            HitGroup.closestHitShader = PipelineCreation.HitGroups[i].ClosestHitInx >=0 ? PipelineCreation.HitGroups[i].ClosestHitInx : VK_SHADER_UNUSED_KHR;
            HitGroup.anyHitShader = PipelineCreation.HitGroups[i].AnyHitInx >=0 ? PipelineCreation.HitGroups[i].AnyHitInx : VK_SHADER_UNUSED_KHR;
            HitGroup.intersectionShader = PipelineCreation.HitGroups[i].IsectInx >=0 ? PipelineCreation.HitGroups[i].IsectInx : VK_SHADER_UNUSED_KHR;
            HitGroup.generalShader = VK_SHADER_UNUSED_KHR;
            HitGroups.push_back(HitGroup);   
        }

        u32 TotalGroupCount = 1 + HitGroups.size() + MissGroups.size();
        VkShaderData->ShaderGroups.reserve(TotalGroupCount);
        VkShaderData->ShaderGroups.push_back(RayGenGroup);
        for(s32 i=0; i<HitGroups.size(); i++)
        {
            VkShaderData->ShaderGroups.push_back(HitGroups[i]); 
        }
        for(s32 i=0; i<MissGroups.size(); i++)
        {
            VkShaderData->ShaderGroups.push_back(MissGroups[i]); 
        }

        VkRayTracingPipelineCreateInfoKHR RayTracingPipelineCreateInfo = {VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR};
        RayTracingPipelineCreateInfo.stageCount= PipelineCreation.Shaders.StagesCount;
        RayTracingPipelineCreateInfo.pStages = (VkPipelineShaderStageCreateInfo*)VkShaderData->ShaderStageCreateInfo;
        RayTracingPipelineCreateInfo.groupCount = TotalGroupCount;
        RayTracingPipelineCreateInfo.pGroups = VkShaderData->ShaderGroups.data();
        RayTracingPipelineCreateInfo.maxPipelineRayRecursionDepth=2;
        RayTracingPipelineCreateInfo.layout = PipelineLayout;
        VK_CALL(VkData->_vkCreateRayTracingPipelinesKHR((VkDevice)VkData->Device, VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &RayTracingPipelineCreateInfo, nullptr, (VkPipeline*)&this->NativeHandle));
        this->BindPoint = vk::PipelineBindPoint::eRayTracingKHR;


        u32 TotalShaders = PipelineCreation.Shaders.StagesCount;
        u32 HandleSize = VkData->RayTracingPipelineProperties.shaderGroupHandleSize;
        u32 HandleSizeAligned = AlignedSize(HandleSize, VkData->RayTracingPipelineProperties.shaderGroupHandleAlignment);

        // Get the shader group handles
        u32 DataSize = TotalGroupCount * HandleSize;
        std::vector<uint8_t> ShaderHandleStorage(DataSize);
        VK_CALL(VkData->_vkGetRayTracingShaderGroupHandlesKHR((VkDevice)VkData->Device, (VkPipeline)this->NativeHandle, 0, TotalGroupCount, DataSize, ShaderHandleStorage.data()));

        u64 RayGenSize = AlignedSize(HandleSizeAligned, VkData->RayTracingPipelineProperties.shaderGroupBaseAlignment);
        u64 MissSize = AlignedSize(HandleSizeAligned * MissGroups.size(), VkData->RayTracingPipelineProperties.shaderGroupBaseAlignment);
        u64 IsectSize = AlignedSize(HandleSizeAligned * HitGroups.size(), VkData->RayTracingPipelineProperties.shaderGroupBaseAlignment);
        u64 CallableSize = 0;

        u64 SBTSize = RayGenSize + MissSize + IsectSize + CallableSize;
        
        SBT = Context->CreateBuffer(SBTSize, bufferUsage::ShaderBindingTable | bufferUsage::TransferSource | bufferUsage::ShaderDeviceAddress, memoryUsage::CpuToGpu);
                    
        u64 BaseAddress = VkData->GetBufferDeviceAddress(SBT);

        auto GetHandle = [&](int i) { return ShaderHandleStorage.data() + i * HandleSize; };
        
        int CurrentInx=0;
        u64 Offset = 0;

        // Ray gen at the start of the SBT
        RayGenSBTAddress.deviceAddress = BaseAddress;
        RayGenSBTAddress.stride = RayGenSize;
        RayGenSBTAddress.size = RayGenSBTAddress.stride; // THat's because there can only be 1 raygen shader.
        BaseAddress += RayGenSBTAddress.size;
        Context->CopyDataToBuffer(SBT, GetHandle(CurrentInx), HandleSize, Offset);
        CurrentInx++;
        Offset += RayGenSBTAddress.size;
        
        
        IsectSBTAddress.deviceAddress = BaseAddress;
        IsectSBTAddress.stride = HandleSizeAligned;
        IsectSBTAddress.size = IsectSize;
        BaseAddress += IsectSBTAddress.size;
        Context->CopyDataToBuffer(SBT, GetHandle(CurrentInx), IsectSize, Offset);
        CurrentInx += HitGroups.size();
        Offset += IsectSBTAddress.size;
        
        
        MissSBTAddress.deviceAddress = BaseAddress;
        MissSBTAddress.stride = HandleSizeAligned;
        MissSBTAddress.size = MissSize;
        BaseAddress += MissSBTAddress.size;
        Context->CopyDataToBuffer(SBT, GetHandle(CurrentInx), MissSize, Offset);
        CurrentInx += MissGroups.size();
        Offset += MissSBTAddress.size;
        


        // TODO
        CallableSBTAddress.deviceAddress = 0;
        CallableSBTAddress.stride = 0;
        CallableSBTAddress.size = 0;

        // Build SBT

        
#endif
       
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