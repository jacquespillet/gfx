#if GFX_API==GFX_VK
#include "VkShader.h"
#include "../Include/Memory.h"
#include <iostream>


namespace gfx
{

void vkShaderData::Create(const shaderStateCreation &Creation)
{
    GET_CONTEXT(VkData, context::Get());

    this->SpirvParseResults= {};
    this->ComputePipeline=false;
    this->RTXPipeline=false;
    
    CompiledShaders=0;
    for(CompiledShaders = 0; CompiledShaders < Creation.StagesCount; CompiledShaders++)
    {
        const shaderStage &ShaderStage = Creation.Stages[CompiledShaders];
        if(ShaderStage.Stage == shaderStageFlags::Compute)
        {
            this->ComputePipeline=true;
        }

        if(ShaderStage.Stage == shaderStageFlags::RaygenKHR || ShaderStage.Stage == shaderStageFlags::MissKHR || ShaderStage.Stage == shaderStageFlags::ClosestHitKHR || ShaderStage.Stage == shaderStageFlags::AnyHitKHR)
        {
            this->RTXPipeline=true;
        }

        vk::ShaderModuleCreateInfo ShaderCreateInfo;
        if(Creation.SpvInput)
        {
            ShaderCreateInfo.codeSize = ShaderStage.CodeSize;
            ShaderCreateInfo.pCode = reinterpret_cast< const u32* >(ShaderStage.Code);
        }
        else
        {
            ShaderCreateInfo = CompileShader(ShaderStage.Code, ShaderStage.CodeSize, ShaderStage.Stage, Creation.Name);
        }


        vk::PipelineShaderStageCreateInfo &ShaderStageCreateInfo = this->ShaderStageCreateInfo[CompiledShaders];
        memset(&ShaderStageCreateInfo, 0, sizeof(vk::PipelineShaderStageCreateInfo));
        ShaderStageCreateInfo = vk::PipelineShaderStageCreateInfo();
        ShaderStageCreateInfo.setPName("main") .setStage(ShaderStageToNative(ShaderStage.Stage));

        this->ShaderStageCreateInfo[CompiledShaders].module = VkData->Device.createShaderModule(ShaderCreateInfo);

        ParseSpirv((void*)((char*)ShaderCreateInfo.pCode), ShaderCreateInfo.codeSize, this->SpirvParseResults);

        DeallocateMemory((void*)ShaderCreateInfo.pCode);
    }
}

EShLanguage GetShaderType(shaderStageFlags::bits Stage)
{
    switch (Stage)
    {
    case shaderStageFlags::Vertex:
        return EShLangVertex;
        break;
    case shaderStageFlags::TessellationControl:
        return EShLangTessControl;
        break;
    case shaderStageFlags::TessellationEvaluation:
        return EShLangTessEvaluation;
        break;
    case shaderStageFlags::Geometry:
        return EShLangGeometry;
        break;
    case shaderStageFlags::Fragment:
        return EShLangFragment;
        break;
    case shaderStageFlags::Compute:
        return EShLangCompute;
        break;
    case shaderStageFlags::RaygenKHR:
        return EShLangRayGen;
        break;
    case shaderStageFlags::IntersectionKHR:
        return EShLangIntersect;
        break;
    case shaderStageFlags::AnyHitKHR:
        return EShLangAnyHit;
        break;
    case shaderStageFlags::ClosestHitKHR:
        return EShLangClosestHit;
        break;
    case shaderStageFlags::MissKHR:
        return EShLangMiss;
        break;
    case shaderStageFlags::CallableKHR:
        return EShLangCallable;
        break;
    case shaderStageFlags::TaskNV:
        return EShLangTaskNV;
        break;
    case shaderStageFlags::MeshNV:
        return EShLangMeshNV;
        break;
    default:
        break;
    }

    return EShLangVertex;
}


vk::ShaderModuleCreateInfo CompileShader(const char *Code, u32 CodeSize, shaderStageFlags::bits Stage, const char *Name)
{
    context *Context = context::Get();
    std::shared_ptr<vkData> Data = std::static_pointer_cast<vkData>(Context->ApiContextData);
    
    vk::ShaderModuleCreateInfo ShaderModule;
    std::vector<u32> ByteCode;
    //Compile
    {
        WriteFileString("Shader", Code);
        constexpr static auto ResourceLimits = GetResourceLimits();
        
        EShLanguage Type = GetShaderType(Stage);

        glslang::TShader Shader {Type};
        Shader.setStrings(&Code, 1);
        Shader.setEnvInput(glslang::EShSource::EShSourceGlsl, Type, glslang::EShClientVulkan, 450);
        Shader.setEnvClient(glslang::EShClient::EShClientVulkan, (glslang::EshTargetClientVersion)Data->ApiVersion);
        Shader.setEnvTarget(glslang::EShTargetLanguage::EShTargetSpv, glslang::EShTargetLanguageVersion::EShTargetSpv_1_5);
        
        bool IsParsed = Shader.parse(&ResourceLimits, 450, false, EShMessages::EShMsgDefault);
        if(!IsParsed)
        {
            printf(Shader.getInfoLog());
            printf(Shader.getInfoDebugLog());
            printf("Shader Code : \n");
            printf("______________________________________________________\n");
            int LineNumber = 1;
            std::istringstream iss(Code);
            std::string line;        
            while (std::getline(iss, line)) {
                std::cout << "Line " << LineNumber << ": " << line << std::endl;
                LineNumber++;
            }            
            // printf(Code);
            printf("______________________________________________________\n");
        }
        assert(IsParsed);

        glslang::TProgram Program;
        Program.addShader(&Shader);  
        bool IsLinked = Program.link(EShMessages::EShMsgDefault);
        if(!IsLinked)
        {  
            printf(Program.getInfoLog());
            printf(Program.getInfoDebugLog());
            printf("Shader Code : \n");
            printf("______________________________________________________\n");
            int LineNumber = 1;
            std::istringstream iss(Code);
            std::string line;        
            while (std::getline(iss, line)) {
                std::cout << "Line " << LineNumber << ": " << line << std::endl;
                LineNumber++;
            }   
            printf("______________________________________________________\n");
        }
        assert(IsLinked);

        auto Intermediate = Program.getIntermediate(Type);
        glslang::GlslangToSpv(*Intermediate, ByteCode);

        u32 *C = (u32*)AllocateMemory(ByteCode.size() * sizeof(u32));
        memcpy(C, ByteCode.data(), ByteCode.size() * sizeof(u32));
        ShaderModule.setPCode(C).setCodeSize(ByteCode.size() * sizeof(u32));
    }    
    return ShaderModule;
}

void AddBindingIfUnique(descriptorSetLayoutCreation &SetLayout, descriptorSetLayoutCreation::binding &Binding)
{
    b8 Found = false;
    for(sz i=0; i<SetLayout.NumBindings; i++)
    {
        const descriptorSetLayoutCreation::binding &OtherBinding = SetLayout.Bindings[i];
        if(OtherBinding.Type == Binding.Type && OtherBinding.Start == Binding.Start)
        {
            Found=true;
            break;
        }
    }

    if(!Found)
    {
        SetLayout.AddBinding(Binding);
    }
}

u32 Max(u32 v0, u32 v1)
{
    return (v0 > v1) ? v0 : v1;
}

void ParseSpirv(void* ByteCode, sz ByteCodeSize, spirvParseResult &Results)
{
    SpvReflectResult SpvResult;
    SpvReflectShaderModule ReflectedShader;
    SpvResult = spvReflectCreateShaderModule(ByteCodeSize, ByteCode, &ReflectedShader);
    assert(SpvResult == SPV_REFLECT_RESULT_SUCCESS);

    u32 DescriptorBindingCount = 0;
    SpvResult = spvReflectEnumerateDescriptorBindings(&ReflectedShader, &DescriptorBindingCount, nullptr);
    assert(SpvResult == SPV_REFLECT_RESULT_SUCCESS);
    std::vector<SpvReflectDescriptorBinding*> DescriptorBindings(DescriptorBindingCount);
    SpvResult = spvReflectEnumerateDescriptorBindings(&ReflectedShader, &DescriptorBindingCount, DescriptorBindings.data());
    assert(SpvResult == SPV_REFLECT_RESULT_SUCCESS);

    //For each binding
    for(const auto &DescriptorBinding : DescriptorBindings)
    {
        // if(DescriptorBinding->set == device::BindlessSet && 
        //     (DescriptorBinding->binding == device::BindlessTextureBinding || DescriptorBinding->binding == (device::BindlessTextureBinding+1)))
        // {
    //     continue;
        // }

        //Create a descriptor set layout
        descriptorSetLayoutCreation &SetLayout = Results.Sets[DescriptorBinding->set];
        SetLayout.SetSetIndex(DescriptorBinding->set);
        
        descriptorSetLayoutCreation::binding Binding{};
        Binding.Start = DescriptorBinding->binding;
        Binding.Count = 1;
        Binding.Name = DescriptorBinding->name;

        //Bindless descriptor
        if (DescriptorBinding->array.dims_count == 1 && DescriptorBinding->descriptor_type == SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) 
        {
            Binding.Count = vkData::BindlessTexturesCount;
        }
        if (DescriptorBinding->array.dims_count == 1 && DescriptorBinding->descriptor_type == SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER) 
        {
            Binding.Count = vkData::BindlessUniformBuffersCount;
        }

        if(DescriptorBinding->descriptor_type == SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
            Binding.Type = vk::DescriptorType::eUniformBuffer;
        if(DescriptorBinding->descriptor_type == SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER)
            Binding.Type = vk::DescriptorType::eStorageBuffer;
        else if(DescriptorBinding->descriptor_type == SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
            Binding.Type = vk::DescriptorType::eCombinedImageSampler;
        else if(DescriptorBinding->descriptor_type == SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_IMAGE)
            Binding.Type = vk::DescriptorType::eStorageImage;
        else if(DescriptorBinding->descriptor_type == SPV_REFLECT_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR)
            Binding.Type = vk::DescriptorType::eAccelerationStructureKHR;
        
		AddBindingIfUnique(SetLayout, Binding);

        Results.SetCount = Max(Results.SetCount, (DescriptorBinding->set + 1));
    }
}



descriptorSetLayoutCreation &descriptorSetLayoutCreation::Reset()
{
    NumBindings=0;
    SetIndex=0;
    return *this;
}
descriptorSetLayoutCreation &descriptorSetLayoutCreation::AddBinding(const binding &Binding)
{
    this->Bindings[NumBindings++] = Binding;
    return *this;
}
descriptorSetLayoutCreation &descriptorSetLayoutCreation::SetSetIndex(u32 Index)
{
    this->SetIndex = Index;
    return *this;
}
descriptorSetLayoutCreation &descriptorSetLayoutCreation::SetBindless(b8 Bindless)
{
    this->Bindless = Bindless;
    return *this;
}
descriptorSetLayoutCreation &descriptorSetLayoutCreation::AddBinding(vk::DescriptorType Type, u32 Index, u32 Count, const char *Name)
{
    this->Bindings[NumBindings++] = {Type, (u16)Index, (u16)Count, Name};
    return *this;
}
descriptorSetLayoutCreation &descriptorSetLayoutCreation::AddBindingAtIndex(const binding &Binding, u32 Index)
{
    this->Bindings[Index] = Binding;
    NumBindings = (Index+1) > NumBindings ? (Index+1) : NumBindings;
    return *this;
}
descriptorSetLayoutCreation &descriptorSetLayoutCreation::SetName(const char *Name)
{
    this->Name = Name;
    return *this;
}

}
#endif