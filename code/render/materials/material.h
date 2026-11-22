#pragma once
#include "ids/id.h"
#include "ids/idallocator.h"
#include "resources/resourceid.h"
#include "coregraphics/texture.h"
#include "coregraphics/buffer.h"
#include "materialvariant.h"
namespace MaterialTemplates
{
struct Entry;
enum class BatchGroup;
};
namespace Materials
{
struct ShaderConfigBatchTexture;
RESOURCE_ID_TYPE(MaterialId);
ID_32_24_8_NAMED_TYPE(MaterialInstanceId, instance, materialId, materialGeneration, material);
typedef IndexT BatchIndex;
enum BindlessBufferDirtyBits
{
    Graphics = 0x1,
    Compute = 0x2,
    All = Graphics | Compute
};
#ifdef WITH_NEBULA_EDITOR
struct MaterialBindlessBufferBinding
{
    char* buffer = nullptr;
    bool* dirtyFlag = nullptr;
};
#endif
MaterialId CreateMaterial(const MaterialTemplates::Entry* entry);
void DestroyMaterial(const MaterialId id);
void MaterialSetTexture(const MaterialId mat, const ShaderConfigBatchTexture* bind, const Resources::ResourceId tex);
void MaterialSetTexture(const MaterialId mat, uint name, const Resources::ResourceId tex);
void MaterialSetTextureBindless(const MaterialId mat, uint name, const uint handle, const uint offset, const Resources::ResourceId tex);
void MaterialSetConstants(const MaterialId mat, const void* data, const uint size);
void MaterialSetConstant(const MaterialId mat, const void* data, const uint size, const uint offset);
void MaterialSetBufferBinding(const MaterialId id, IndexT index);
IndexT MaterialGetBufferBinding(const MaterialId id);
void MaterialAddLODTexture(const MaterialId mat, const Resources::ResourceId tex);
void MaterialSetLowestLod(const MaterialId mat, float lod);
void MaterialApply(const MaterialId id, const CoreGraphics::CmdBufferId buf, IndexT index);
const MaterialTemplates::Entry* MaterialGetTemplate(const MaterialId mat);
const Materials::BatchIndex MaterialGetBatchIndex(const MaterialId mat, const MaterialTemplates::BatchGroup batch);
uint64_t MaterialGetSortCode(const MaterialId mat);
#ifdef WITH_NEBULA_EDITOR
void MaterialBindlessForEditor(const MaterialId mat, char* buf, bool* dirtyFlag);
const MaterialBindlessBufferBinding& MaterialGetBindlessForEditor(const MaterialId mat);
ubyte* MaterialGetConstants(const MaterialId mat);
const Resources::ResourceId MaterialGetTexture(const MaterialId mat, const IndexT i);
void MaterialInvalidate(const MaterialId mat);
#endif
struct MaterialConstant
{
    MaterialVariant defaultValue;
    IndexT bufferIndex;
    bool instanceConstant : 1;
    IndexT binding;
    CoreGraphics::BufferId buffer;
    MaterialConstant()
        : buffer(CoreGraphics::InvalidBufferId)
    {}
};
struct MaterialTexture
{
    CoreGraphics::TextureId defaultValue;
    IndexT slot;
};
enum
{
    Material_MinLOD,
    Material_LODTextures,
    Material_Table,
    Material_Buffer,
    Material_InstanceTables,
    Material_InstanceBuffers,
    Material_Textures,
    Material_Constants,
    Material_BufferOffset,
    Material_Template
#ifdef WITH_NEBULA_EDITOR
    , Material_TextureValues
    , Material_BufferPointer
#endif
};
typedef Ids::IdAllocator<
    float,
    Util::Array<Resources::ResourceId>,
    Util::FixedArray<CoreGraphics::ResourceTableId>,
    CoreGraphics::BufferId,
    Util::FixedArray<Util::FixedArray<CoreGraphics::ResourceTableId>>,
    Util::FixedArray<Util::Tuple<IndexT, SizeT>>,
    Util::FixedArray<Util::Array<MaterialTexture>>,
    Util::FixedArray<Util::Array<MaterialConstant>>,
    IndexT,
    const MaterialTemplates::Entry*
#ifdef WITH_NEBULA_EDITOR
    , Util::Array<Resources::ResourceId>
    , MaterialBindlessBufferBinding
#endif
> MaterialAllocator;
extern MaterialAllocator materialAllocator;
MaterialInstanceId CreateMaterialInstance(const MaterialId material);
void DestroyMaterialInstance(const MaterialInstanceId materialInstance);
CoreGraphics::ConstantBufferOffset MaterialInstanceAllocate(const MaterialInstanceId mat, const BatchIndex batch);
void MaterialInstanceApply(const MaterialInstanceId id, const CoreGraphics::CmdBufferId buf, IndexT index, IndexT bufferIndex);
SizeT MaterialInstanceBufferSize(const MaterialInstanceId sur, const BatchIndex batch);
enum MaterialInstanceMembers
{
    MaterialInstance_Offsets
};
typedef Ids::IdAllocator<
    uint
> MaterialInstanceAllocator;
extern MaterialInstanceAllocator materialInstanceAllocator;
}