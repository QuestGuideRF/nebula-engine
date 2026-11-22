#pragma once
#include "core/config.h"
#include "io/streamreader.h"
#include "coregraphics/vertexbuffer.h"
#include "coregraphics/indexbuffer.h"
#include "coregraphics/primitivegroup.h"
#include "coregraphics/base/vertexbufferbase.h"
#include "coregraphics/base/memoryvertexbufferloaderbase.h"
#include "coregraphics/base/indexbufferbase.h"
#include "coregraphics/base/memoryindexbufferloaderbase.h"
namespace CoreGraphics
{
class Nvx3StreamReader : public IO::StreamReader
{
    __DeclareClass(Nvx3StreamReader);
public:
    Nvx3StreamReader();
    virtual ~Nvx3StreamReader();
    void SetRawMode(bool b);
    bool IsRawMode() const;
    void SetUsage(Base::ResourceBase::Usage usage);
    Base::ResourceBase::Usage GetUsage() const;
    void SetAccess(Base::ResourceBase::Access access);
    Base::ResourceBase::Access GetAccess() const;
    virtual bool Open();
    virtual void Close();
    const Ptr<Base::VertexBufferBase>& GetVertexBuffer() const;
    const Ptr<Base::IndexBufferBase>& GetIndexBuffer() const;
    const Util::Array<CoreGraphics::PrimitiveGroup>& GetPrimitiveGroups() const;
    float* GetVertexData() const;
    ushort* GetIndexData() const;
    SizeT GetNumVertices() const;
    SizeT GetNumIndices() const;
    SizeT GetVertexWidth() const;
    const Util::Array<CoreGraphics::VertexComponent>& GetVertexComponents() const;
    void SetVertexBuffer(const Ptr<Base::VertexBufferBase>& vBuf);
    void SetVertexBufferLoader(const Ptr<Base::MemoryVertexBufferLoaderBase>& vBufLoader);
    static void ConvertVertexBufferEndianess(void* vertexPtr, SizeT numVertices, const Util::Array<CoreGraphics::VertexComponent>& vertexComps);
private:
    void ReadHeaderData();
    void ReadPrimitiveGroups();
    void SetupVertexComponents();
    void UpdateGroupBoundingBoxes();
    void SetupVertexBuffer();
    void SetupIndexBuffer();
    enum N3VertexComponent
    {
        N3Coord        = (1<<0),
        N3Normal       = (1<<1),
        N3NormalUB4N   = (1<<2),
        N3Uv0          = (1<<3),
        N3Uv0S2        = (1<<4),
        N3Uv1          = (1<<5),
        N3Uv1S2        = (1<<6),
        N3Uv2          = (1<<7),
        N3Uv2S2        = (1<<8),
        N3Uv3          = (1<<9),
        N3Uv3S2        = (1<<10),
        N3Color        = (1<<11),
        N3ColorUB4N    = (1<<12),
        N3Tangent      = (1<<13),
        N3TangentUB4N  = (1<<14),
        N3Binormal     = (1<<15),
        N3BinormalUB4N = (1<<16),
        N3Weights      = (1<<17),
        N3WeightsUB4N  = (1<<18),
        N3JIndices     = (1<<19),
        N3JIndicesUB4  = (1<<20),
        N3NumVertexComponents = 21,
        N3AllComponents = ((1<<N3NumVertexComponents) - 1),
    };
    Base::ResourceBase::Usage usage;
    Base::ResourceBase::Access access;
    bool rawMode;
    Ptr<Base::VertexBufferBase> vertexBuffer;
    Ptr<Base::IndexBufferBase> indexBuffer;
    Ptr<Base::MemoryVertexBufferLoaderBase> vertexBufferLoader;
    Ptr<Base::MemoryIndexBufferLoaderBase> indexBufferLoader;
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
    uint vertexComponentMask;
    Util::Array<CoreGraphics::VertexComponent> vertexComponents;
};
inline void
Nvx3StreamReader::SetVertexBuffer(const Ptr<Base::VertexBufferBase>& vBuf)
{
    this->vertexBuffer = vBuf;
}
inline void
Nvx3StreamReader::SetVertexBufferLoader(const Ptr<Base::MemoryVertexBufferLoaderBase>& vBufLoader)
{
    this->vertexBufferLoader = vBufLoader;
}
inline void
Nvx3StreamReader::SetRawMode(bool b)
{
    this->rawMode = b;
}
inline bool
Nvx3StreamReader::IsRawMode() const
{
    return this->rawMode;
}
inline const Ptr<Base::VertexBufferBase>&
Nvx3StreamReader::GetVertexBuffer() const
{
    n_assert(!this->rawMode && this->vertexBuffer.isvalid());
    return this->vertexBuffer;
}
inline const Ptr<Base::IndexBufferBase>&
Nvx3StreamReader::GetIndexBuffer() const
{
    n_assert(!this->rawMode && this->indexBuffer.isvalid());
    return this->indexBuffer;
}
inline const Util::Array<CoreGraphics::PrimitiveGroup>&
Nvx3StreamReader::GetPrimitiveGroups() const
{
    return this->primGroups;
}
inline float*
Nvx3StreamReader::GetVertexData() const
{
    return (float*) this->vertexDataPtr;
}
inline ushort*
Nvx3StreamReader::GetIndexData() const
{
    return (ushort*) this->indexDataPtr;
}
inline SizeT
Nvx3StreamReader::GetNumVertices() const
{
    return this->numVertices;
}
inline SizeT
Nvx3StreamReader::GetNumIndices() const
{
    return this->numIndices;
}
inline SizeT
Nvx3StreamReader::GetVertexWidth() const
{
    return this->vertexWidth;
}
inline const Util::Array<CoreGraphics::VertexComponent>&
Nvx3StreamReader::GetVertexComponents() const
{
    return this->vertexComponents;
}
inline void
Nvx3StreamReader::SetUsage(Base::ResourceBase::Usage usage_)
{
    this->usage = usage_;
}
inline Base::ResourceBase::Usage
Nvx3StreamReader::GetUsage() const
{
    return this->usage;
}
inline void
Nvx3StreamReader::SetAccess(Base::ResourceBase::Access access_)
{
    this->access = access_;
}
inline Base::ResourceBase::Access
Nvx3StreamReader::GetAccess() const
{
    return this->access;
}
}