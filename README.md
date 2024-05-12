# GFX

This is a simple graphics library for creating 3D apps.

It features a "low level" graphics api abstraction layer called Gfx, supporting 4 backends : 

* OpenGL

* Vulkan

* Direct3D 11

* Direct3D 12

It also features a "high level" api that is implemented on top of the low level api, called Hlgfx, that contains common abstractions for creating 3d apps, a bit like Three.js (Camera, 3D object, Meshes, Scene graph...). 

This high level api is used to create a simple 3d engine as an example.

It's not a finished product, there are still some things to add to the libraries, but most of the system is working.


# build instructions

## Requirements : 

    Visual Studio (Tested only on Visual Studio 2019)

    Latest [dxc](https://github.com/microsoft/DirectXShaderCompiler/releases) release copied into the vendor/dxc folder

## Commands : 
```
### Clone the repo and checkout to the latest branch
git clone --recursive https://github.com/jacquespillet/gfx.git
cd gfx

### Generate the solution
mkdir build
cd build
cmake ../

### Build
cd ..
Build.bat


```
First build may take a while because it's going to build all the dependencies with the project.


# Low level api features 

## Example Code

This is a very simple example usage of the Gfx api, to create a simple app that displays a textured triangle with a colour tint passed as a uniform variable :
[Here](https://raw.githubusercontent.com/jacquespillet/gfx/master/src/Main_HelloTriangle.cpp)'s the full code for that example.

We first initialize the graphics context and the swapchain : 

```cpp
// Initialize the graphics API
gfx::context::initializeInfo ContextInitialize;
ContextInitialize.Extensions = Window->GetRequiredExtensions();
ContextInitialize.ErrorCallback = ErrorCallback;
ContextInitialize.InfoCallback = InfoCallback;
ContextInitialize.Debug = true;
GfxContext = gfx::context::Initialize(ContextInitialize, *Window);
Swapchain = GfxContext->CreateSwapchain(Width, Height);
```

We then create the texture for the triangle : 

```cpp
gfx::imageData ImageData = gfx::ImageFromFile("resources/Textures/Debug.jpg");
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
TextureHandle1 = GfxContext->CreateImage(ImageData, ImageCreateInfo);
gfx::image *Texture1 = GfxContext->GetImage(TextureHandle1);
```

We then create the vertex stream description, and the vertex buffers for the triangle. 
A vertex here is defined by a 3d position, and a rgba colour.

```cpp
float vertices[] =
{
    0.0f, 0.25f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
    0.25f, -0.25f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
    -0.25f, -0.25f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f
};
gfx::vertexStreamData VertexStream1 = {};
VertexStream1
    .SetSize(sizeof(vertices))
    .SetStride(7 * sizeof(float))
    .SetData(&vertices)
    .SetStreamIndex(0)
    .AddAttribute({sizeof(float), 3, gfx::vertexAttributeType::Float, false, gfx::attributeSemantic::POSITION, 0, 0})
    .AddAttribute({sizeof(float), 4, gfx::vertexAttributeType::Float, false, gfx::attributeSemantic::COLOR, 0, 1});
gfx::vertexBufferCreateInfo VertexBufferCreateInfo = {};
VertexBufferCreateInfo.Init()
                        .AddVertexStream(VertexStream1);
VertexBufferHandle = GfxContext->CreateVertexBuffer(VertexBufferCreateInfo);
```

We then create a uniform buffer for storing the colour of the triangle : 

```cpp
UniformBufferHandle1 = GfxContext->CreateBuffer(sizeof(uniformData), gfx::bufferUsage::UniformBuffer, gfx::memoryUsage::CpuToGpu);
gfx::buffer *UniformBuffer1 = GfxContext->GetBuffer(UniformBufferHandle1);
UniformBuffer1->CopyData((uint8_t*)&UniformData1, sizeof(uniformData), 0);
```

We can now create a "uniform group" that contains the texture and the uniform buffer : 

```cpp
Uniforms = std::make_shared<gfx::uniformGroup>();
Uniforms->Reset()
        .AddUniformBuffer(0, UniformBufferHandle1)
        .AddTexture(4, TextureHandle1);
```

With all that, we can create a rendering pipeline, and bind this uniform group to that rendering pipeline.
We describe pipelines with json files that contain all the needed informations for creating one : 
* Shader Code

* Vertex Streams

* Depth Functions

* Blend Functions

* Triangle culling

```cpp
PipelineHandleSwapchain = GfxContext->CreatePipelineFromFile("resources/Shaders/Triangle/Triangle.json");
GfxContext->BindUniformsToPipeline(Uniforms, PipelineHandleSwapchain, 0);
Uniforms->Update();
```

And that's all for the initialization. Now, here's the code for the main render loop : 

```cpp
GfxContext->StartFrame();

// Begin recording commands into the command buffer
std::shared_ptr<gfx::commandBuffer> CommandBuffer = GfxContext->GetCurrentFrameCommandBuffer();
CommandBuffer->Begin();

CommandBuffer->BeginPass(GfxContext->GetSwapchainFramebuffer(), {0.5f, 0.0f, 0.8f, 1.0f}, {1.0f, 0});
CommandBuffer->SetViewport(0.0f, 0.0f, (float)Width, (float)Height);
CommandBuffer->SetScissor(0, 0, Width, Height);

CommandBuffer->BindGraphicsPipeline(PipelineHandleSwapchain);

CommandBuffer->BindUniformGroup(Uniforms, 0);

CommandBuffer->BindVertexBuffer(VertexBufferHandle);
CommandBuffer->DrawArrays(0, 3); 

CommandBuffer->EndPass();

GfxContext->EndFrame();

GfxContext->Present();
```

And here's the result : 
![HelloTriangle](https://github.com/jacquespillet/gfx/blob/master/resources/Gallery/HelloTriangle.PNG?raw=true)

## Examples

* [Simple Graphics Pipeline](https://github.com/jacquespillet/gfx/blob/master/src/Main_HelloTriangle.cpp)

* [Compute Pipeline](https://github.com/jacquespillet/gfx/blob/master/src/Main_HelloTriangle_Compute.cpp)

* [Instanced Rendering](https://github.com/jacquespillet/gfx/blob/master/src/Main_HelloTriangle_Instanced.cpp)

* [Multi Sampling Anti Aliasing](https://github.com/jacquespillet/gfx/blob/master/src/Main_HelloTriangle_MultiSampling.cpp)

* [Offscreen Render Targets](https://github.com/jacquespillet/gfx/blob/master/src/Main_HelloTriangle_OffscreenRenderTarget.cpp)

* [Dearimgui Support](https://github.com/jacquespillet/gfx/blob/master/src/Main_HelloImgui.cpp)

* [Cubemap Textures](https://github.com/jacquespillet/gfx/blob/master/src/Main_HelloCubeMap.cpp)



# High level api features

The Hlgfx library contains abstractions classes for creating 3d apps : 

* [Camera](https://github.com/jacquespillet/gfx/blob/master/src/Hlgfx/Camera.cpp) and [Camera Controller](https://github.com/jacquespillet/gfx/blob/master/src/Hlgfx/CameraController.cpp) for user interaction

* [Material system](https://github.com/jacquespillet/gfx/blob/master/src/Hlgfx/Material.cpp)

* [Lights](https://github.com/jacquespillet/gfx/blob/master/src/Hlgfx/Light.cpp)

* [Scene graph](https://github.com/jacquespillet/gfx/blob/master/src/Hlgfx/Light.cpp)

* [Transform](https://github.com/jacquespillet/gfx/blob/master/src/Hlgfx/Transform.cpp)

* [Renderer](https://github.com/jacquespillet/gfx/blob/master/src/Hlgfx/Renderer.cpp)

* [Object3D](https://github.com/jacquespillet/gfx/blob/master/src/Hlgfx/Object3D.cpp)

* [Geometry](https://github.com/jacquespillet/gfx/blob/master/src/Hlgfx/Geometry.cpp)

* [Mesh](https://github.com/jacquespillet/gfx/blob/master/src/Hlgfx/Mesh.cpp)


# Mini engine

The mini engine contained in Main_HelloHLGFX shows how to use the Hlgfx library to create a simple 3d app.

This engine allows to author 3d scenes by importing glTF or any file supported by assimp.

It has a scene graph that allows to parent objects together to create complex 3d object hierarchies.

It also has a PBR material authoring system.

It also contains an asset management system that allows to import and instantiate assets into the scene. 


# Screenshots


# Scene assets
