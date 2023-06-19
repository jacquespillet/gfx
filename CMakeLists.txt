cmake_minimum_required(VERSION 2.9)
project(gl)
set(PROJECT_BRIEF "gl")

# set(CMAKE_BUILD_TYPE RELEASE)
set(LINK_FLAGS "")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COMPILE_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${LINK_FLAGS}")
set(CMAKE_CXX_STANDARD 17)
add_compile_definitions(_GLFW_WIN32 GLFW_EXPOSE_NATIVE_WIN32 VK_USE_PLATFORM_WIN32_KHR)
# include_directories("common/")

################################################
##########      Library         ################
################################################

################
# Third parties
################

# # GLM
# set(GLM_INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ext/glm)
# include_directories(${GLM_INC_DIR})

# # stb
# set(STB_INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ext/stb)
# include_directories(${STB_INC_DIR})

# # tinygltf
# set(GLTF_INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ext/tinygltf)
# include_directories(${GLTF_INC_DIR})

# # Imgui
# set(IMGUI_INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ext/dearimgui)
# set(IMGUI_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/ext/dearimgui/imgui_demo.cpp
#                  ${CMAKE_CURRENT_SOURCE_DIR}/ext/dearimgui/imgui_impl_glfw.cpp
#                  ${CMAKE_CURRENT_SOURCE_DIR}/ext/dearimgui/imgui_impl_vulkan.cpp
#                  ${CMAKE_CURRENT_SOURCE_DIR}/ext/dearimgui/imgui.cpp
#                  ${CMAKE_CURRENT_SOURCE_DIR}/ext/dearimgui/imgui_draw.cpp
#                  ${CMAKE_CURRENT_SOURCE_DIR}/ext/dearimgui/imgui_widgets.cpp
# )
# include_directories(${IMGUI_INC_DIR})

# Glfw
set(GLFW_INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ext/glfw/build/install/include)
set(GLFW_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ext/glfw/build/install/lib)
include_directories(${GLFW_INC_DIR})
link_directories(${GLFW_LIB_DIR})

# # ASSIMP
# set(ASSIMP_INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ext/assimp/include)
# set(ASSIMP_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ext/assimp/lib)
# set(ASSIMP_DLL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ext/assimp/bin)
# include_directories(${ASSIMP_INC_DIR})
# link_directories(${ASSIMP_LIB_DIR})

# # Glew
# set(GLEW_INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ext/glew/include)
# set(GLEW_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ext/glew/lib)
# set(GLEW_DLL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ext/glew/bin)
# include_directories(${GLEW_INC_DIR})
# link_directories(${GLEW_LIB_DIR})

# volk
# set(VOLK_INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ext/volk_local/)
# set(VOLK_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/ext/volk_local/volk/volk.c)
# include_directories(${VOLK_INC_DIR})

# Vulkan
# set(VULKAN_LIB_DIR $ENV{VULKAN_SDK}/Lib)
# set(VULKAN_INC_DIR $ENV{VULKAN_SDK}/Include)
# set(VULKAN_BIN_DIR $ENV{VULKAN_SDK}/Bin)
# include_directories(${VULKAN_INC_DIR})
# link_directories(${VULKAN_LIB_DIR})

# # Vma
# set(VMA_INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ext/vma/include)
# set(VMA_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ext/vma/lib)
# include_directories(${VMA_INC_DIR})
# link_directories(${VMA_LIB_DIR})

# # Vma
# set(WYHASH_INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ext/wyhash)
# include_directories(${WYHASH_INC_DIR})

# # glslang
# set(GLSLANG_INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ext/glslang/include/glslang)
# set(GLSLANG_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ext/glslang/lib)
# include_directories(${GLSLANG_INC_DIR})
# link_directories(${GLSLANG_LIB_DIR})

# # spirv_reflect
# set(SPIRVREFLECT_INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ext/spirv-reflect)
# set(SPIRVREFLECT_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/ext/spirv-reflect/spirv_reflect.c)
# include_directories(${SPIRVREFLECT_INC_DIR})

# # tlsf
# set(TLSF_INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ext/tlsf)
# set(TLSF_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/ext/tlsf/tlsf.c)
# include_directories(${TLSF_INC_DIR})



################################################
##########      Test              ##############
################################################

set(sourceCode
        src/Main.cpp
        ${VOLK_SOURCE}
)
                
add_executable(Main ${sourceCode})
target_link_libraries(Main glfw3)

install(TARGETS Main
        RUNTIME
        DESTINATION bin)


install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/resources/
        DESTINATION bin/resources
)

