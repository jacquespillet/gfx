#pragma once
#include "../Include/Types.h"

namespace gfx
{
struct vkAccelerationStructureData
{
    vkAccelerationStructureData() = default;
    void InitBLAS(uint32_t NumVertices, uint32_t Stride, gfx::format Format, bufferHandle VertexBufferHandle, gfx::indexType IndexType = gfx::indexType::Uint16, uint32_t NumTriangles = 0, bufferHandle IndexBufferHandle = InvalidHandle, uint32_t PositionOffset=0);
    void InitTLAS(std::vector<glm::mat4> &Transforms, std::vector<accelerationStructureHandle> &AccelerationStructures, std::vector<int> Instances);
    void UpdateInstanceTransform(std::vector<u32> &Indices, std::vector<m4x4*> &Transforms);
    void RemoveInstances(std::vector<u32> &IndicesToRemove);
    vk::AccelerationStructureKHR AccelerationStructure = VK_NULL_HANDLE;
    bufferHandle BufferHandle = InvalidHandle;
    u64 DeviceAddress = 0;

    // TLAS
    std::vector<VkAccelerationStructureInstanceKHR> Instances{};
    bufferHandle InstancesBuffer = InvalidHandle;

    bool IsTLAS = false;

    void CreateAS(vk::AccelerationStructureBuildSizesInfoKHR Size, vk::AccelerationStructureTypeKHR Type);
    void DestroyAS();

    static gfx::bufferHandle ScratchBuffer;    
};
}