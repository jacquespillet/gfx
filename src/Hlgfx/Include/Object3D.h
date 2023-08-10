#pragma once

#include <memory>
#include "Types.h"
#include "Transform.h"
#include <vector>
#include <string>

namespace hlgfx
{
struct camera;
struct scene;

enum class object3DType
{
    Object3d,
    Mesh,
    Scene,
};

struct object3D
{
    object3D(std::string);
    object3D(const object3D &Other) = default;
    ~object3D();

    static bool AddToScene;
    
    virtual std::shared_ptr<object3D> Clone(b8 CloneUUID);
    
    void SetParent(std::shared_ptr<object3D> Parent);
    
    std::string Name;
    object3D *Parent;
    transform Transform;
    u32 RenderOrder=0;
    b8 FrustumCulled=false;
    b8 CastShadow=false;
    b8 ReceiveShadow=false;
    std::string UUID;
    std::vector<std::shared_ptr<object3D>> Children;

    
    virtual void Serialize(std::string FilePath);
    virtual void Serialize(std::ofstream &Stream);
    void SetRenderOrder(u32 RenderOrder);
    void SetFrustumCulled(b8 FrustumCulled);
    void SetCastShadow(b8 CastShadow);
    void SetReceiveShadow(b8 ReceiveShadow);

    virtual void AddObject(std::shared_ptr<object3D> Object);
    virtual void OnEarlyUpdate();
    virtual void OnUpdate();
    virtual void OnBeforeRender(std::shared_ptr<camera> Camera);
    virtual void OnRender(std::shared_ptr<camera> Camera);
    virtual void OnAfterRender(std::shared_ptr<camera> Camera);

    void DeleteChild(std::shared_ptr<object3D> Child);

    virtual void DrawGUI();
    virtual void DrawMaterial();


    static std::shared_ptr<object3D> Deserialize(const std::string &FileName);
    static std::shared_ptr<object3D> Deserialize(std::ifstream &FileStream);
};
}