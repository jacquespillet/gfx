#include "../Include/Pipeline.h"
#include "../Include/GfxContext.h"
#include "../Include/Memory.h"
#include "Util.h"

namespace gfx
{
    
void ShaderConcatenate(std::string &FileName, std::string &Code, std::string &ParentPath)
{
    std::string Path = ParentPath + "/" + FileName;
    std::string FileContent = ReadFileString(Path.c_str());

    Code += FileContent + "\n";
}

const char *AllocateCString(std::string Str)
{

    char *Result = (char*)AllocateMemory(Str.size() + 1);
    memcpy((void*)Result, Str.data(), Str.size());
    Result[Str.size()] = '\0';
    return Result;
}

//

vertexInputCreation &vertexInputCreation::Reset()
{
    NumVertexStreams = NumVertexAttributes = 0;
    return *this;
}

vertexInputCreation &vertexInputCreation::AddVertexStream(const vertexStream &Stream)
{
    VertexStreams[NumVertexStreams++] = Stream;
    return *this;
}

vertexInputCreation &vertexInputCreation::AddVertexAttribute(const vertexAttribute &Attribute)
{
    VertexAttributes[NumVertexAttributes++] = Attribute;
    return *this;
}


//
shaderStateCreation &shaderStateCreation::SetName(const char * Name)
{
    this->Name = Name;
    return *this;
}

shaderStateCreation &shaderStateCreation::AddStage(const char *Code, u32 CodeSize, shaderStageFlags::bits Stage)
{
    Stages[StagesCount].Code = Code;   
    Stages[StagesCount].CodeSize = CodeSize;   
    Stages[StagesCount].Stage = Stage;
    ++StagesCount;
    return *this;   
}

shaderStateCreation &shaderStateCreation::AddStage(const char *FileName, shaderStageFlags::bits Stage)
{
    
    std::string CodeStr = ReadShaderFile(FileName).c_str();
    char *Code = (char*)AllocateMemory(CodeStr.size() + 1);


    strcpy(Code, CodeStr.c_str());

    Stages[StagesCount].Code = Code;   
    Stages[StagesCount].CodeSize = (u32)strlen(Code);   
    Stages[StagesCount].Stage = Stage;
    ++StagesCount;
    return *this;   
}

shaderStateCreation &shaderStateCreation::Reset()
{
    StagesCount=0;
    return *this;
}


//
blendState &blendState::SetColor(blendFactor Source, blendFactor Dest, blendOperation Operation)
{
    this->SourceColor = Source;
    this->DestinationColor = Dest;
    this->ColorOp = Operation;
    BlendEnabled=1;
    return *this;
}

//
blendState &blendStateCreation::AddBlendState()
{
    return BlendStates[ActiveStates++]; 
}


blendFactor GetBlendFactor( const std::string factor ) {
    if ( factor == "ZERO" ) {
        return blendFactor::Zero;
    }
    if ( factor == "ONE" ) {
        return blendFactor::One;
    }
    if ( factor == "SRC_COLOR" ) {
        return blendFactor::SrcColor;
    }
    if ( factor == "ONE_MINUS_SRC_COLOR" ) {
        return blendFactor::OneMinusSrcColor;
    }
    if ( factor == "DST_COLOR" ) {
        return blendFactor::DstColor;
    }
    if ( factor == "ONE_MINUS_DST_COLOR" ) {
        return blendFactor::OneMinusDstColor;
    }
    if ( factor == "SRC_ALPHA" ) {
        return blendFactor::SrcAlpha;
    }
    if ( factor == "ONE_MINUS_SRC_ALPHA" ) {
        return blendFactor::OneMinusSrcAlpha;
    }
    if ( factor == "DST_ALPHA" ) {
        return blendFactor::DstAlpha;
    }
    if ( factor == "ONE_MINUS_DST_ALPHA" ) {
        return blendFactor::OneMinusDstAlpha;
    }
    if ( factor == "CONSTANT_COLOR" ) {
        return blendFactor::ConstantColor;
    }
    if ( factor == "ONE_MINUS_CONSTANT_COLOR" ) {
        return blendFactor::OneMinusConstantColor;
    }
    if ( factor == "CONSTANT_ALPHA" ) {
        return blendFactor::ConstantAlpha;
    }
    if ( factor == "ONE_MINUS_CONSTANT_ALPHA" ) {
        return blendFactor::OneMinusConstantAlpha;
    }
    if ( factor == "SRC_ALPHA_SATURATE" ) {
        return blendFactor::SrcAlphaSaturate;
    }
    if ( factor == "SRC1_COLOR" ) {
        return blendFactor::Src1Color;
    }
    if ( factor == "ONE_MINUS_SRC1_COLOR" ) {
        return blendFactor::OneMinusSrc1Color;
    }
    if ( factor == "SRC1_ALPHA" ) {
        return blendFactor::Src1Alpha;
    }
    if ( factor == "ONE_MINUS_SRC1_ALPHA" ) {
        return blendFactor::OneMinusSrc1Alpha;
    }

    return blendFactor::One;
}

blendOperation GetBlendOp( const std::string op ) {
    if ( op == "ADD" ) {
        blendOperation::Add;
    }
    if ( op == "SUBTRACT" ) {
        blendOperation::Subtract;
    }
    if ( op == "REVERSE_SUBTRACT" ) {
        blendOperation::ReverseSubtract;
    }
    if ( op == "MIN" ) {
        blendOperation::Min;
    }
    if ( op == "MAX" ) {
        blendOperation::Max;
    }

    return blendOperation::Add;
}

void ParseGPUPipeline(nlohmann::json &PipelineJSON, pipelineCreation &PipelineCreation, std::string &ParentPath)
{
    using json = nlohmann::json;
    
    std::string Name;
    PipelineJSON["name"].get_to(Name);
    PipelineCreation.Name = AllocateCString(Name);

    json Shaders = PipelineJSON["shaders"];
    if(!Shaders.is_null())
    {
        for(sz i=0; i<Shaders.size(); i++)
        {
            json ShaderStage = Shaders[i];

            std::string Name;
            std::string Code;

            json Includes = ShaderStage["includes"];
            if(Includes.is_array())
            {
                for(sz j=0; j<Includes.size(); j++)
                {
                    Includes[j].get_to(Name);
                    ShaderConcatenate(Name, Code, ParentPath);
                }
            }


            ShaderStage["shader"].get_to(Name);
            ShaderConcatenate(Name, Code, ParentPath);

            ShaderStage["stage"].get_to(Name);
            
            std::string CustomDefines;
            if(Name == "vertex")
                CustomDefines = "#define VERTEX";
            if(Name == "fragment")
                CustomDefines = "#define FRAGMENT";
            if(Name == "compute")
                CustomDefines = "#define COMPUTE";
            
            // Code.replace(Code.begin(), Code.end(), "{{CustomDefines}}", CustomDefines);
            Code = std::regex_replace(Code, std::regex("CUSTOM_DEFINES"), CustomDefines);
            const char *CodeCStr = AllocateCString(Code.c_str());

            if(Name == "vertex")
                PipelineCreation.Shaders.AddStage(CodeCStr, strlen(CodeCStr), shaderStageFlags::bits::Vertex);
            else if(Name == "fragment")
                PipelineCreation.Shaders.AddStage(CodeCStr, strlen(CodeCStr), shaderStageFlags::bits::Fragment);
            else if(Name == "compute")
                PipelineCreation.Shaders.AddStage(CodeCStr, strlen(CodeCStr), shaderStageFlags::bits::Compute);
        }
    }

    json VertexInputs = PipelineJSON["vertex_input"];
    if(VertexInputs.is_array())
    {
        PipelineCreation.VertexInput.NumVertexAttributes =0;
        PipelineCreation.VertexInput.NumVertexStreams =0;

        for(sz i=0; i<VertexInputs.size(); i++)
        {
            vertexAttribute VertexAttribute{};

            json VertexInput = VertexInputs[i];
            VertexAttribute.Location = (u16) VertexInput.value("attribute_location", 0u);
            VertexAttribute.Binding = (u16) VertexInput.value("attribute_binding", 0u);
            VertexAttribute.Offset = VertexInput.value("attribute_offset", 0u);

            json AttributeFormat = VertexInput["attribute_format"];
            if(AttributeFormat.is_string())
            {
                std::string Name;
                AttributeFormat.get_to(Name);
                for(u32 j=0; j<vertexComponentFormat::Count; j++)
                {
                    vertexComponentFormat::values EnumValue = (vertexComponentFormat::values)j;
                    if(Name == vertexComponentFormat::ToString(EnumValue))
                    {
                        VertexAttribute.Format = EnumValue;
                        break;
                    }
                }
            }
            PipelineCreation.VertexInput.AddVertexAttribute(VertexAttribute);


            vertexStream VertexStream{};
            VertexStream.Binding = (u16) VertexInput.value("stream_binding", 0);
            VertexStream.Stride = (u16) VertexInput.value("stream_stride", 0);

            json StreamRate = VertexInput["stream_rate"];
            if(StreamRate.is_string())
            {
                std::string Name;
                StreamRate.get_to(Name);

                if(Name == "Vertex")
                {
                    VertexStream.InputRate = vertexInputRate::PerVertex;
                }
                else if(Name == "Instance")
                {
                    VertexStream.InputRate = vertexInputRate::PerInstance;
                }
                else
                {
                    assert(false);
                }
            }
            PipelineCreation.VertexInput.AddVertexStream(VertexStream);
        }
    }

    json Depth = PipelineJSON["depth"];
    if(!Depth.is_null())
    {
        PipelineCreation.DepthStencil.DepthEnable = 1;
        PipelineCreation.DepthStencil.DepthWriteEnable = Depth.value("write", false);

        json Comparison = Depth["test"];
        if(Comparison.is_string())
        {
            std::string Name;
            Comparison.get_to(Name);

            if(Name == "less_or_equal")
            {
                PipelineCreation.DepthStencil.DepthComparison = compareOperation::LessOrEqual;
            }
            else if(Name == "equal")
            {
                PipelineCreation.DepthStencil.DepthComparison = compareOperation::Equal;
            } 
            else if(Name == "never")
            {
                PipelineCreation.DepthStencil.DepthComparison = compareOperation::Never;
            } 
            else if(Name == "always")
            {
                PipelineCreation.DepthStencil.DepthComparison = compareOperation::Always;
            } 
            else
            {
                assert(false);
            }
        }
    }

    json BlendStates = PipelineJSON["blend"];
    if(!BlendStates.is_null())
    {
        for(sz i=0; i<BlendStates.size(); i++)
        {
            json Blend = BlendStates[i];

            std::string Enabled = Blend.value("enable", "");
            std::string SrcColour = Blend.value("src_colour", "");
            std::string DstColour = Blend.value("dst_colour", "");
            std::string BlendOp = Blend.value("op", "");

            blendState &BlendState = PipelineCreation.BlendState.AddBlendState();
            BlendState.BlendEnabled = Enabled == "true";
            BlendState.SetColor(GetBlendFactor(SrcColour), GetBlendFactor(DstColour), GetBlendOp(BlendOp));
        }
    }

    json Cull = PipelineJSON["cull"];
    if(Cull.is_string())
    {
        std::string Name;
        Cull.get_to(Name);
        if(Name== "back")
        {
            PipelineCreation.Rasterization.CullMode == cullMode::Back;
        }
        else
        {
            assert(false);
        }
    }

    //TODO: Other than swapchain
    json RenderPass = PipelineJSON["render_pass"];
    if(RenderPass.is_string())
    {
        std::string Name;
        RenderPass.get_to(Name);
        if(Name == "Swapchain")
        {
            PipelineCreation.RenderPass = context::Get()->SwapchainOutput;
        }
        else
        {
            // const renderPass *RenderPass = device::Get()->GetRenderPass(Node->_RenderPass);
            // PipelineCreation._RenderPass = RenderPass->_Output;
        }
    } 
    else
    {
        PipelineCreation.RenderPass = context::Get()->SwapchainOutput;
    }   
}


pipelineHandle context::CreatePipelineFromFile(const char *FileName)
{
    std::string PathStr(FileName);
    std::string ParentPath = std::filesystem::path(PathStr).parent_path().string();

    using json = nlohmann::json;
    std::string ReadResult = ReadFileString(FileName);
    json Data = json::parse(ReadResult.data());

    std::vector<pipelineCreation> PipelineCreations;

    json Name = Data["name"];
    std::string NameString;
    if(Name.is_string())
    {
        Name.get_to(NameString);
    } 

    json Pipelines = Data["pipelines"];
    if(Pipelines.is_array())
    {
        u32 Size = (u32)Pipelines.size();
        for(u32 i=0; i<Size; i++)
        {
            json Pipeline = Pipelines[i];
            pipelineCreation PipelineCreation{};
            // PipelineCreation.Name = TechniqueCreation.Name;
            PipelineCreation.Shaders.Reset();

            json InheritFrom = Pipeline["inherit_from"];
            if(InheritFrom.is_string())
            {
                std::string InheritedName;
                InheritFrom.get_to(InheritedName);

                for(u32 j=0; j<Size; j++)
                {
                    json PipelineJ = Pipelines[j];
                    std::string Name;
                    PipelineJ["name"].get_to(Name);
                    if(Name == InheritedName)
                    {
                        ParseGPUPipeline(PipelineJ, PipelineCreation, ParentPath);
                    }
                }
            }

            ParseGPUPipeline(Pipeline, PipelineCreation, ParentPath);
            PipelineCreations.push_back(PipelineCreation);
            WriteFileString("Shader", PipelineCreation.Shaders.Stages[0].Code);
        }
    }
    for(u32 i=0; i<PipelineCreations.size(); i++)
    {
        pipelineHandle pipeline = this->CreatePipeline(PipelineCreations[i]);
        return pipeline;
    }
}    



}