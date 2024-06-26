#include "GLTF.h"

#include "../../Gfx/Include/Context.h"

#include <stb_image.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#pragma warning ( disable : 4996 )
#include <stb_image_write.h>
#pragma warning ( default : 4996 )

#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NO_INCLUDE_STB_IMAGE
#define TINYGLTF_NO_INCLUDE_STB_IMAGE_WRITE
#define TINYGLTF_NO_INCLUDE_STB_IMAGE_RESIZE
#include <tiny_gltf.h>

#include "../Include/Geometry.h"
#include "../Include/Material.h"
#include "../Include/Texture.h"
#include "../Include/Mesh.h"
#include "../Include/Util.h"
#include "../Include/Context.h"
#include "gfx/Include/Pipeline.h"


#include <glm/gtx/matrix_decompose.hpp>

namespace hlgfx
{
namespace loaders
{
namespace gltf
{

struct geometryData
{
    std::shared_ptr<indexedGeometryBuffers> Buffers;
    u32 MaterialIndex;
};

gfx::samplerFilter MinFilter(int Filter)
{
    if(Filter == TINYGLTF_TEXTURE_FILTER_LINEAR)
        return gfx::samplerFilter::Linear;
    else if(Filter == TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_NEAREST)
        return gfx::samplerFilter::NearestMipmapNearest;
    else if(Filter == TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_NEAREST)
        return gfx::samplerFilter::LinearMipmapNearest;
    else if(Filter == TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_LINEAR)
        return gfx::samplerFilter::NearestMipmapLinear;
    else if(Filter == TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR)
        return gfx::samplerFilter::LinearMipmapLinear;

    return gfx::samplerFilter::Linear;
}
gfx::samplerFilter MagFilter(int Filter)
{
    if(Filter == TINYGLTF_TEXTURE_FILTER_NEAREST)
        return gfx::samplerFilter::Nearest;
    else if(Filter == TINYGLTF_TEXTURE_FILTER_LINEAR)
        return gfx::samplerFilter::Linear;
    return gfx::samplerFilter::Linear;
}
gfx::samplerWrapMode BorderMode(int Mode)
{
    if(Mode == TINYGLTF_TEXTURE_WRAP_CLAMP_TO_EDGE)
        return gfx::samplerWrapMode::ClampToEdge;
    else if(Mode == TINYGLTF_TEXTURE_WRAP_MIRRORED_REPEAT)
        return gfx::samplerWrapMode::MirroredRepeat;
    else if(Mode == TINYGLTF_TEXTURE_WRAP_REPEAT)
        return gfx::samplerWrapMode::Repeat;

    return gfx::samplerWrapMode::Repeat;
}


void LoadGeometry(tinygltf::Model &GLTFModel, std::vector<std::shared_ptr<geometryData>> &Geometries, context::project &Project, std::vector<std::vector<uint32_t>> &InstanceMapping, u32 BaseMaterialIndex)
{
    uint32_t GIndexBase=0;
    InstanceMapping.resize(GLTFModel.meshes.size());

    int NoNameCount=0;
    for(int MeshIndex=0; MeshIndex<GLTFModel.meshes.size(); MeshIndex++)
    {
        tinygltf::Mesh gltfMesh = GLTFModel.meshes[MeshIndex];
        uint32_t BaseIndex = Geometries.size();
        Geometries.resize(BaseIndex + gltfMesh.primitives.size());
        InstanceMapping[MeshIndex].resize(gltfMesh.primitives.size());

        for(int j=0; j<gltfMesh.primitives.size(); j++)
        {
            tinygltf::Primitive GLTFPrimitive = gltfMesh.primitives[j];


            std::shared_ptr<geometryData> &Geometry = Geometries[BaseIndex + j];
            Geometry = std::make_shared<geometryData>();
            Geometry->MaterialIndex = BaseMaterialIndex + GLTFPrimitive.material;
            Geometry->Buffers = std::make_shared<indexedGeometryBuffers>();

            if(Project.Geometries.size() < Geometry->Buffers->ID + 1)
            {
                Project.Geometries.resize(Geometry->Buffers->ID + 1);
            }
            Project.Geometries[Geometry->Buffers->ID] = Geometry->Buffers;
            
            std::string GeometryName = gltfMesh.name;
            if(GeometryName == "")
            {
                GeometryName = "Geometry_" + std::to_string(NoNameCount++);
            }
            
            Geometry->Buffers->Name = GeometryName;
            
            InstanceMapping[MeshIndex][j] = BaseIndex + j;

            if(GLTFPrimitive.mode != TINYGLTF_MODE_TRIANGLES)
                continue;
            
            //Get the index of each needed attribute
            int IndicesIndex = GLTFPrimitive.indices;
            int PositionIndex = -1;
            int NormalIndex = -1;
            int TangentIndex = -1;
            int UVIndex=-1;
            if(GLTFPrimitive.attributes.count("POSITION") >0)
                PositionIndex = GLTFPrimitive.attributes["POSITION"];
            if(GLTFPrimitive.attributes.count("NORMAL") >0)
                NormalIndex = GLTFPrimitive.attributes["NORMAL"];
            if(GLTFPrimitive.attributes.count("TANGENT") >0)
                TangentIndex = GLTFPrimitive.attributes["TANGENT"];
            if(GLTFPrimitive.attributes.count("TEXCOORD_0") >0)
                UVIndex = GLTFPrimitive.attributes["TEXCOORD_0"];

            //Positions
            tinygltf::Accessor PositionAccessor = GLTFModel.accessors[PositionIndex];
            tinygltf::BufferView PositionBufferView = GLTFModel.bufferViews[PositionAccessor.bufferView];
            const tinygltf::Buffer &PositionBuffer = GLTFModel.buffers[PositionBufferView.buffer];
            const uint8_t *PositionBufferAddress = PositionBuffer.data.data();
            //3 * float
            int PositionStride = tinygltf::GetComponentSizeInBytes(PositionAccessor.componentType) * tinygltf::GetNumComponentsInType(PositionAccessor.type);
            if(PositionBufferView.byteStride > 0) PositionStride = (int)PositionBufferView.byteStride;

            //Normals
            tinygltf::Accessor NormalAccessor;
            tinygltf::BufferView NormalBufferView;
            const uint8_t *NormalBufferAddress=0;
            int NormalStride=0;
            if(NormalIndex >= 0)
            {
                NormalAccessor = GLTFModel.accessors[NormalIndex];
                NormalBufferView = GLTFModel.bufferViews[NormalAccessor.bufferView];
                const tinygltf::Buffer &normalBuffer = GLTFModel.buffers[NormalBufferView.buffer];
                NormalBufferAddress = normalBuffer.data.data();
                //3 * float
                NormalStride = tinygltf::GetComponentSizeInBytes(NormalAccessor.componentType) * tinygltf::GetNumComponentsInType(NormalAccessor.type);
                if(NormalBufferView.byteStride > 0) NormalStride =(int) NormalBufferView.byteStride;
            }

            //Tangents
            tinygltf::Accessor TangentAccessor;
            tinygltf::BufferView TangentBufferView;
            const uint8_t *TangentBufferAddress=0;
            int TangentStride=0;
            if(TangentIndex >= 0)
            {
                TangentAccessor = GLTFModel.accessors[TangentIndex];
                TangentBufferView = GLTFModel.bufferViews[TangentAccessor.bufferView];
                const tinygltf::Buffer &TangentBuffer = GLTFModel.buffers[TangentBufferView.buffer];
                TangentBufferAddress = TangentBuffer.data.data();
                //3 * float
                TangentStride = tinygltf::GetComponentSizeInBytes(TangentAccessor.componentType) * tinygltf::GetNumComponentsInType(TangentAccessor.type);
                if(TangentBufferView.byteStride > 0) TangentStride =(int) TangentBufferView.byteStride;
            }
    
            //UV
            tinygltf::Accessor UVAccessor;
            tinygltf::BufferView UVBufferView;
            const uint8_t *UVBufferAddress=0;
            int UVStride=0;
            if(UVIndex >= 0)
            {
                UVAccessor = GLTFModel.accessors[UVIndex];
                UVBufferView = GLTFModel.bufferViews[UVAccessor.bufferView];
                const tinygltf::Buffer &uvBuffer = GLTFModel.buffers[UVBufferView.buffer];
                UVBufferAddress = uvBuffer.data.data();
                //3 * float
                UVStride = tinygltf::GetComponentSizeInBytes(UVAccessor.componentType) * tinygltf::GetNumComponentsInType(UVAccessor.type);
                if(UVBufferView.byteStride > 0) UVStride = (int)UVBufferView.byteStride;
            }

            //Indices
            tinygltf::Accessor IndicesAccessor = GLTFModel.accessors[IndicesIndex];
            tinygltf::BufferView IndicesBufferView = GLTFModel.bufferViews[IndicesAccessor.bufferView];
            const tinygltf::Buffer &IndicesBuffer = GLTFModel.buffers[IndicesBufferView.buffer];
            const uint8_t *IndicesBufferAddress = IndicesBuffer.data.data();
            int IndicesStride = tinygltf::GetComponentSizeInBytes(IndicesAccessor.componentType) * tinygltf::GetNumComponentsInType(IndicesAccessor.type); 

            Geometry->Buffers->VertexData.resize(PositionAccessor.count);
            for (size_t k = 0; k < PositionAccessor.count; k++)
            {
                v3f Position(0);
                v3f Normal(0);
                v4f Tangent(0);
                v2f UV(0);

                {
                    const uint8_t *address = PositionBufferAddress + PositionBufferView.byteOffset + PositionAccessor.byteOffset + (k * PositionStride);
                    memcpy(&Position, address, 12);
                }

                if(NormalIndex>=0)
                {
                    const uint8_t *address = NormalBufferAddress + NormalBufferView.byteOffset + NormalAccessor.byteOffset + (k * NormalStride);
                    memcpy(&Normal, address, 12);
                }

                if(TangentIndex>=0)
                {
                    const uint8_t *address = TangentBufferAddress + TangentBufferView.byteOffset + TangentAccessor.byteOffset + (k * TangentStride);
                    memcpy(&Tangent, address, 16);
                }

                if(UVIndex>=0)
                {
                    const uint8_t *address = UVBufferAddress + UVBufferView.byteOffset + UVAccessor.byteOffset + (k * UVStride);
                    memcpy(&UV, address, 8);
                }

                Geometry->Buffers->VertexData[k] =
                {
                    v4f(Position.x, Position.y, Position.z, UV.x),
                    v4f(Normal.x, Normal.y, Normal.z, UV.y),
                    Tangent
                };
            }


            //Fill indices buffer
            Geometry->Buffers->IndexData.resize(IndicesAccessor.count);
            const uint8_t *baseAddress = IndicesBufferAddress + IndicesBufferView.byteOffset + IndicesAccessor.byteOffset;
            if(IndicesStride == 1)
            {
                std::vector<uint8_t> Quarter;
                Quarter.resize(IndicesAccessor.count);
                memcpy(Quarter.data(), baseAddress, (IndicesAccessor.count) * IndicesStride);
                for(size_t i=0; i<IndicesAccessor.count; i+=3)
                {
                    Geometry->Buffers->IndexData[i] = Quarter[i];
                }
            }
            else if(IndicesStride == 2)
            {
                std::vector<uint16_t> Half;
                Half.resize(IndicesAccessor.count);
                memcpy(Half.data(), baseAddress, (IndicesAccessor.count) * IndicesStride);
                for(size_t i=0; i<IndicesAccessor.count; i++)
                {
                    Geometry->Buffers->IndexData[i] = Half[i];
                }
            }
            else
            {
                memcpy(Geometry->Buffers->IndexData.data(), baseAddress, (IndicesAccessor.count) * IndicesStride);
            }

            if(TangentIndex<0)
            {
                CalculateTangents(Geometry->Buffers->VertexData, Geometry->Buffers->IndexData);
            }

            Geometry->Buffers->BuildBuffers();
        }
    }
}    

void TraverseNodes(tinygltf::Model &GLTFModel, uint32_t nodeIndex, std::vector<std::shared_ptr<geometryData>> &Geometries, std::vector<std::vector<uint32_t>> &InstanceMapping, context::project &Project, std::shared_ptr<object3D> Parent)
{
    tinygltf::Node GLTFNode = GLTFModel.nodes[nodeIndex];

    std::string NodeName = GLTFNode.name;
    if(NodeName.compare("") == 0)
    {
        NodeName = "Node";
    }
    std::shared_ptr<object3D> Node = std::make_shared<object3D>(NodeName.c_str());

    if(Project.Objects.size() < Node->ID + 1)
    {
        Project.Objects.resize(Node->ID + 1);
    }
    Project.Objects[Node->ID] = Node;

    if(GLTFNode.matrix.size() > 0)
    {
        m4x4 Matrix;
        Matrix[0][0] = (float)GLTFNode.matrix[0]; Matrix[0][1] = (float)GLTFNode.matrix[1]; Matrix[0][2] = (float)GLTFNode.matrix[2]; Matrix[0][3] = (float)GLTFNode.matrix[3];
        Matrix[1][0] = (float)GLTFNode.matrix[4]; Matrix[1][1] = (float)GLTFNode.matrix[5]; Matrix[1][2] = (float)GLTFNode.matrix[6]; Matrix[1][3] = (float)GLTFNode.matrix[7];
        Matrix[2][0] = (float)GLTFNode.matrix[8]; Matrix[2][1] = (float)GLTFNode.matrix[9]; Matrix[2][2] = (float)GLTFNode.matrix[10]; Matrix[2][3] = (float)GLTFNode.matrix[11];
        Matrix[3][0] = (float)GLTFNode.matrix[12]; Matrix[3][1] = (float)GLTFNode.matrix[13]; Matrix[3][2] = (float)GLTFNode.matrix[14]; Matrix[3][3] = (float)GLTFNode.matrix[15];
            
        glm::vec3 Scale;
        glm::quat Rotation;
        glm::vec3 Translation;
        glm::vec3 Skew;
        glm::vec4 Perspective;
        glm::decompose(Matrix, Scale, Rotation, Translation, Skew, Perspective);    
        Node->Transform.SetLocalPosition(Translation);
        Node->Transform.SetLocalRotation(glm::eulerAngles(Rotation));
        Node->Transform.SetLocalScale(Scale);
    }
    else
    {
        glm::mat4 translate, rotation, scale;
        if(GLTFNode.translation.size()>0)
        {
            Node->Transform.SetLocalPosition(v3f(GLTFNode.translation[0], GLTFNode.translation[1], GLTFNode.translation[2]));
        }

        if(GLTFNode.rotation.size() > 0)
        {
            glm::quat Quat((float)GLTFNode.rotation[3], (float)GLTFNode.rotation[0], (float)GLTFNode.rotation[1], (float)GLTFNode.rotation[2]);
            v3f EulerAngles = glm::eulerAngles(Quat);
            Node->Transform.SetLocalRotation(EulerAngles);
        }

        if(GLTFNode.scale.size() > 0)
        {
            Node->Transform.SetLocalScale(v3f(GLTFNode.scale[0], GLTFNode.scale[1], GLTFNode.scale[2]));
        }
    }

    //Leaf node
    if(GLTFNode.children.size() == 0 && GLTFNode.mesh != -1)
    {
        tinygltf::Mesh GLTFMesh = GLTFModel.meshes[GLTFNode.mesh];
        for(int i=0; i<GLTFMesh.primitives.size(); i++)
        {
            std::shared_ptr<mesh> Mesh = std::make_shared<mesh>();

            if(Project.Objects.size() < Mesh->ID + 1)
            {
                Project.Objects.resize(Mesh->ID + 1);
            }            
            Project.Objects[Mesh->ID] = Mesh;
            // Mesh->Transform.SetLocalPosition(0,0,0);

            uint32_t Inx = InstanceMapping[GLTFNode.mesh][i];
            Mesh->GeometryID = Geometries[Inx]->Buffers->ID;
            Mesh->Name = GLTFMesh.name;
            Mesh->MaterialID = Project.Materials[Geometries[Inx]->MaterialIndex]->ID;

            if(strcmp(Mesh->Name.c_str(), "") == 0)
            {
                Mesh->Name = "Mesh" + std::to_string(GLTFNode.mesh) + "_" + std::to_string(i);
            }
            Node->AddObject(Mesh);
        }   
    }

    Parent->AddObject(Node);

    for (size_t i = 0; i < GLTFNode.children.size(); i++)
    {
        TraverseNodes(GLTFModel, GLTFNode.children[i], Geometries, InstanceMapping, Project, Node);
    }
    

}

void LoadInstances(tinygltf::Model &GLTFModel, std::vector<std::shared_ptr<geometryData>> &Geometries, std::vector<std::vector<uint32_t>> &InstanceMapping, context::project &Project, std::shared_ptr<object3D> Root)
{
    const tinygltf::Scene GLTFScene = GLTFModel.scenes[GLTFModel.defaultScene];
    for (size_t i = 0; i < GLTFScene.nodes.size(); i++)
    {
        TraverseNodes(GLTFModel, GLTFScene.nodes[i], Geometries, InstanceMapping, Project, Root);
    }
}

std::string ExtractFileName(const std::string& Path) {
    // Find the position of the last directory separator
    size_t LastSlashPos = Path.find_last_of("/\\");
    
    // Extract the file name
    if (LastSlashPos != std::string::npos) {
        return Path.substr(LastSlashPos + 1);
    } else {
        // No directory separator found, return the entire Path
        return Path;
    }
}


void LoadTextures(tinygltf::Model &GLTFModel, context::project &Project)
{
    for (size_t i = 0; i < GLTFModel.textures.size(); i++)
    {
        tinygltf::Texture& GLTFTex = GLTFModel.textures[i];
        tinygltf::Image GLTFImage = GLTFModel.images[GLTFTex.source];
        std::string TexName = GLTFTex.name;
        if(strcmp(GLTFTex.name.c_str(), "") == 0)
        {
            TexName = ExtractFileName(GLTFImage.uri);
        }
        
        assert(GLTFImage.component==4);
        assert(GLTFImage.bits==8);
        
        gfx::imageData ImageData = {};
        ImageData.ChannelCount = GLTFImage.component;
        ImageData.Data = GLTFImage.image.data();
        ImageData.DataSize = GLTFImage.image.size();
        ImageData.Width = GLTFImage.width;
        ImageData.Height = GLTFImage.height;
        ImageData.Format = gfx::format::R8G8B8A8_UNORM;
        ImageData.Type = gfx::type::UNSIGNED_BYTE;

		gfx::imageCreateInfo ImageCreateInfo = 
		    {
			    {0.0f,0.0f,0.0f,0.0f},
                gfx::samplerFilter::Linear,
                gfx::samplerFilter::Linear,
                gfx::samplerWrapMode::Repeat,
                gfx::samplerWrapMode::Repeat,
			    gfx::samplerWrapMode::Repeat,
			    true
		    };
        if (GLTFTex.sampler >= 0)
        {
            tinygltf::Sampler &Sampler = GLTFModel.samplers[GLTFTex.sampler];
            ImageCreateInfo =
            {
                {0.0f,0.0f,0.0f,0.0f},
                MinFilter(Sampler.minFilter),
                MagFilter(Sampler.magFilter),
                BorderMode(Sampler.wrapS),
                BorderMode(Sampler.wrapT),
                gfx::samplerWrapMode::Repeat,
                true
            };
        }
		gfx::imageHandle Image = gfx::context::Get()->CreateImage(ImageData, ImageCreateInfo);
        std::shared_ptr<texture> ImagePtr = std::make_shared<texture>(TexName, Image);

        if(Project.Textures.size() < ImagePtr->ID + 1)
        {
            Project.Textures.resize(ImagePtr->ID + 1);
        }     
        Project.Textures[ImagePtr->ID] = ImagePtr;
    }
}

void LoadMaterials(tinygltf::Model &GLTFModel, context::project &Project, u32 BaseTextureID)
{
    
    int NoNameCount=0;
    for (size_t i = 0; i < GLTFModel.materials.size(); i++)
    {
        const tinygltf::Material GLTFMaterial = GLTFModel.materials[i];
        const tinygltf::PbrMetallicRoughness PBR = GLTFMaterial.pbrMetallicRoughness;

        
        materialFlags::bits Flags = (materialFlags::bits)(materialFlags::DepthWriteEnabled | materialFlags::DepthTestEnabled);
        if(GLTFMaterial.alphaMode == "BLEND")
            Flags = (materialFlags::bits)(Flags |  materialFlags::BlendEnabled);
        if(!GLTFMaterial.doubleSided)
            Flags = (materialFlags::bits)(Flags |  materialFlags::CullModeOn);
        
        Flags = (materialFlags::bits)(Flags |  materialFlags::PBR);
        
        std::string MatName = GLTFMaterial.name;
        if(MatName == "")
        {
            MatName = "Material_" + std::to_string(NoNameCount++);
        }
        
        std::shared_ptr<material> Material = std::make_shared<pbrMaterial>(MatName, Flags);  
        std::shared_ptr<pbrMaterial> PBRMat = std::static_pointer_cast<pbrMaterial>(Material);
        
        if(Project.Materials.size() < Material->ID + 1)
        {
            Project.Materials.resize(Material->ID + 1);
        }     
        Project.Materials[Material->ID] = Material;
        
        PBRMat->UniformData.BaseColorFactor = v3f(PBR.baseColorFactor[0], PBR.baseColorFactor[1], PBR.baseColorFactor[2]);
        PBRMat->UniformData.OpacityFactor = PBR.baseColorFactor[3];
        PBRMat->UniformData.RoughnessFactor = PBR.roughnessFactor;
        PBRMat->UniformData.MetallicFactor = PBR.metallicFactor;
        PBRMat->UniformData.Emission = v3f(GLTFMaterial.emissiveFactor[0], GLTFMaterial.emissiveFactor[1], GLTFMaterial.emissiveFactor[2]);
        PBRMat->UniformData.EmissiveFactor = 1.0f;
        PBRMat->UniformData.AlphaCutoff = GLTFMaterial.alphaCutoff;
        PBRMat->UniformData.OcclusionStrength = 1.0f;
        PBRMat->UniformData.UseBaseColor = 0.0f;
        PBRMat->UniformData.UseEmissionTexture = 0.0f;
        PBRMat->UniformData.UseMetallicRoughnessTexture = 0.0f;
        PBRMat->UniformData.UseNormalTexture = 0.0f;
        PBRMat->UniformData.UseOcclusionTexture = 0.0f;
 
        if(PBR.baseColorTexture.index > -1)
        {
            PBRMat->SetBaseColorTexture(Project.Textures[BaseTextureID + PBR.baseColorTexture.index]);
            PBRMat->UniformData.UseBaseColor = 1.0f;
        }
        if(PBR.metallicRoughnessTexture.index > -1)
        {
            PBRMat->SetMetallicRoughnessTexture(Project.Textures[BaseTextureID + PBR.metallicRoughnessTexture.index]);
            PBRMat->UniformData.UseMetallicRoughnessTexture = 1.0f;
        }
        if(GLTFMaterial.occlusionTexture.index > -1)
        {
            PBRMat->SetOcclusionTexture(Project.Textures[BaseTextureID + GLTFMaterial.occlusionTexture.index]);
            PBRMat->UniformData.UseOcclusionTexture = 1.0f;
        }
        if(GLTFMaterial.normalTexture.index > -1)
        {
            PBRMat->SetNormalTexture(Project.Textures[BaseTextureID + GLTFMaterial.normalTexture.index]);
            PBRMat->UniformData.UseNormalTexture = 1.0f;
        }
        if(GLTFMaterial.emissiveTexture.index > -1)
        {
            PBRMat->SetEmissiveTexture(Project.Textures[BaseTextureID + GLTFMaterial.emissiveTexture.index]);
            PBRMat->UniformData.UseEmissionTexture = 1.0f;
        }

        PBRMat->Update();

    }
}    

std::shared_ptr<object3D> Load(std::string FileName)
{
    std::shared_ptr<object3D> Result;

    tinygltf::Model GLTFModel;
    tinygltf::TinyGLTF ModelLoader;

    std::string Error, Warning;

    std::string Extension = FileName.substr(FileName.find_last_of(".") + 1);
    bool OK = false;
    if(Extension == "gltf")
    {
        OK = ModelLoader.LoadASCIIFromFile(&GLTFModel, &Error, &Warning, FileName);
    }
    else if(Extension == "glb")
    {
        OK = ModelLoader.LoadBinaryFromFile(&GLTFModel, &Error, &Warning, FileName);
    }
    else OK=false;
        
    if(!OK) 
    {
        printf("Could not load model %s \n",FileName);
        return Result;
    }

    Result = std::make_shared<object3D>(FileNameFromPath(FileName));
    
    std::vector<std::vector<uint32_t>> InstanceMapping;
    std::vector<std::shared_ptr<geometryData>> Geometries;
    std::vector<std::shared_ptr<texture>> Textures;
    std::vector<std::shared_ptr<material>> Materials;
    
    context::project &Project = context::Get()->Project;
    u32 BaseTextureID = Project.Textures.size();
    u32 BaseGeometryID = Project.Geometries.size();
    u32 BaseMaterialID = Project.Materials.size();
    u32 BaseObjectID = Project.Objects.size();

    LoadTextures(GLTFModel, Project);
    LoadMaterials(GLTFModel, Project, BaseTextureID);
    LoadGeometry(GLTFModel, Geometries, Project, InstanceMapping, BaseMaterialID);
    LoadInstances(GLTFModel, Geometries, InstanceMapping, Project, Result);


    

    return Result;
}

}
}
}