cmake_minimum_required(VERSION 2.9)
project(gl)
set(PROJECT_BRIEF "gl")

# set(CMAKE_BUILD_TYPE RELEASE)
set(LINK_FLAGS "")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COMPILE_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${LINK_FLAGS}")
set(CMAKE_CXX_STANDARD 17)

add_compile_definitions(_GLFW_WIN32 GLFW_EXPOSE_NATIVE_WIN32 VK_USE_PLATFORM_WIN32_KHR NOMINMAX GLM_ENABLE_EXPERIMENTAL )

set(GFX_API_GL 0)
set(GFX_API_VK 1)
set(GFX_API_D3D12 2)
set(GFX_API_D3D11 3)
set(GFX_API ${GFX_API_D3D12})

add_definitions(-DGFX_GL=${GFX_API_GL})
add_definitions(-DGFX_VK=${GFX_API_VK})
add_definitions(-DGFX_D3D12=${GFX_API_D3D12})
add_definitions(-DGFX_D3D11=${GFX_API_D3D11})
add_definitions(-DGFX_API=${GFX_API})

set(BUILD_ALL 0)


################################################
##########      Library         ################
################################################

################
# Third parties
################

# Imgui
set(IMGUI_INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ext/dearimgui)
set(IMGUI_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/ext/dearimgui/imgui_demo.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/ext/dearimgui/imgui_impl_glfw.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/ext/dearimgui/imgui.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/ext/dearimgui/imgui_draw.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/ext/dearimgui/imgui_widgets.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/ext/dearimgui/ImGuizmo.cpp
                 )
if(${GFX_API} EQUAL ${GFX_API_D3D12})
set(IMGUI_SOURCE ${IMGUI_SOURCE} ${CMAKE_CURRENT_SOURCE_DIR}/ext/dearimgui/imgui_impl_dx12.cpp)
set(IMGUI_SOURCE ${IMGUI_SOURCE} ${CMAKE_CURRENT_SOURCE_DIR}/ext/dearimgui/imgui_impl_win32.cpp)
elseif(${GFX_API} EQUAL ${GFX_API_GL})
set(IMGUI_SOURCE ${IMGUI_SOURCE} ${CMAKE_CURRENT_SOURCE_DIR}/ext/dearimgui/imgui_impl_opengl3.cpp)
elseif(${GFX_API} EQUAL ${GFX_API_VK})
set(IMGUI_SOURCE ${IMGUI_SOURCE} ${CMAKE_CURRENT_SOURCE_DIR}/ext/dearimgui/imgui_impl_vulkan.cpp)
elseif(${GFX_API} EQUAL ${GFX_API_D3D11})
set(IMGUI_SOURCE ${IMGUI_SOURCE} ${CMAKE_CURRENT_SOURCE_DIR}/ext/dearimgui/imgui_impl_win32.cpp)
set(IMGUI_SOURCE ${IMGUI_SOURCE} ${CMAKE_CURRENT_SOURCE_DIR}/ext/dearimgui/imgui_impl_dx11.cpp)
endif()

include_directories(${IMGUI_INC_DIR})

# GLM
set(GLM_INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ext/glm)
include_directories(${GLM_INC_DIR})

# # stb
set(STB_INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ext/stb)
include_directories(${STB_INC_DIR})

# # uuid
set(UUID_INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ext/uuid_v4)
include_directories(${UUID_INC_DIR})

# # tinygltf
set(TINYGLTF_INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ext/tinygltf)
include_directories(${TINYGLTF_INC_DIR})


# Glfw
set(GLFW_INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ext/glfw/build/install/include)
set(GLFW_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ext/glfw/build/install/lib)
include_directories(${GLFW_INC_DIR})
link_directories(${GLFW_LIB_DIR})

# FileDialog
set(FILEDIALOG_INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ext/fileDialog/include)
set(FILEDIALOG_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/ext/fileDialog/src/nfd_common.c)
set(FILEDIALOG_SOURCE ${FILEDIALOG_SOURCE} ${CMAKE_CURRENT_SOURCE_DIR}/ext/fileDialog/src/nfd_win.cpp)
include_directories(${FILEDIALOG_INC_DIR})
link_directories(${FILEDIALOG_LIB_DIR})

# JSON
set(JSON_INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ext/json)
include_directories(${JSON_INC_DIR})


# # Glew
set(GLEW_INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ext/glew/include)
set(GLEW_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ext/glew/lib/Release/x64)
set(GLEW_DLL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ext/glew/bin/Release/x64)
include_directories(${GLEW_INC_DIR})
link_directories(${GLEW_LIB_DIR})

# ASSIMP
set(ASSIMP_INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ext/assimp/include)
set(ASSIMP_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ext/assimp/lib)
set(ASSIMP_DLL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ext/assimp/bin)
include_directories(${ASSIMP_INC_DIR})
link_directories(${ASSIMP_LIB_DIR})


# tlsf
set(TLSF_INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ext/tlsf)
set(TLSF_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/ext/tlsf/tlsf.c)
include_directories(${TLSF_INC_DIR})


# Vulkan

if(${GFX_API} EQUAL ${GFX_API_VK})
        set(VULKAN_LIB_DIR $ENV{VULKAN_SDK}/Lib)
        set(VULKAN_INC_DIR $ENV{VULKAN_SDK}/Include)
        set(VULKAN_BIN_DIR $ENV{VULKAN_SDK}/Bin)
        include_directories(${VULKAN_INC_DIR})
        link_directories(${VULKAN_LIB_DIR})
        # Vma
        set(VMA_INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ext/vma/include)
        set(VMA_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ext/vma/lib)
        include_directories(${VMA_INC_DIR})
        link_directories(${VMA_LIB_DIR})

        # glslang
        set(GLSLANG_INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ext/glslang/include)
        set(GLSLANG_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ext/glslang/lib)
        include_directories(${GLSLANG_INC_DIR})
        link_directories(${GLSLANG_LIB_DIR})

        # spirv_reflect
        set(SPIRVREFLECT_INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ext/spirv-reflect)
        set(SPIRVREFLECT_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/ext/spirv-reflect/spirv_reflect.c)
        include_directories(${SPIRVREFLECT_INC_DIR})
endif()

if(${GFX_API} EQUAL ${GFX_API_D3D12})
        set(D3DX_INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ext/d3dx)
        include_directories(${D3DX_INC_DIR})
        set(DXC_INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ext/dxc/include)
        set(DXC_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ext/dxc/lib)
        set(DXC_BIN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ext/dxc/bin)
        include_directories(${DXC_INC_DIR})
        link_directories(${DXC_LIB_DIR})
endif()

if(${GFX_API} EQUAL ${GFX_API_D3D11})

endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

################################################
##########      Code              ##############
################################################

set(extCode    
        ${TLSF_SOURCE}
        ${VOLK_SOURCE}
        ${SPIRVREFLECT_SOURCE}
        ${IMGUI_SOURCE}
        ${FILEDIALOG_SOURCE}
)

set(vkCode
        src/Gfx/VK/VkContext.cpp
        src/Gfx/VK/VkCommandBuffer.cpp
        src/Gfx/VK/VkBuffer.cpp
        src/Gfx/VK/VkMapping.cpp
        src/Gfx/VK/VkImage.cpp
        src/Gfx/VK/VkMemoryAllocation.cpp
        src/Gfx/VK/VkResourceManager.cpp
        src/Gfx/VK/VkPipeline.cpp
        src/Gfx/VK/VkShader.cpp
        src/Gfx/VK/VkVirtualFrames.cpp
        src/Gfx/VK/VkUniform.cpp
        src/Gfx/VK/VkImgui.cpp
)

set(d3d12Code
        src/Gfx/D3D12/D12ResourceManager.cpp
        src/Gfx/D3D12/D12Context.cpp
        src/Gfx/D3D12/D12Pipeline.cpp
        src/Gfx/D3D12/D12VirtualFrames.cpp
        src/Gfx/D3D12/D12CommandBuffer.cpp
        src/Gfx/D3D12/D12Swapchain.cpp
        src/Gfx/D3D12/D12Framebuffer.cpp
        src/Gfx/D3D12/D12Mapping.cpp
        src/Gfx/D3D12/D12Buffer.cpp
        src/Gfx/D3D12/D12Image.cpp
        src/Gfx/D3D12/D12Uniform.cpp
        src/Gfx/D3D12/D12Imgui.cpp
)

set(d3d11Code
        src/Gfx/D3D11/D11Context.cpp
        src/Gfx/D3D11/D11Uniforms.cpp
        src/Gfx/D3D11/D11ResourceManager.cpp
        src/Gfx/D3D11/D11Image.cpp
        src/Gfx/D3D11/D11Pipeline.cpp
        src/Gfx/D3D11/D11Buffer.cpp
        src/Gfx/D3D11/D11Swapchain.cpp
        src/Gfx/D3D11/D11CommandBuffer.cpp
        src/Gfx/D3D11/D11Mapping.cpp
        src/Gfx/D3D11/D11Framebuffer.cpp
        src/Gfx/D3D11/D11Imgui.cpp
)

set(glCode
        src/Gfx/GL/GLContext.cpp
        src/Gfx/GL/GLPipeline.cpp
        src/Gfx/GL/GLResourceManager.cpp
        src/Gfx/GL/GLBuffer.cpp
        src/Gfx/GL/GLImage.cpp
        src/Gfx/GL/GLMapping.cpp
        src/Gfx/GL/GLShader.cpp
        src/Gfx/GL/GLCommandBuffer.cpp
        src/Gfx/GL/GLUniform.cpp
        src/Gfx/GL/GLImgui.cpp
)
        
set(commonCode
        src/Gfx/Common/Memory.cpp
        src/Gfx/Common/ResourceManager.cpp
        src/Gfx/Common/Context.cpp
        src/Gfx/Common/Image.cpp
        src/Gfx/Common/Util.cpp
        src/Gfx/Common/Pipeline.cpp
        src/Gfx/Common/Uniform.cpp
        src/Gfx/Common/Framebuffer.cpp
)
set(appCode
        src/App/Window.cpp
)


set (sourceCode
        ${extCode}
        ${appCode}
        ${commonCode}
)

if(${GFX_API} EQUAL ${GFX_API_VK})
        set(GFX_API_STR "VK")
        set (sourceCode
        ${sourceCode}
        ${vkCode}
        )
elseif(${GFX_API} EQUAL ${GFX_API_D3D12})
        set(GFX_API_STR "D3D12")
        set (sourceCode
        ${sourceCode}
                ${d3d12Code}
                )
elseif(${GFX_API} EQUAL ${GFX_API_GL})
        set(GFX_API_STR "GL")
        set (sourceCode
                ${sourceCode}
                ${glCode}
        )
elseif(${GFX_API} EQUAL ${GFX_API_D3D11})
        set(GFX_API_STR "D3D11")
        set (sourceCode
                ${sourceCode}
                ${d3d11Code}
        )
endif()

set (sourceCode ${sourceCode}
                src/Hlgfx/Context.cpp
                src/Hlgfx/GUI.cpp
                src/Hlgfx/Texture.cpp
                src/Hlgfx/Object3D.cpp
                src/Hlgfx/Transform.cpp
                src/Hlgfx/Mesh.cpp
                src/Hlgfx/Material.cpp
                src/Hlgfx/Geometry.cpp
                src/Hlgfx/Camera.cpp
                src/Hlgfx/Scene.cpp
                src/Hlgfx/CameraController.cpp
                src/Hlgfx/Util.cpp
                src/Hlgfx/Loaders/GLTF.cpp
                src/Hlgfx/Loaders/Assimp.cpp
)

################################################
##########      Libs              ##############
################################################
set(commonLibs glfw3 assimp-vc141-mt.lib)

if(${GFX_API} EQUAL ${GFX_API_VK})
        set(gfxApiLibs ${gfxApiLibs} vulkan-1)
        set(gfxApiLibs ${gfxApiLibs} debug VulkanMemoryAllocatord optimized VulkanMemoryAllocator)
        set(gfxApiLibs
                ${gfxApiLibs}  
                debug ${GLSLANG_LIB_DIR}/OSDependentd.lib optimized ${GLSLANG_LIB_DIR}/OSDependent.lib
                debug ${GLSLANG_LIB_DIR}/SPIRVd.lib optimized ${GLSLANG_LIB_DIR}/SPIRV.lib
                debug ${GLSLANG_LIB_DIR}/glslangd.lib optimized ${GLSLANG_LIB_DIR}/glslang.lib
                debug ${GLSLANG_LIB_DIR}/MachineIndependentd.lib optimized ${GLSLANG_LIB_DIR}/MachineIndependent.lib
                debug ${GLSLANG_LIB_DIR}/SPVRemapperd.lib optimized ${GLSLANG_LIB_DIR}/SPVRemapper.lib
                debug ${GLSLANG_LIB_DIR}/GenericCodeGend.lib optimized ${GLSLANG_LIB_DIR}/GenericCodeGen.lib
                debug ${GLSLANG_LIB_DIR}/glslang-default-resource-limitsd.lib optimized ${GLSLANG_LIB_DIR}/glslang-default-resource-limits.lib
                debug ${GLSLANG_LIB_DIR}/HLSLd.lib optimized ${GLSLANG_LIB_DIR}/HLSL.lib
                debug ${GLSLANG_LIB_DIR}/OGLCompilerd.lib optimized ${GLSLANG_LIB_DIR}/OGLCompiler.lib
        )
endif()

if(${GFX_API} EQUAL ${GFX_API_D3D12})
        set(gfxApiLibs ${gfxApiLibs} D3D12.lib)
        set(gfxApiLibs ${gfxApiLibs} DXGI.lib)
        set(gfxApiLibs ${gfxApiLibs} D3DCompiler.lib)
        set(gfxApiLibs ${gfxApiLibs} dxcompiler.lib)
        install(FILES ${DXC_BIN_DIR}/dxil.dll DESTINATION bin)
endif()

if(${GFX_API} EQUAL ${GFX_API_D3D11})
        set(gfxApiLibs ${gfxApiLibs} d3d11.lib)
endif()

if(${GFX_API} EQUAL ${GFX_API_GL})
        set(gfxApiLibs ${gfxApiLibs} glew32)
        set(gfxApiLibs ${gfxApiLibs} opengl32)
        install(FILES ${GLEW_DLL_DIR}/glew32.dll DESTINATION bin)
endif()



if(${BUILD_ALL} EQUAL 1)
        # HelloTriangle
        set(targetName HelloTriangle_${GFX_API_STR})
        set(mainCode ${sourceCode} src/Main_HelloTriangle.cpp)
        add_executable(${targetName} ${mainCode})
        target_link_libraries(${targetName} ${commonLibs} ${gfxApiLibs})
        install(TARGETS ${targetName} RUNTIME DESTINATION bin)
        
        # HelloTriangleIndexed
        set(targetName HelloTriangleIndexed_${GFX_API_STR})
        set(mainCode ${sourceCode} src/Main_HelloTriangle_Indexed.cpp)
        add_executable(${targetName} ${mainCode})
        target_link_libraries(${targetName} ${commonLibs} ${gfxApiLibs})
        install(TARGETS ${targetName} RUNTIME DESTINATION bin)

        # HelloTriangleMultiStram
        set(targetName HelloTriangleMultiVertexStream_${GFX_API_STR})
        set(mainCode ${sourceCode} src/Main_HelloTriangle_MultiVertexStreams.cpp)
        add_executable(${targetName} ${mainCode})
        target_link_libraries(${targetName} ${commonLibs} ${gfxApiLibs})
        install(TARGETS ${targetName} RUNTIME DESTINATION bin)

        # Instanced
        set(targetName HelloTriangleInstanced_${GFX_API_STR})
        set(mainCode ${sourceCode} src/Main_HelloTriangle_Instanced.cpp)
        add_executable(${targetName} ${mainCode})
        target_link_libraries(${targetName} ${commonLibs} ${gfxApiLibs})
        install(TARGETS ${targetName} RUNTIME DESTINATION bin)

        # HelloCube
        set(targetName HelloCube_${GFX_API_STR})
        set(mainCode ${sourceCode} src/Main_HelloCube.cpp)
        add_executable(${targetName} ${mainCode})
        target_link_libraries(${targetName} ${commonLibs} ${gfxApiLibs})
        install(TARGETS ${targetName} RUNTIME DESTINATION bin)
        
        # Multisampling
        set(targetName HelloTriangleMSAA_${GFX_API_STR})
        set(mainCode ${sourceCode} src/Main_HelloTriangle_MultiSampling.cpp)
        add_executable(${targetName} ${mainCode})
        target_link_libraries(${targetName} ${commonLibs} ${gfxApiLibs})
        install(TARGETS ${targetName} RUNTIME DESTINATION bin)  
        
        # Storage buffer
        set(targetName HelloTriangleStorageBuffer_${GFX_API_STR})
        set(mainCode ${sourceCode} src/Main_HelloTriangle_StorageBuffer.cpp)
        add_executable(${targetName} ${mainCode})
        target_link_libraries(${targetName} ${commonLibs} ${gfxApiLibs})
        install(TARGETS ${targetName} RUNTIME DESTINATION bin)
        
        # Compute
        set(targetName HelloTriangleCompute_${GFX_API_STR})
        set(mainCode ${sourceCode} src/Main_HelloTriangle_Compute.cpp)
        add_executable(${targetName} ${mainCode})
        target_link_libraries(${targetName} ${commonLibs} ${gfxApiLibs})
        install(TARGETS ${targetName} RUNTIME DESTINATION bin)
        
        # Offscreen
        set(targetName HelloTriangleOffscreen_${GFX_API_STR})
        set(mainCode ${sourceCode} src/Main_HelloTriangle_OffscreenRenderTarget.cpp)
        add_executable(${targetName} ${mainCode})
        target_link_libraries(${targetName} ${commonLibs} ${gfxApiLibs})
        install(TARGETS ${targetName} RUNTIME DESTINATION bin)

        # Cubemap
        set(targetName HelloCubemap_${GFX_API_STR})
        set(mainCode ${sourceCode} src/Main_HelloCubeMap.cpp)
        add_executable(${targetName} ${mainCode})
        target_link_libraries(${targetName} ${commonLibs} ${gfxApiLibs})
        install(TARGETS ${targetName} RUNTIME DESTINATION bin)
        
        # Imgui
        set(targetName HelloImgui_${GFX_API_STR})
        set(mainCode ${sourceCode} src/Main_HelloImgui.cpp)
        add_executable(${targetName} ${mainCode})
        target_link_libraries(${targetName} ${commonLibs} ${gfxApiLibs})
        install(TARGETS ${targetName} RUNTIME DESTINATION bin)

        # ModelViewer
        set(targetName Main)
        set(mainCode ${sourceCode} src/Main_HelloHLGFX.cpp)
        add_executable(${targetName} ${mainCode})
        target_link_libraries(${targetName} ${commonLibs} ${gfxApiLibs})
        install(TARGETS ${targetName} RUNTIME DESTINATION bin)
else()
        set(targetName Main)
        set(mainCode ${sourceCode} src/Main_HelloHLGFX.cpp)
        add_executable(${targetName} ${mainCode})
        target_link_libraries(${targetName} ${commonLibs} ${gfxApiLibs})
        install(TARGETS ${targetName} RUNTIME DESTINATION bin)
endif()


install(FILES ${ASSIMP_DLL_DIR}/assimp-vc141-mt.dll DESTINATION bin)

# Install
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/resources/
        DESTINATION bin/resources
)

