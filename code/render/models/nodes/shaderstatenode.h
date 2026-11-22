#pragma once
#include "modelnode.h"
#include "resources/resourceid.h"
#include "transformnode.h"
#include "coregraphics/resourcetable.h"
namespace Models
{
class ShaderStateNode : public TransformNode
{
public:
    ShaderStateNode();
    virtual ~ShaderStateNode();
    struct DrawPacket;
    static const uint NumMaxOffsets = 4;
    struct DrawPacket
    {
#ifndef PUBLIC_BUILD
        Math::bbox boundingBox;
        uint32_t nodeInstanceHash;
#endif
        Materials::MaterialInstanceId materialInstance;
        CoreGraphics::ResourceTableId table;
        uint32_t offsets[NumMaxOffsets];
        uint8_t numOffsets;
        uint8_t slot;
        void Apply(const CoreGraphics::CmdBufferId cmdBuf, IndexT batchIndex, IndexT bufferIndex);
    };
    const Materials::MaterialId GetMaterial() const { return this->material; };
    static Util::FixedArray<CoreGraphics::ResourceTableId> CreateResourceTables();
protected:
    friend class ModelContext;
    friend class ModelLoader;
    bool Load(const Util::FourCC& fourcc, const Util::StringAtom& tag, const Ptr<IO::BinaryReader>& reader, bool immediate) override;
    virtual void Unload() override;
    virtual void OnFinishedLoading(ModelStreamingData* streamingData) override;
    Resources::ResourceId materialRes;
    Materials::MaterialId material;
    Resources::ResourceName materialName;
    Util::FixedArray<CoreGraphics::ResourceTableId> resourceTables;
};
}