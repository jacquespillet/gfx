cmake_minimum_required(VERSION 2.9)
project(gl)
set(PROJECT_BRIEF "gl")

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
set(CMAKE_CXX_STANDARD 17)
add_compile_options(/wd4251)

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

if(${GFX_API} EQUAL ${GFX_API_VK} OR ${GFX_API} EQUAL ${GFX_API_D3D12} OR ${GFX_API} EQUAL ${GFX_API_D3D11})
add_compile_definitions(GLM_FORCE_DEPTH_ZERO_TO_ONE)
endif()

# if(${GFX_API} EQUAL ${GFX_API_D3D12} OR ${GFX_API} EQUAL ${GFX_API_D3D11})
# add_compile_definitions(GLM_FORCE_LEFT_HANDED)
# endif()

set(BUILD_ALL 0)


################################################
##########      Library         ################
################################################

################
# Third parties
################

# Imgui
set(IMGUI_INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui ${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui/backends ${CMAKE_CURRENT_SOURCE_DIR}/vendor/ImGuizmo/)
set(IMGUI_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui/imgui_demo.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui/backends/imgui_impl_glfw.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui/imgui.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui/imgui_draw.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui/imgui_widgets.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui/imgui_tables.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/vendor/ImGuizmo/ImGuizmo.cpp
)
if(${GFX_API} EQUAL ${GFX_API_D3D12})
set(IMGUI_SOURCE ${IMGUI_SOURCE} ${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui/backends/imgui_impl_dx12.cpp)
set(IMGUI_SOURCE ${IMGUI_SOURCE} ${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui/backends/imgui_impl_win32.cpp)
elseif(${GFX_API} EQUAL ${GFX_API_GL})
set(IMGUI_SOURCE ${IMGUI_SOURCE} ${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui/backends/imgui_impl_opengl3.cpp)
elseif(${GFX_API} EQUAL ${GFX_API_VK})
set(IMGUI_SOURCE ${IMGUI_SOURCE} ${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui/backends/imgui_impl_vulkan.cpp)
elseif(${GFX_API} EQUAL ${GFX_API_D3D11})
set(IMGUI_SOURCE ${IMGUI_SOURCE} ${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui/backends/imgui_impl_win32.cpp)
set(IMGUI_SOURCE ${IMGUI_SOURCE} ${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui/backends/imgui_impl_dx11.cpp)
endif()

include_directories(${IMGUI_INC_DIR})

# GLM
set(GLM_INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vendor/glm)
include_directories(${GLM_INC_DIR})

# # stb
set(STB_INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vendor/stb)
include_directories(${STB_INC_DIR})

# # uuid
set(UUID_INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vendor/uuid_v4)
include_directories(${UUID_INC_DIR})

# # tinygltf
set(TINYGLTF_INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vendor/tinygltf)
include_directories(${TINYGLTF_INC_DIR})

# Glfw
set(USE_MSVC_RUNTIME_LIBRARY_DLL  ON)
set(BUILD_SHARED_LIBS  ON) 
set(GLFW_INSTALL ON)
add_subdirectory(vendor/glfw)
include_directories(vendor/glfw/include)


# FileDialog
set(FILEDIALOG_INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vendor/fileDialog/src/include)
set(FILEDIALOG_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/vendor/fileDialog/src/nfd_common.c)
set(FILEDIALOG_SOURCE ${FILEDIALOG_SOURCE} ${CMAKE_CURRENT_SOURCE_DIR}/vendor/fileDialog/src/nfd_win.cpp)
include_directories(${FILEDIALOG_INC_DIR})
link_directories(${FILEDIALOG_LIB_DIR})

# JSON
set(JSON_INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vendor/json)
include_directories(${JSON_INC_DIR})


# # Glad
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/vendor/glad/cmake" glad_cmake)
glad_add_library(glad_gl_core_33 STATIC API gl:core=3.3)
include_directories("${CMAKE_CURRENT_BINARY_DIR}/gladsources/glad_gl_core_33/include")
set(GLAD_SOURCE ${CMAKE_CURRENT_BINARY_DIR}/gladsources/glad_gl_core_33/src/gl.c)


# ASSIMP
set(ASSIMP_INSTALL_PDB OFF)
set(USE_STATIC_CRT ON)
add_subdirectory(vendor/assimp)
include_directories("${CMAKE_CURRENT_BINARY_DIR}/vendor/assimp/include")
include_directories(vendor/assimp/include)


# tlsf
set(TLSF_INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vendor/tlsf)
set(TLSF_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/vendor/tlsf/tlsf.c)
include_directories(${TLSF_INC_DIR})


# Vulkan

if(${GFX_API} EQUAL ${GFX_API_VK})

        # Vma
        include_directories(${CMAKE_CURRENT_SOURCE_DIR}/vendor/vma/include)
        
        # # Spirv tools
        # set(SPIRV_TOOLS_BUILD_STATIC ON)
        # add_subdirectory(vendor/spirv_tools)
        
        # # Spirv headers
        set(SPIRV_HEADERS_ENABLE_INSTALL ON)
        add_subdirectory(vendor/spirv_header)
        
        # glslang
        # set(ENABLE_OPT ON)
        # set(GLSLANG_ENABLE_INSTALL ON)
        # set(ENABLE_GLSLANG_BINARIES ON)
        set(BUILD_SHARED_LIBS  OFF) 
        add_subdirectory(vendor/glslang)
        
        # spirv_reflect
        set(SPIRVREFLECT_INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vendor/spirv_reflect)
        set(SPIRVREFLECT_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/vendor/spirv_reflect/spirv_reflect.c)
        include_directories(${SPIRVREFLECT_INC_DIR})


        set(VULKAN_LIB_DIR $ENV{VULKAN_SDK}/Lib)
        set(VULKAN_INC_DIR $ENV{VULKAN_SDK}/Include)
        set(VULKAN_BIN_DIR $ENV{VULKAN_SDK}/Bin)
        include_directories(${VULKAN_INC_DIR})
        link_directories(${VULKAN_LIB_DIR})        
endif()

if(${GFX_API} EQUAL ${GFX_API_D3D12})
        set(D3DX_INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vendor/d3d12/include/directx)
        include_directories(${D3DX_INC_DIR})
        
        set(DXC_INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vendor/dxc/inc)
        set(DXC_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vendor/dxc/lib/x64)
        set(DXC_BIN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vendor/dxc/bin/x64)
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
        src/Gfx/VK/VkVmaUsage.cpp
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
        src/Gfx/VK/VkAccelerationStructure.cpp
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
        ${GLAD_SOURCE}
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
                src/Hlgfx/Renderer.cpp
                src/Hlgfx/Light.cpp
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
set(commonLibs glfw assimp user32 gdi32)

if(${GFX_API} EQUAL ${GFX_API_VK})
        set(gfxApiLibs vulkan-1 SPIRV-Headers SPIRV-Tools)

        set(gfxApiLibsDebug ${gfxApiLibs})
        set(gfxApiLibsDebug
                ${gfxApiLibsDebug}  
                OSDependentd
                SPIRVd
                glslangd
                MachineIndependentd
                SPVRemapperd
                GenericCodeGend
                HLSLd
                OGLCompilerd
        )
        
        set(gfxApiLibsRelease ${gfxApiLibs})
        set(gfxApiLibsRelease
                ${gfxApiLibsRelease}  
                OSDependent
                SPIRV
                glslang
                MachineIndependent
                SPVRemapper
                GenericCodeGen
                HLSL
                OGLCompiler
        )
endif()

if(${GFX_API} EQUAL ${GFX_API_D3D12})
        set(gfxApiLibs ${gfxApiLibs} D3D12.lib)
        set(gfxApiLibs ${gfxApiLibs} DXGI.lib)
        set(gfxApiLibs ${gfxApiLibs} D3DCompiler.lib)
        set(gfxApiLibs ${gfxApiLibs} dxcompiler.lib)
        
        set(gfxApiLibsDebug ${gfxApiLibs})
        set(gfxApiLibsRelease ${gfxApiLibs})

        install(FILES ${DXC_BIN_DIR}/dxil.dll DESTINATION bin)
endif()

if(${GFX_API} EQUAL ${GFX_API_D3D11})
        set(gfxApiLibs ${gfxApiLibs} d3d11.lib)
        set(gfxApiLibsRelease ${gfxApiLibs})
        set(gfxApiLibsDebug ${gfxApiLibs})
        endif()
        
        if(${GFX_API} EQUAL ${GFX_API_GL})
        set(gfxApiLibs ${gfxApiLibs} opengl32)
        set(gfxApiLibsRelease ${gfxApiLibs})
        set(gfxApiLibsDebug ${gfxApiLibs})
endif()



if(${BUILD_ALL} EQUAL 1)
        # HelloTriangle
        set(targetName HelloTriangle_${GFX_API_STR})
        set(mainCode ${sourceCode} src/Main_HelloTriangle.cpp)
        add_executable(${targetName} ${mainCode})
        target_link_libraries(${targetName} ${commonLibs})
        target_link_libraries(${targetName} debug ${gfxApiLibsDebug} optimized ${gfxApiLibsRelease})
        install(TARGETS ${targetName} RUNTIME DESTINATION bin)
        
        # HelloTriangleIndexed
        set(targetName HelloTriangleIndexed_${GFX_API_STR})
        set(mainCode ${sourceCode} src/Main_HelloTriangle_Indexed.cpp)
        add_executable(${targetName} ${mainCode})
        target_link_libraries(${targetName} ${commonLibs})
        target_link_libraries(${targetName} debug ${gfxApiLibsDebug} optimized ${gfxApiLibsRelease})
        install(TARGETS ${targetName} RUNTIME DESTINATION bin)

        # HelloTriangleMultiStram
        set(targetName HelloTriangleMultiVertexStream_${GFX_API_STR})
        set(mainCode ${sourceCode} src/Main_HelloTriangle_MultiVertexStreams.cpp)
        add_executable(${targetName} ${mainCode})
        target_link_libraries(${targetName} ${commonLibs})
        target_link_libraries(${targetName} debug ${gfxApiLibsDebug} optimized ${gfxApiLibsRelease})        
        install(TARGETS ${targetName} RUNTIME DESTINATION bin)

        # Instanced
        set(targetName HelloTriangleInstanced_${GFX_API_STR})
        set(mainCode ${sourceCode} src/Main_HelloTriangle_Instanced.cpp)
        add_executable(${targetName} ${mainCode})
        target_link_libraries(${targetName} ${commonLibs})
        target_link_libraries(${targetName} debug ${gfxApiLibsDebug} optimized ${gfxApiLibsRelease})        
        install(TARGETS ${targetName} RUNTIME DESTINATION bin)

        # HelloCube
        set(targetName HelloCube_${GFX_API_STR})
        set(mainCode ${sourceCode} src/Main_HelloCube.cpp)
        add_executable(${targetName} ${mainCode})
        target_link_libraries(${targetName} ${commonLibs})
        target_link_libraries(${targetName} debug ${gfxApiLibsDebug} optimized ${gfxApiLibsRelease})        
        install(TARGETS ${targetName} RUNTIME DESTINATION bin)
        
        # Multisampling
        set(targetName HelloTriangleMSAA_${GFX_API_STR})
        set(mainCode ${sourceCode} src/Main_HelloTriangle_MultiSampling.cpp)
        add_executable(${targetName} ${mainCode})
        target_link_libraries(${targetName} ${commonLibs})
        target_link_libraries(${targetName} debug ${gfxApiLibsDebug} optimized ${gfxApiLibsRelease})        
        install(TARGETS ${targetName} RUNTIME DESTINATION bin)  
        
        # Storage buffer
        set(targetName HelloTriangleStorageBuffer_${GFX_API_STR})
        set(mainCode ${sourceCode} src/Main_HelloTriangle_StorageBuffer.cpp)
        add_executable(${targetName} ${mainCode})
        target_link_libraries(${targetName} ${commonLibs})
        target_link_libraries(${targetName} debug ${gfxApiLibsDebug} optimized ${gfxApiLibsRelease})        
        install(TARGETS ${targetName} RUNTIME DESTINATION bin)
        
        # Compute
        set(targetName HelloTriangleCompute_${GFX_API_STR})
        set(mainCode ${sourceCode} src/Main_HelloTriangle_Compute.cpp)
        add_executable(${targetName} ${mainCode})
        target_link_libraries(${targetName} ${commonLibs})
        target_link_libraries(${targetName} debug ${gfxApiLibsDebug} optimized ${gfxApiLibsRelease})        
        install(TARGETS ${targetName} RUNTIME DESTINATION bin)
        
        # Offscreen
        set(targetName HelloTriangleOffscreen_${GFX_API_STR})
        set(mainCode ${sourceCode} src/Main_HelloTriangle_OffscreenRenderTarget.cpp)
        add_executable(${targetName} ${mainCode})
        target_link_libraries(${targetName} ${commonLibs})
        target_link_libraries(${targetName} debug ${gfxApiLibsDebug} optimized ${gfxApiLibsRelease})        
        install(TARGETS ${targetName} RUNTIME DESTINATION bin)

        # Cubemap
        set(targetName HelloCubemap_${GFX_API_STR})
        set(mainCode ${sourceCode} src/Main_HelloCubeMap.cpp)
        add_executable(${targetName} ${mainCode})
        target_link_libraries(${targetName} ${commonLibs})
        target_link_libraries(${targetName} debug ${gfxApiLibsDebug} optimized ${gfxApiLibsRelease})        
        install(TARGETS ${targetName} RUNTIME DESTINATION bin)
        
        # Imgui
        set(targetName HelloImgui_${GFX_API_STR})
        set(mainCode ${sourceCode} src/Main_HelloImgui.cpp)
        add_executable(${targetName} ${mainCode})
        target_link_libraries(${targetName} ${commonLibs})
        target_link_libraries(${targetName} debug ${gfxApiLibsDebug} optimized ${gfxApiLibsRelease})        
        install(TARGETS ${targetName} RUNTIME DESTINATION bin)

        # ModelViewer
        set(targetName Main_${GFX_API_STR})
        set(mainCode ${sourceCode} src/Main_HelloHLGFX.cpp)
        add_executable(${targetName} ${mainCode})
        target_link_libraries(${targetName} ${commonLibs})
        target_link_libraries(${targetName} debug ${gfxApiLibsDebug} optimized ${gfxApiLibsRelease})        
        install(TARGETS ${targetName} RUNTIME DESTINATION bin)
else()
        set(targetName Main_${GFX_API_STR})
        set(mainCode ${sourceCode} src/Main_HelloHLGFX.cpp)
        add_executable(${targetName} ${mainCode})
        target_link_libraries(${targetName} ${commonLibs})
        target_link_libraries(${targetName} debug ${gfxApiLibsDebug} optimized ${gfxApiLibsRelease})

        install(TARGETS ${targetName} RUNTIME DESTINATION bin)
endif()


# Install
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/resources/
        DESTINATION bin/resources
)

