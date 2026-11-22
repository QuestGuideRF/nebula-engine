#pragma once
#include "resources/resourceloader.h"
#include "coregraphics/gpubuffertypes.h"
#include "coregraphics/meshresource.h"
#include "coregraphics/mesh.h"
namespace CoreGraphics
{
class MeshLoader : public Resources::ResourceLoader
{
    __DeclareClass(MeshLoader);
public:
    MeshLoader();
    virtual ~MeshLoader();
	struct StreamMeshLoadMetaData
	{
		bool copySource;
	};
    static const CoreGraphics::VertexLayoutId GetLayout(const CoreGraphics::VertexLayoutType type);
private:
    struct MeshStreamData
    {
        void* mappedData;
        CoreGraphics::VertexAlloc indexAllocationOffset, vertexAllocationOffset;
    };
    ResourceInitOutput InitializeResource(const ResourceLoadJob& job, const Ptr<IO::Stream>& stream) override;
    ResourceStreamOutput StreamResource(const ResourceLoadJob& job) override;
    void Unload(const Resources::ResourceId id) override;
    uint LodMask(const _StreamData& stream, float lod, bool async) const override;
    void UpdateLoaderSyncState() override;
    ResourceLoader::_StreamData SetupMeshFromNvx(const Ptr<IO::Stream>& stream, const ResourceLoadJob& job, const MeshResourceId meshResource);
    struct MeshesToSubmit
    {
        Resources::ResourceId id;
        uint bits;
        Util::Array<Memory::RangeAllocation> rangesToFlush;
        CoreGraphics::CmdBufferId cmdBuf;
    };
    Threading::SafeQueue<MeshesToSubmit> meshesToSubmit;
    struct FinishedMesh
    {
        uint64_t submissionId;
        uint bits;
        Util::Array<Memory::RangeAllocation> rangesToFree;
        CoreGraphics::CmdBufferId cmdBuf;
    };
    Util::HashTable<Resources::ResourceId, Util::Array<FinishedMesh>> meshesToFinish;
    Threading::CriticalSection meshLock;
    CoreGraphics::CmdBufferPoolId asyncTransferPool, immediateTransferPool;
};
}