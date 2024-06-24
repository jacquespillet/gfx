#include "VkAccelerationStructure.h"
#include "VkContext.h"
#include "VkBuffer.h"
#include "VkCommandBuffer.h"
#include "../Include/Context.h"
#include "../Include/AccelerationStructure.h"

namespace gfx
{

void vkAccelerationStructureData::CreateAS(vk::AccelerationStructureBuildSizesInfoKHR Size, vk::AccelerationStructureTypeKHR Type)
{
#if 0
    if(AccelerationStructure != 0)
    {
        DestroyAS();
    }
#endif

    GET_CONTEXT(VkContext, context::Get());
    BufferHandle = context::Get()->CreateBuffer(Size.accelerationStructureSize, gfx::bufferUsage::AccelerationStructureStorage | gfx::bufferUsage::ShaderDeviceAddress, gfx::memoryUsage::GpuOnly);
    buffer *Buffer = context::Get()->GetBuffer(BufferHandle);
    GET_API_DATA(VkBufferData, vkBufferData, Buffer);

    vk::AccelerationStructureCreateInfoKHR AccelerationStructureCreateInfo;
    AccelerationStructureCreateInfo.buffer = VkBufferData->Handle;
    AccelerationStructureCreateInfo.size = Size.accelerationStructureSize;
    AccelerationStructureCreateInfo.type = Type;
    VkContext->_vkCreateAccelerationStructureKHR((VkDevice)VkContext->Device, (VkAccelerationStructureCreateInfoKHR*)&AccelerationStructureCreateInfo, nullptr, (VkAccelerationStructureKHR*) &AccelerationStructure);

    VkAccelerationStructureDeviceAddressInfoKHR AccelerationStructureDeviceAddressInfo {VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR};
    AccelerationStructureDeviceAddressInfo.accelerationStructure = AccelerationStructure;
    DeviceAddress = VkContext->_vkGetAccelerationStructureDeviceAddressKHR((VkDevice)VkContext->Device, &AccelerationStructureDeviceAddressInfo);
}

void vkAccelerationStructureData::InitBLAS(uint32_t NumVertices, uint32_t Stride, gfx::format Format, bufferHandle VertexBufferHandle, gfx::indexType IndexType, uint32_t NumTriangles, bufferHandle IndexBufferHandle, uint32_t PositionOffset)
{
    IsTLAS = false;
    
    gfx::context *Context = context::Get();
    GET_CONTEXT(VkContext, Context);

    // Identity matrix for the blas
    VkTransformMatrixKHR TransformMatrix = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f, 
        0.0f, 0.0f, 1.0f, 0.0f
    };
    bufferHandle MatrixBufferHandle = context::Get()->CreateBuffer(sizeof(VkTransformMatrixKHR), gfx::bufferUsage::ShaderDeviceAddress | gfx::bufferUsage::AccelerationStructureBuildInputReadonly, gfx::memoryUsage::CpuToGpu);
    context::Get()->CopyDataToBuffer(MatrixBufferHandle, &TransformMatrix, sizeof(VkTransformMatrixKHR), 0);

    // Create the AS geometry data from the vertex/index buffers
    vk::AccelerationStructureGeometryKHR AccelerationStructureGeometry;
    AccelerationStructureGeometry.setFlags(vk::GeometryFlagBitsKHR::eOpaque);
    AccelerationStructureGeometry.setGeometryType(vk::GeometryTypeKHR::eTriangles);
    vk::AccelerationStructureGeometryDataKHR Geometry;
    Geometry.triangles.vertexFormat = FormatToNative(Format);
    Geometry.triangles.vertexData.deviceAddress = VkContext->GetBufferDeviceAddress(VertexBufferHandle) + PositionOffset;
    Geometry.triangles.maxVertex = NumVertices;
    Geometry.triangles.vertexStride = Stride;
    Geometry.triangles.transformData.deviceAddress = VkContext->GetBufferDeviceAddress(MatrixBufferHandle);
    Geometry.triangles.transformData.hostAddress=nullptr;
    uint32_t NumPrimitives = 0;
    if(IndexBufferHandle != InvalidHandle)
    {
        Geometry.triangles.indexType = IndexTypeToNative(IndexType);
        Geometry.triangles.indexData.deviceAddress = VkContext->GetBufferDeviceAddress(IndexBufferHandle);
        NumPrimitives = NumTriangles;
    }
    else
    {
        Geometry.triangles.indexType = vk::IndexType::eNoneKHR;
        Geometry.triangles.indexData.deviceAddress = 0;
        NumPrimitives = NumVertices / 3;
    }
    AccelerationStructureGeometry.setGeometry(Geometry);
    
    // Build the geometry
    vk::AccelerationStructureBuildGeometryInfoKHR AccelerationStructureBuildGeometryInfo;
    AccelerationStructureBuildGeometryInfo.type = vk::AccelerationStructureTypeKHR::eBottomLevel;
    AccelerationStructureBuildGeometryInfo.flags = vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace;
    AccelerationStructureBuildGeometryInfo.geometryCount=1;
    AccelerationStructureBuildGeometryInfo.pGeometries = &AccelerationStructureGeometry;

    // Get the AS size
    vk::AccelerationStructureBuildSizesInfoKHR AccelerationStructureBuildSizeInfo;
    VkContext->_vkGetAccelerationStructureBuildSizesKHR(
        (VkDevice)VkContext->Device,
        VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
        (VkAccelerationStructureBuildGeometryInfoKHR*)&AccelerationStructureBuildGeometryInfo,
        &NumPrimitives,
        (VkAccelerationStructureBuildSizesInfoKHR*)&AccelerationStructureBuildSizeInfo
    );

    // Allocate the AS
    CreateAS(AccelerationStructureBuildSizeInfo, vk::AccelerationStructureTypeKHR::eBottomLevel);

    bufferHandle ScratchBufferHandle = Context->CreateBuffer(AccelerationStructureBuildSizeInfo.buildScratchSize, gfx::bufferUsage::StorageBuffer | gfx::bufferUsage::ShaderDeviceAddress, gfx::memoryUsage::GpuOnly);

    // Create the acceleration structure
    vk::AccelerationStructureBuildGeometryInfoKHR AccelerationBuildGeometryInfo;
    AccelerationBuildGeometryInfo.type = vk::AccelerationStructureTypeKHR::eBottomLevel;
    AccelerationBuildGeometryInfo.flags = vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace;
    AccelerationBuildGeometryInfo.mode = vk::BuildAccelerationStructureModeKHR::eBuild;
    AccelerationBuildGeometryInfo.dstAccelerationStructure = AccelerationStructure;
    AccelerationBuildGeometryInfo.geometryCount=1;
    AccelerationBuildGeometryInfo.pGeometries = &AccelerationStructureGeometry;
    AccelerationBuildGeometryInfo.scratchData.deviceAddress = VkContext->GetBufferDeviceAddress(ScratchBufferHandle);

    VkAccelerationStructureBuildRangeInfoKHR AccelerationStructureBuildRangeInfo {};
    AccelerationStructureBuildRangeInfo.primitiveCount = NumPrimitives;
    AccelerationStructureBuildRangeInfo.primitiveOffset = 0;
    AccelerationStructureBuildRangeInfo.firstVertex=0;
    AccelerationStructureBuildRangeInfo.transformOffset=0;
    std::vector<VkAccelerationStructureBuildRangeInfoKHR*> AccelerationBuildStructureRangeInfos = {&AccelerationStructureBuildRangeInfo};

    // Build on the GPU
    // TODO: Add a host AS build capability as well
    auto CommandBuffer = Context->GetImmediateCommandBuffer();
    CommandBuffer->Begin();

    GET_API_DATA(VkCommandBufferData, vkCommandBufferData, CommandBuffer);
    vk::CommandBuffer VkCommandBufferHandle = VkCommandBufferData->Handle;
    VkContext->_vkCmdBuildAccelerationStructuresKHR(
            (VkCommandBuffer)VkCommandBufferHandle,
            1,
            (VkAccelerationStructureBuildGeometryInfoKHR*)&AccelerationBuildGeometryInfo,
            AccelerationBuildStructureRangeInfos.data());
    CommandBuffer->End();

    Context->SubmitCommandBufferImmediate(CommandBuffer);

    Context->DestroyBuffer(ScratchBufferHandle);
    Context->DestroyBuffer(MatrixBufferHandle);
}


void vkAccelerationStructureData::InitTLAS(std::vector<glm::mat4> &Transforms, std::vector<accelerationStructureHandle> &AccelerationStructures, std::vector<int> InstanceBLASIndices)
{
    IsTLAS = true;
    gfx::context *Context = context::Get();
    GET_CONTEXT(VkContext, Context);

    std::vector<VkAccelerationStructureInstanceKHR> BLASInstances;
    BLASInstances.resize(InstanceBLASIndices.size());
    for(size_t i=0; i<InstanceBLASIndices.size(); i++)
    {	

        glm::mat4& Transform = Transforms[i];
        VkTransformMatrixKHR TransformMatrix = 
        {
            Transform[0][0], Transform[1][0], Transform[2][0], Transform[3][0],
            Transform[0][1], Transform[1][1], Transform[2][1], Transform[3][1],
            Transform[0][2], Transform[1][2], Transform[2][2], Transform[3][2],
        };

        int MeshIndex = InstanceBLASIndices[i];
        accelerationStructure *BLAS = Context->GetAccelerationStructure(AccelerationStructures[MeshIndex]);
        GET_API_DATA(VkBLAS, vkAccelerationStructureData, BLAS);

        VkAccelerationStructureInstanceKHR &BLASInstance = BLASInstances[i];
        BLASInstance.transform = TransformMatrix;
        BLASInstance.instanceCustomIndex = MeshIndex; //TODO: Is that right ?
        BLASInstance.mask = 0xFF;
        BLASInstance.instanceShaderBindingTableRecordOffset = 0; //This defines which hit shader gets called for this instance.
        BLASInstance.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
        BLASInstance.accelerationStructureReference = VkBLAS->DeviceAddress;

    }

    InstancesBuffer = Context->CreateBuffer(sizeof(VkAccelerationStructureInstanceKHR) * BLASInstances.size(), bufferUsage::ShaderDeviceAddress | bufferUsage::AccelerationStructureBuildInputReadonly, memoryUsage::CpuToGpu);
    Context->CopyDataToBuffer(InstancesBuffer, BLASInstances.data(), sizeof(VkAccelerationStructureInstanceKHR) * BLASInstances.size(), 0);

    // TODO: Do we need that ?
    TransformMatricesBuffer = Context->CreateBuffer(sizeof(glm::mat4) * Transforms.size(), bufferUsage::ShaderDeviceAddress | bufferUsage::StorageBuffer, memoryUsage::CpuToGpu);
    Context->CopyDataToBuffer(TransformMatricesBuffer, Transforms.data(), sizeof(glm::mat4) * Transforms.size(), 0);

    vk::DeviceOrHostAddressConstKHR InstanceDataDeviceAddress;
    InstanceDataDeviceAddress.deviceAddress = VkContext->GetBufferDeviceAddress(InstancesBuffer);

    vk::AccelerationStructureGeometryKHR AccelerationStructureGeometry;
    AccelerationStructureGeometry.geometryType = vk::GeometryTypeKHR::eInstances;
    AccelerationStructureGeometry.flags = vk::GeometryFlagBitsKHR::eOpaque;
    AccelerationStructureGeometry.geometry.instances.arrayOfPointers =VK_FALSE;
    AccelerationStructureGeometry.geometry.instances.data = InstanceDataDeviceAddress;
    AccelerationStructureGeometry.geometry.instances.sType = vk::StructureType::eAccelerationStructureGeometryInstancesDataKHR;
    
    vk::AccelerationStructureBuildGeometryInfoKHR AccelerationStructureBuildGeometryInfo;
    AccelerationStructureBuildGeometryInfo.type = vk::AccelerationStructureTypeKHR::eTopLevel;
    AccelerationStructureBuildGeometryInfo.flags =vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace;
    AccelerationStructureBuildGeometryInfo.geometryCount=1;
    AccelerationStructureBuildGeometryInfo.pGeometries = &AccelerationStructureGeometry;

    uint32_t PrimitivesCount = static_cast<uint32_t>(BLASInstances.size());

    vk::AccelerationStructureBuildSizesInfoKHR AccelerationStructureBuildSizesInfo;
    VkContext->_vkGetAccelerationStructureBuildSizesKHR(
        (VkDevice)VkContext->Device,
        VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
        (VkAccelerationStructureBuildGeometryInfoKHR*)&AccelerationStructureBuildGeometryInfo,
        &PrimitivesCount,
        (VkAccelerationStructureBuildSizesInfoKHR*)&AccelerationStructureBuildSizesInfo
    );

    CreateAS(AccelerationStructureBuildSizesInfo, vk::AccelerationStructureTypeKHR::eTopLevel);

    bufferHandle ScratchBufferHandle = Context->CreateBuffer(AccelerationStructureBuildSizesInfo.buildScratchSize, gfx::bufferUsage::StorageBuffer | gfx::bufferUsage::ShaderDeviceAddress, gfx::memoryUsage::GpuOnly);

    vk::AccelerationStructureBuildGeometryInfoKHR AccelerationBuildGeometryInfo;
    AccelerationBuildGeometryInfo.type = vk::AccelerationStructureTypeKHR::eTopLevel;
    AccelerationBuildGeometryInfo.flags = vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace;
    AccelerationBuildGeometryInfo.mode = vk::BuildAccelerationStructureModeKHR::eBuild;
    AccelerationBuildGeometryInfo.dstAccelerationStructure = AccelerationStructure;
    AccelerationBuildGeometryInfo.geometryCount=1;
    AccelerationBuildGeometryInfo.pGeometries=&AccelerationStructureGeometry;
    AccelerationBuildGeometryInfo.scratchData.deviceAddress = VkContext->GetBufferDeviceAddress(ScratchBufferHandle);
    

    VkAccelerationStructureBuildRangeInfoKHR AccelerationStructureBuildRangeInfo {};
    AccelerationStructureBuildRangeInfo.primitiveCount = PrimitivesCount;
    AccelerationStructureBuildRangeInfo.primitiveOffset = 0;
    AccelerationStructureBuildRangeInfo.firstVertex=0;
    AccelerationStructureBuildRangeInfo.transformOffset=0;
    std::vector<VkAccelerationStructureBuildRangeInfoKHR*> AccelerationStructureBuildRangeInfos = {&AccelerationStructureBuildRangeInfo};
    
    auto CommandBuffer = Context->GetImmediateCommandBuffer();
    CommandBuffer->Begin();

    GET_API_DATA(VkCommandBufferData, vkCommandBufferData, CommandBuffer);
    vk::CommandBuffer VkCommandBufferHandle = VkCommandBufferData->Handle;
    VkContext->_vkCmdBuildAccelerationStructuresKHR(
            (VkCommandBuffer)VkCommandBufferHandle,
            1,
            (VkAccelerationStructureBuildGeometryInfoKHR*)&AccelerationBuildGeometryInfo,
            AccelerationStructureBuildRangeInfos.data());
    CommandBuffer->End();

    Context->SubmitCommandBufferImmediate(CommandBuffer);

    Context->DestroyBuffer(ScratchBufferHandle);
}

}