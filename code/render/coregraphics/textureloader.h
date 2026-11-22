#pragma once
#include "resources/resourceloader.h"
#include "gliml.h"
namespace CoreGraphics
{
struct TextureStreamData;
class TextureLoader : public Resources::ResourceLoader
{
    __DeclareClass(TextureLoader);
public:
    TextureLoader();
    virtual ~TextureLoader();
private:
    friend void FinishMips(TextureLoader* loader, TextureStreamData* streamData, uint mipBits, const CoreGraphics::TextureId texture, const char* name);
    ResourceLoader::ResourceInitOutput InitializeResource(const ResourceLoadJob& job, const Ptr<IO::Stream>& stream) override;
    ResourceLoader::ResourceStreamOutput StreamResource(const ResourceLoadJob& job) override;
    void Unload(const Resources::ResourceId id) override;
    uint LodMask(const _StreamData& stream, float lod, bool async) const override;
    void UpdateLoaderSyncState() override;
    struct MipLoadMainThread
    {
        Resources::ResourceId id;
        uint bits;
        Util::Array<Memory::RangeAllocation> rangesToFlush;
        CoreGraphics::CmdBufferId transferCmdBuf, graphicsCmdBuf;
    };
    Threading::SafeQueue<MipLoadMainThread> mipLoadsToSubmit;
    struct MipHandoverLoaderThread
    {
        uint64_t handoverSubmissionId;
        uint bits;
        Util::Array<Memory::RangeAllocation> rangesToFree;
        CoreGraphics::CmdBufferId uploadBuffer, receiveBuffer;
    };
    Util::HashTable<Resources::ResourceId, Util::Array<MipHandoverLoaderThread>> mipHandovers;
    Threading::CriticalSection handoverLock;
    CoreGraphics::CmdBufferPoolId asyncTransferPool, immediateTransferPool, asyncHandoverPool, immediateHandoverPool;
};
}