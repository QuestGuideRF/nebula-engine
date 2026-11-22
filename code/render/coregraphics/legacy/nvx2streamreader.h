#pragma once
#include "core/config.h"
#if NEBULA_LEGACY_SUPPORT
#include "io/streamreader.h"
#include "coregraphics/buffer.h"
#include "coregraphics/primitivegroup.h"
namespace Legacy
{
class Nvx2StreamReader : public IO::StreamReader
{
    __DeclareClass(Nvx2StreamReader);
public:
    Nvx2StreamReader();
    virtual ~Nvx2StreamReader();
    void SetRawMode(bool b);
    bool IsRawMode() const;
    void SetUsage(CoreGraphics::GpuBufferTypes::Usage usage);
    CoreGraphics::GpuBufferTypes::Usage GetUsage() const;
    void SetAccess(CoreGraphics::GpuBufferTypes::Access access);
	CoreGraphics::GpuBufferTypes::Access GetAccess() const;
    void SetBuffersCopySource(bool b);
    bool Open(const Resources::ResourceName& name);
    void Close();
    const CoreGraphics::BufferId GetVertexBuffer() const;
    const CoreGraphics::BufferId GetIndexBuffer() const;
    const Util::Array<CoreGraphics::PrimitiveGroup>& GetPrimitiveGroups() const;
    float* GetVertexData() const;
    ushort* GetIndexData() const;
    SizeT GetNumVertices() const;
    SizeT GetNumIndices() const;
    SizeT GetVertexWidth() const;
    SizeT GetNumEdges() const;
    SizeT GetBaseVertexOffset() const;
    SizeT GetBaseIndexOffset() const;
    const Util::Array<CoreGraphics::VertexComponent>& GetVertexComponents() const;
    static void ConvertVertexBufferEndianess(void* vertexPtr, SizeT numVertices, const Util::Array<CoreGraphics::VertexComponent>& vertexComps);
private:
    void ReadHeaderData();
    void ReadPrimitiveGroups();
    void SetupVertexComponents();
    void UpdateGroupBoundingBoxes();
    void SetupVertexBuffer(const Resources::ResourceName& name);
    void SetupIndexBuffer(const Resources::ResourceName& name);
    enum N2VertexComponent
    {
        N2Coord        = (1<<0),
        N2Normal       = (1<<1),
        N2NormalB4N    = (1<<2),
        N2Uv0          = (1<<3),
        N2Uv0S2        = (1<<4),
        N2Uv1          = (1<<5),
        N2Uv1S2        = (1<<6),
        N2Uv2          = (1<<7),
        N2Uv2S2        = (1<<8),
        N2Uv3          = (1<<9),
        N2Uv3S2        = (1<<10),
        N2Tangent      = (1<<11),
        N2TangentB4N   = (1<<12),
        N2Binormal     = (1<<13),
        N2BinormalB4N  = (1<<14),
        N2Color        = (1<<15),
        N2ColorUB4N    = (1<<16),
        N2Weights      = (1<<17),
        N2WeightsUB4N  = (1<<18),
        N2JIndices     = (1<<19),
        N2JIndicesUB4  = (1<<20),
        N2NumVertexComponents = 21,
        N2AllComponents = ((1<<N2NumVertexComponents) - 1),
    };
    CoreGraphics::GpuBufferTypes::Usage usage;
    CoreGraphics::GpuBufferTypes::Access access;
    bool rawMode;
    Util::StringAtom tag;
    Resources::ResourceName name;
    CoreGraphics::BufferId vbo;
    CoreGraphics::BufferId ibo;
    CoreGraphics::VertexLayoutId layout;
    bool copySourceFlag;
    Util::Array<CoreGraphics::PrimitiveGroup> primGroups;
    void* mapPtr;
    void* groupDataPtr;
    void* vertexDataPtr;
    void* indexDataPtr;
    SizeT groupDataSize;
    SizeT vertexDataSize;
    SizeT indexDataSize;
    uint numGroups;
    uint numVertices;
    uint vertexWidth;
    uint numIndices;
    uint numEdges;
    uint vertexComponentMask;
    CoreGraphics::VertexAlloc vertexAllocation, indexAllocation;
    Util::Array<CoreGraphics::VertexComponent> vertexComponents;
};
inline void
Nvx2StreamReader::SetRawMode(bool b)
{
    this->rawMode = b;
}
inline bool
Nvx2StreamReader::IsRawMode() const
{
    return this->rawMode;
}
inline const CoreGraphics::BufferId
Nvx2StreamReader::GetVertexBuffer() const
{
    return this->vbo;
}
inline const CoreGraphics::BufferId
Nvx2StreamReader::GetIndexBuffer() const
{
    return this->ibo;
}
inline const Util::Array<CoreGraphics::PrimitiveGroup>&
Nvx2StreamReader::GetPrimitiveGroups() const
{
    return this->primGroups;
}
inline float*
Nvx2StreamReader::GetVertexData() const
{
    return (float*) this->vertexDataPtr;
}
inline ushort*
Nvx2StreamReader::GetIndexData() const
{
    return (ushort*) this->indexDataPtr;
}
inline SizeT
Nvx2StreamReader::GetNumVertices() const
{
    return this->numVertices;
}
inline SizeT
Nvx2StreamReader::GetNumIndices() const
{
    return this->numIndices;
}
inline SizeT
Nvx2StreamReader::GetVertexWidth() const
{
    return this->vertexWidth;
}
inline SizeT
Nvx2StreamReader::GetNumEdges() const
{
    return this->numEdges;
}
inline SizeT
Nvx2StreamReader::GetBaseVertexOffset() const
{
    return this->vertexAllocation.offset;
}
inline SizeT
Nvx2StreamReader::GetBaseIndexOffset() const
{
    return this->indexAllocation.offset;
}
inline const Util::Array<CoreGraphics::VertexComponent>&
Nvx2StreamReader::GetVertexComponents() const
{
    return this->vertexComponents;
}
inline void
Nvx2StreamReader::SetUsage(CoreGraphics::GpuBufferTypes::Usage usage_)
{
    this->usage = usage_;
}
inline CoreGraphics::GpuBufferTypes::Usage
Nvx2StreamReader::GetUsage() const
{
    return this->usage;
}
inline void
Nvx2StreamReader::SetAccess(CoreGraphics::GpuBufferTypes::Access access_)
{
    this->access = access_;
}
inline CoreGraphics::GpuBufferTypes::Access
Nvx2StreamReader::GetAccess() const
{
    return this->access;
}
inline void
Nvx2StreamReader::SetBuffersCopySource(bool b)
{
    this->copySourceFlag = b;
}
}
#endif