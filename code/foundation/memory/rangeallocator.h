#pragma once
#include "core/types.h"
#include "util/bit.h"
#include "util/array.h"
#include <utility>
namespace Memory
{
struct RangeAllocation
{
    uint offset;
    uint size;
    uint node;
    static constexpr uint OOM = 0xFFFFFFFF;
};
class RangeAllocator
{
public:
    RangeAllocator();
    RangeAllocator(uint size, SizeT maxNumAllocs);
    ~RangeAllocator();
    void Clear();
    bool Empty();
    RangeAllocation Alloc(uint size, uint alignment = 1);
    void Dealloc(const RangeAllocation& allocation);
private:
    uint InsertNode(uint size, uint offset);
    void RemoveNode(uint nodeIndex);
    struct RangeAllocatorNode
    {
        RangeAllocatorNode() :
            resident(false)
            , size(0)
            , offset(0)
            , binPrev(END)
            , binNext(END)
            , blockPrev(END)
            , blockNext(END)
        {};
        bool resident;
        uint size;
        uint offset;
        static constexpr uint END = 0xFFFFFFFF;
        uint binPrev, binNext;
        uint blockPrev, blockNext;
    };
    union BinIndex
    {
        BinIndex() : index(0) {};
        struct
        {
            uint16_t bin : 4;
            uint16_t bucket : 12;
        };
        uint16_t index;
    };
    static BinIndex IndexFromSize(uint size, bool round = false);
    static uint BucketFromSize(uint size);
    static uint BinFromSize(uint size, uint bucket);
    uint size;
    uint freeStorage;
    uint freeNodeIterator;
    static constexpr uint NUM_BUCKETS = 0x20;
    static constexpr uint NUM_BINS_PER_BUCKET = 0x10;
    uint bucketUsageMask;
    uint binMasks[NUM_BUCKETS];
    uint binHeads[NUM_BUCKETS * NUM_BINS_PER_BUCKET];
    Util::Array<RangeAllocatorNode> nodes;
    Util::Array<uint> freeNodes;
};
inline
RangeAllocator::RangeAllocator() :
    RangeAllocator(0,0)
{
}
inline
RangeAllocator::RangeAllocator(uint size, SizeT maxNumAllocs)
{
    this->size = size;
    this->freeNodes.Resize(maxNumAllocs);
    this->nodes.Resize(maxNumAllocs);
    this->Clear();
}
inline
RangeAllocator::~RangeAllocator()
{
}
inline void
RangeAllocator::Clear()
{
    this->bucketUsageMask = 0x0;
    memset(this->binMasks, 0x0, sizeof(this->binMasks));
    for (uint i = 0; i < NUM_BUCKETS * NUM_BINS_PER_BUCKET; i++)
    {
        this->binHeads[i] = RangeAllocatorNode::END;
    }
    for (uint i = 0; i < this->nodes.Size(); i++)
    {
        this->freeNodes[i] = this->nodes.Size() - i - 1;
    }
    this->freeNodeIterator = this->nodes.Size() - 1;
    this->freeStorage = 0;
    if (this->size > 0)
        this->InsertNode(this->size, 0);
}
inline bool
RangeAllocator::Empty()
{
    return this->freeStorage == this->size;
}
inline RangeAllocation
RangeAllocator::Alloc(uint size, uint alignment)
{
    uint alignedSize = size + alignment - 1;
    if (this->freeStorage < alignedSize)
    {
        return RangeAllocation{ .offset = RangeAllocation::OOM, .size = 0, .node = RangeAllocatorNode::END };
    }
    BinIndex minIndex = IndexFromSize(alignedSize, true);
    uint bin = 0xFFFFFFFF;
    uint bucket = minIndex.bucket;
    if (this->bucketUsageMask & (1 << bucket))
    {
        bin = Util::Lsb(this->binMasks[bucket], minIndex.bin);
    }
    if (bin == 0xFFFFFFFF)
    {
        bucket = Util::Lsb(this->bucketUsageMask, bucket + 1);
        if (bucket == 0xFFFFFFFF)
        {
            return RangeAllocation{ .offset = RangeAllocation::OOM, .size = 0, .node = RangeAllocatorNode::END };
        }
        bin = Util::FirstOne(this->binMasks[bucket]);
    }
    BinIndex binIndex;
    binIndex.bucket = bucket;
    binIndex.bin = bin;
    uint nodeIndex = this->binHeads[binIndex.index];
    RangeAllocatorNode& node = this->nodes[nodeIndex];
    n_assert(!node.resident);
    uint totalSize = node.size;
    uint baseOffset = node.offset;
    node.size = alignedSize;
    node.resident = true;
    node.offset = Math::align(node.offset, alignment);
    this->binHeads[binIndex.index] = node.binNext;
    if (node.binNext != RangeAllocatorNode::END)
        this->nodes[node.binNext].binPrev = RangeAllocatorNode::END;
    if (this->binHeads[binIndex.index] == RangeAllocatorNode::END)
    {
        this->binMasks[binIndex.bucket] &= ~(1 << binIndex.bin);
        if (this->binMasks[binIndex.bucket] == 0)
        {
            this->bucketUsageMask &= ~(1 << binIndex.bucket);
        }
    }
    this->freeStorage -= totalSize;
    uint remainder = totalSize - node.size;
    if (remainder > 0)
    {
        uint newNodeIndex = this->InsertNode(remainder, node.offset + size);
        if (newNodeIndex != RangeAllocatorNode::END)
        {
            RangeAllocatorNode& newNode = this->nodes[newNodeIndex];
            if (node.blockNext != RangeAllocatorNode::END)
            {
                this->nodes[node.blockNext].blockPrev = newNodeIndex;
            }
            newNode.blockPrev = nodeIndex;
            newNode.blockNext = node.blockNext;
            node.blockNext = newNodeIndex;
        }
    }
    return RangeAllocation{ .offset = node.offset, .size = alignedSize, .node = nodeIndex };
}
inline void
RangeAllocator::Dealloc(const RangeAllocation& allocation)
{
    RangeAllocatorNode& node = this->nodes[allocation.node];
    n_assert(node.resident);
    node.resident = false;
    uint size = node.size;
    uint offset = node.offset;
    if (node.blockPrev != RangeAllocatorNode::END)
    {
        RangeAllocatorNode& prev = this->nodes[node.blockPrev];
        if (!prev.resident)
        {
            offset = prev.offset;
            size += prev.size;
            this->RemoveNode(node.blockPrev);
            node.blockPrev = prev.blockPrev;
        }
    }
    if (node.blockNext != RangeAllocatorNode::END)
    {
        RangeAllocatorNode& next = this->nodes[node.blockNext];
        if (!next.resident)
        {
            size += next.size;
            this->RemoveNode(node.blockNext);
            node.blockNext = next.blockNext;
        }
    }
    uint leftNode = node.blockPrev;
    uint rightNode = node.blockNext;
    this->freeNodes[++this->freeNodeIterator] = allocation.node;
    uint mergedNode = this->InsertNode(size, offset);
    if (leftNode != RangeAllocatorNode::END)
    {
        this->nodes[leftNode].blockNext = mergedNode;
        this->nodes[mergedNode].blockPrev = leftNode;
    }
    if (rightNode != RangeAllocatorNode::END)
    {
        this->nodes[rightNode].blockPrev = mergedNode;
        this->nodes[mergedNode].blockNext = rightNode;
    }
}
inline uint
RangeAllocator::InsertNode(uint size, uint offset)
{
    if (this->freeNodeIterator == 0xFFFFFFFF)
        return RangeAllocatorNode::END;
    BinIndex index = IndexFromSize(size);
    if (this->binHeads[index.index] == RangeAllocatorNode::END)
    {
        this->binMasks[index.bucket] |= (1 << index.bin);
        this->bucketUsageMask |= (1 << index.bucket);
    }
    uint headNodeIndex = this->binHeads[index.index];
    uint newNodeIndex = this->freeNodes[this->freeNodeIterator--];
    RangeAllocatorNode& newNode = this->nodes[newNodeIndex];
    newNode.size = size;
    newNode.offset = offset;
    newNode.binNext = headNodeIndex;
    newNode.binPrev = RangeAllocatorNode::END;
    newNode.resident = false;
    this->freeStorage += size;
    if (headNodeIndex != RangeAllocatorNode::END)
    {
        RangeAllocatorNode& headNode = this->nodes[headNodeIndex];
        headNode.binPrev = newNodeIndex;
    }
    this->binHeads[index.index] = newNodeIndex;
    return newNodeIndex;
}
inline void
RangeAllocator::RemoveNode(uint nodeIndex)
{
    RangeAllocatorNode& node = this->nodes[nodeIndex];
    node.resident = false;
    this->freeStorage -= node.size;
    if (node.binPrev == RangeAllocatorNode::END)
    {
        BinIndex index = IndexFromSize(node.size);
        this->binHeads[index.index] = node.binNext;
        if (node.binNext != RangeAllocatorNode::END)
        {
            this->nodes[node.binNext].binPrev = RangeAllocatorNode::END;
        }
        if (node.binNext == RangeAllocatorNode::END)
        {
            this->binMasks[index.bucket] &= ~(1 << index.bin);
            if (this->binMasks[index.bucket] == 0x0)
            {
                this->bucketUsageMask &= ~(1 << index.bucket);
            }
        }
    }
    else
    {
        this->nodes[node.binPrev].binNext = node.binNext;
        if (node.binNext != RangeAllocatorNode::END)
        {
            this->nodes[node.binNext].binPrev = node.binPrev;
        }
    }
    this->freeNodes[++this->freeNodeIterator] = nodeIndex;
}
inline uint
RangeAllocator::BucketFromSize(uint size)
{
#if __WIN32__
    DWORD count = 0;
    _BitScanReverse(&count, size);
#else
    int count = 31 - __builtin_clz(size);
#endif
    return count;
}
inline uint
RangeAllocator::BinFromSize(uint size, uint bucket)
{
    uint mask = (size >> (bucket - 4)) & (NUM_BINS_PER_BUCKET - 1u);
    return mask;
}
inline RangeAllocator::BinIndex
RangeAllocator::IndexFromSize(uint size, bool round)
{
    RangeAllocator::BinIndex ret;
    ret.bucket = BucketFromSize(size);
    ret.bin = BinFromSize(size, ret.bucket);
    if (round)
    {
        uint mask = (1 << ret.bucket) | (ret.bin << (ret.bucket - 4));
        if ((~mask & size) != 0)
            ret.bin++;
    }
    return ret;
}
}