#include "Assimp.h"
#include "../Include/Geometry.h"
#include "../Include/Material.h"
#include "../Include/Object3D.h"
#include "../Include/Mesh.h"
#include "../Include/Util.h"

#include "Gfx/Include/Image.h"
#include <assimp/Importer.hpp>
#include <assimp/Scene.h>
#include <assimp/postprocess.h>
#include <glm/gtx/matrix_decompose.hpp>

namespace hlgfx
{

namespace loaders
{
namespace assimp
{
struct geometryData
{
    std::shared_ptr<indexedGeometryBuffers> Buffers;
    u32 MaterialIndex;
};

void LoadTextures(std::string Path, const aiScene *Scene, std::unordered_map<std::string, std::shared_ptr<texture>> &Textures)
{
    for (sz i = 0; i < Scene->mNumTextures; i++)
    {
        aiTexture *aTexture = Scene->mTextures[i];
        std::string FullPath = Path + "/" + std::string(aTexture->mFilename.C_Str());
        std::shared_ptr<texture> Texture = std::make_shared<texture>(FileNameFromPath(std::string(aTexture->mFilename.C_Str())));

        gfx::imageData ImageData = gfx::ImageFromFile((char*)FullPath.c_str());
		gfx::imageCreateInfo ImageCreateInfo = 
		{
			{0.0f,0.0f,0.0f,0.0f},
			gfx::samplerFilter::Linear,
			gfx::samplerFilter::Linear,
			gfx::samplerWrapMode::ClampToBorder,
			gfx::samplerWrapMode::ClampToBorder,
			gfx::samplerWrapMode::ClampToBorder,
			true
		};
		Texture->Handle = gfx::context::Get()->CreateImage(ImageData, ImageCreateInfo);
        Textures[aTexture->mFilename.C_Str()] = Texture;
    }
}

std::shared_ptr<texture> GetTexture(std::string FileName, std::string Name)
{
    std::shared_ptr<texture> Texture = std::make_shared<texture>(Name);
    gfx::imageData ImageData = gfx::ImageFromFile((char*)FileName.c_str());
    gfx::imageCreateInfo ImageCreateInfo = 
    {
        {0.0f,0.0f,0.0f,0.0f},
        gfx::samplerFilter::Linear,
        gfx::samplerFilter::Linear,
        gfx::samplerWrapMode::ClampToBorder,
        gfx::samplerWrapMode::ClampToBorder,
        gfx::samplerWrapMode::ClampToBorder,
        true
    };
    Texture->Handle = gfx::context::Get()->CreateImage(ImageData, ImageCreateInfo);
    return Texture;
}

void LoadMaterials(std::string Path, const aiScene *Scene, std::vector<std::shared_ptr<material>> &Materials, std::unordered_map<std::string, std::shared_ptr<texture>> &Textures)
{
    for (sz i = 0; i < Scene->mNumMaterials; i++)
    {
        aiMaterial *aMaterial = Scene->mMaterials[i];

        materialFlags::bits Flags = (materialFlags::bits)(materialFlags::DepthWriteEnabled | materialFlags::DepthTestEnabled | materialFlags::PBR);
        
        float OpacityFactor;
        aMaterial->Get(AI_MATKEY_OPACITY, OpacityFactor);
        if(OpacityFactor < 0.99) Flags = (materialFlags::bits)(Flags | materialFlags::BlendEnabled);
        
        std::shared_ptr<pbrMaterial> Material = std::make_shared<pbrMaterial>(aMaterial->GetName().C_Str(), Flags);

        b8 PBR = false;

        f32 RoughnessFactor = -1;
        aiColor3D BaseColor(0,0,0);
        aiColor3D Emission;
        f32 MetallicFactor = -1;
        f32 EmissiveFactor = -1;

        aMaterial->Get(AI_MATKEY_ROUGHNESS_FACTOR, RoughnessFactor);
        if(RoughnessFactor > 0) PBR = true;

        if(PBR)
        {

        }
        else
        {
            aMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, BaseColor);
            aMaterial->Get(AI_MATKEY_COLOR_EMISSIVE, Emission);

            //Diffuse Color
            {
                aiString TexName("");
                aMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &TexName);
                if(std::string(TexName.C_Str()) != "")
                {
                    if(Textures.find(TexName.C_Str()) == Textures.end())
                    {
                        std::string FullPath = Path + "/" + std::string(TexName.C_Str());
                        Textures[TexName.C_Str()] = GetTexture(FullPath, FileNameFromPath(std::string(TexName.C_Str())));            
                    }
                    Material->SetBaseColorTexture(Textures[TexName.C_Str()]);            
                }
            }

            // //Normal map
            // {
            //     aiString TexName("");
            //     aMaterial->GetTexture(aiTextureType_NORMALS, 0, &TexName);
            //     if(std::string(TexName.C_Str()) == "")
            //         aMaterial->GetTexture(aiTextureType_HEIGHT, 0, &TexName);
            //     if(std::string(TexName.C_Str()) != "")
            //     {
            //         if(Textures.find(TexName.C_Str()) == Textures.end())
            //         {
            //             std::string FullPath = Path + "/" + std::string(TexName.C_Str());
            //             Textures[TexName.C_Str()] = GetTexture(FullPath, FileNameFromPath(std::string(TexName.C_Str())));            
            //         }
            //         Material->SetNormalTexture(Textures[TexName.C_Str()]);            
            //     }
            // }
        }

        
        Material->UniformData.BaseColorFactor = v3f(BaseColor.r, BaseColor.g, BaseColor.b);
        Material->UniformData.OpacityFactor = OpacityFactor;
        Material->UniformData.RoughnessFactor = RoughnessFactor > 0 ? RoughnessFactor : 1;
        Material->UniformData.MetallicFactor = MetallicFactor > 0 ? MetallicFactor : 0;
        Material->UniformData.EmissiveFactor = EmissiveFactor > 0 ? EmissiveFactor : 0;
        Material->UniformData.Emission = v3f(Emission.r, Emission.g, Emission.b);
        Material->UniformData.OcclusionStrength = 1.0f;
        Material->UniformData.AlphaCutoff = 0.0f;
        
        

        Materials.push_back(Material);
    }
}

void LoadGeometry(const aiScene *Scene, std::vector<std::shared_ptr<geometryData>> &Geometries)
{
    Geometries.resize(Scene->mNumMeshes);
    for (sz i = 0; i < Scene->mNumMeshes; i++)
    {
        aiMesh *aMesh = Scene->mMeshes[i];

        std::shared_ptr<geometryData> &Geometry = Geometries[i];
        Geometry = std::make_shared<geometryData>();
        Geometry->MaterialIndex = aMesh->mMaterialIndex;
        Geometry->Buffers = std::make_shared<indexedGeometryBuffers>();
        Geometry->Buffers->Name = aMesh->mName.C_Str();

        for (sz j = 0; j < aMesh->mNumVertices; j++)
        {
            aiVector3D Position = aMesh->mVertices[j];
            aiVector3D Normal = aMesh->mNormals[j];
            aiVector3D UV = aMesh->mTextureCoords[0][j];

            Geometry->Buffers->VertexData.push_back({
                v4f(Position.x, Position.y, Position.z, UV.x),    
                v4f(Normal.x, Normal.y, Normal.z, UV.y),    
            });
        }

        for (sz FaceInx = 0; FaceInx < aMesh->mNumFaces; ++FaceInx) {
            aiFace& Face = aMesh->mFaces[FaceInx];
            for (sz VertexIndex = 0; VertexIndex < Face.mNumIndices; ++VertexIndex) {
                Geometry->Buffers->IndexData.push_back(Face.mIndices[VertexIndex]);
            }
        }        
        
        Geometry->Buffers->BuildBuffers();
    }   
}

void ProcessNode(const aiNode *aNode, std::shared_ptr<object3D> Parent, const aiScene *Scene, std::vector<std::shared_ptr<geometryData>> &Geometries, std::vector<std::shared_ptr<material>> &Materials)
{
    std::string NodeName = aNode->mName.C_Str();
    if(NodeName.compare("") == 0) NodeName = "Node";
    std::shared_ptr<object3D> Node = std::make_shared<object3D>(NodeName.c_str());

    //Transform
    m4x4 Matrix;
    Matrix[0][0] = (float)aNode->mTransformation.a1; Matrix[0][1] = (float)aNode->mTransformation.a2; Matrix[0][2] = (float)aNode->mTransformation.a3; Matrix[0][3] = (float)aNode->mTransformation.a4;
    Matrix[1][0] = (float)aNode->mTransformation.b1; Matrix[1][1] = (float)aNode->mTransformation.b2; Matrix[1][2] = (float)aNode->mTransformation.b3; Matrix[1][3] = (float)aNode->mTransformation.b4;
    Matrix[2][0] = (float)aNode->mTransformation.c1; Matrix[2][1] = (float)aNode->mTransformation.c2; Matrix[2][2] = (float)aNode->mTransformation.c3; Matrix[2][3] = (float)aNode->mTransformation.c4;
    Matrix[3][0] = (float)aNode->mTransformation.d1; Matrix[3][1] = (float)aNode->mTransformation.d2; Matrix[3][2] = (float)aNode->mTransformation.d3; Matrix[3][3] = (float)aNode->mTransformation.d4;
    glm::vec3 Scale;
    glm::quat Rotation;
    glm::vec3 Translation;
    glm::vec3 Skew;
    glm::vec4 Perspective;
    glm::decompose(Matrix, Scale, Rotation, Translation, Skew, Perspective);    
    Node->Transform.SetLocalPosition(Translation);
    Node->Transform.SetLocalRotation(glm::eulerAngles(Rotation));
    Node->Transform.SetLocalScale(Scale);

    //Meshes
    for (s16 i = 0; i < aNode->mNumMeshes; i++)
    {
        std::shared_ptr<mesh> Mesh = std::make_shared<mesh>();
        Mesh->GeometryBuffers = Geometries[aNode->mMeshes[i]]->Buffers;
        Mesh->Material = Materials[Geometries[aNode->mMeshes[i]]->MaterialIndex];
        Mesh->Name = Scene->mMeshes[aNode->mMeshes[i]]->mName.C_Str();
        if(Mesh->Name.compare("")==0)
        {
            Mesh->Name = "Mesh " + Mesh->GeometryBuffers->Name + " " + std::to_string(i);
        }
        Node->AddObject(Mesh);
    }
    
    Parent->AddObject(Node);

    for (sz i = 0; i < aNode->mNumChildren; i++)
    {
        ProcessNode(aNode->mChildren[i], Node, Scene, Geometries, Materials);
    }
    
}

void LoadInstances(const aiNode *RootNode, std::shared_ptr<object3D> Root, const aiScene *Scene, std::vector<std::shared_ptr<geometryData>> &Geometries, std::vector<std::shared_ptr<material>> &Materials)
{
    for (sz i = 0; i < RootNode->mNumChildren; ++i) {
        ProcessNode(RootNode->mChildren[i], Root, Scene, Geometries, Materials);
    }    
}

std::shared_ptr<object3D> Load(std::string FileName)
{
    std::shared_ptr<object3D> Result;

    Assimp::Importer Importer;  
    const aiScene* Scene = Importer.ReadFile(FileName,
                                            aiProcess_Triangulate |
                                            aiProcess_GenUVCoords |
                                            aiProcess_GenNormals |
                                            aiProcess_FlipUVs |
                                            aiProcess_CalcTangentSpace);

    if (!Scene || Scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !Scene->mRootNode) {
        printf("Could not load model %s\n", FileName);
        return Result;
    }    

    Result = std::make_shared<object3D>(FileNameFromPath(FileName));

    std::vector<std::vector<uint32_t>> InstanceMapping;
    std::vector<std::shared_ptr<geometryData>> Geometries;
    std::unordered_map<std::string, std::shared_ptr<texture>> Textures;
    std::vector<std::shared_ptr<material>> Materials;

    std::string Path = PathFromFile(FileName);

    LoadTextures(Path, Scene, Textures);
    LoadMaterials(Path, Scene, Materials, Textures);
    LoadGeometry(Scene, Geometries);
    LoadInstances(Scene->mRootNode, Result, Scene, Geometries, Materials);

    Importer.FreeScene();
    return Result;
}

}
}
}
