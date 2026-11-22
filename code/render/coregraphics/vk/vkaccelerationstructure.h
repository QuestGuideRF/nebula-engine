#pragma once
#include "ids/idallocator.h"
#include "coregraphics/accelerationstructure.h"
namespace Vulkan
{
struct GeometrySetup
{
    VkAccelerationStructureBuildSizesInfoKHR buildSizes;
    VkAccelerationStructureBuildGeometryInfoKHR buildGeometryInfo;
    VkAccelerationStructureGeometryKHR geometry;
    VkAccelerationStructureBuildRangeInfoKHR primitiveGroup;
};
enum
{
    Blas_Device,
    Blas_Handle,
    Blas_Buffer,
    Blas_Scratch,
    Blas_Geometry,
    Blas_View
};
typedef Ids::IdAllocatorSafe<
    0xFFF
    , VkDevice
    , VkAccelerationStructureKHR
    , CoreGraphics::BufferId
    , CoreGraphics::BufferId
    , GeometrySetup
    , VkDeviceAddress
> VkBlasAllocator;
extern VkBlasAllocator blasAllocator;
const VkDevice BlasGetVkDevice(const CoreGraphics::BlasId id);
const VkBuffer BlasGetVkBuffer(const CoreGraphics::BlasId id);
const VkAccelerationStructureKHR BlasGetVk(const CoreGraphics::BlasId id);
const VkAccelerationStructureBuildGeometryInfoKHR& BlasGetVkBuild(const CoreGraphics::BlasId id);
const VkAccelerationStructureBuildRangeInfoKHR& BlasGetVkRanges(const CoreGraphics::BlasId id);
struct InstanceSetup
{
    VkAccelerationStructureGeometryInstancesDataKHR instanceData;
    VkAccelerationStructureBuildGeometryInfoKHR geometryInfo;
};
enum
{
    BlasInstance_Instance,
    BlasInstance_Transform,
};
typedef Ids::IdAllocatorSafe<
    0xFFFF
    , VkAccelerationStructureInstanceKHR
    , Math::mat4
    , uint
> VkBlasInstanceAllocator;
extern VkBlasInstanceAllocator blasInstanceAllocator;
struct SceneSetup
{
    VkAccelerationStructureGeometryKHR geo;
    VkAccelerationStructureBuildGeometryInfoKHR geometryInfo;
    VkAccelerationStructureBuildSizesInfoKHR buildSizes;
    Util::Array<VkAccelerationStructureBuildRangeInfoKHR> rangeInfos;
};
enum
{
    Tlas_Device,
    Tlas_Scene,
    Tlas_Handle,
    Tlas_Buffer,
    Tlas_BuildScratch,
    Tlas_UpdateScratch,
    Tlas_BuildScratchAddr,
    Tlas_UpdateScratchAddr,
};
typedef Ids::IdAllocatorSafe<
    0xFFF
    , VkDevice
    , SceneSetup
    , VkAccelerationStructureKHR
    , CoreGraphics::BufferId
    , CoreGraphics::BufferId
    , CoreGraphics::BufferId
    , VkDeviceAddress
    , VkDeviceAddress
> VkTlasAllocator;
extern VkTlasAllocator tlasAllocator;
const VkDevice TlasGetVkDevice(const CoreGraphics::TlasId id);
const VkBuffer TlasGetVkBuffer(const CoreGraphics::TlasId id);
const VkAccelerationStructureKHR TlasGetVk(const CoreGraphics::TlasId id);
const VkAccelerationStructureBuildGeometryInfoKHR& TlasGetVkBuild(const CoreGraphics::TlasId id);
const Util::Array<VkAccelerationStructureBuildRangeInfoKHR>& TlasGetVkRanges(const CoreGraphics::TlasId id);
}