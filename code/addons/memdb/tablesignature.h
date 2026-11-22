#pragma once
#include "ids/id.h"
#include "util/fixedarray.h"
#include "attributeid.h"
namespace MemDb
{
class TableSignature
{
public:
    TableSignature() : mask(nullptr), size(0) {};
    TableSignature(TableSignature const& rhs);
    TableSignature(Util::FixedArray<AttributeId> const& descriptors);
    TableSignature(std::initializer_list<AttributeId> descriptors);
    TableSignature(AttributeId const* descriptors, SizeT num);
    ~TableSignature();
    TableSignature& operator=(TableSignature const& rhs);
    bool const operator==(TableSignature const& rhs) const;
    bool const IsValid() const { return size > 0; }
    bool const IsSet(AttributeId component) const;
    void FlipBit(AttributeId component);
    void SetBit(AttributeId component);
    void ClearBit(AttributeId component);
    static bool const CheckBits(TableSignature const& src, TableSignature const& mask);
    static bool const HasAny(TableSignature const& src, TableSignature const& mask);
protected:
    void Setup(AttributeId const* descriptors, SizeT num);
    __m128i* mask;
    uint8_t size;
};
inline
TableSignature::TableSignature(Util::FixedArray<AttributeId> const& descriptors)
{
    this->Setup(descriptors.Begin(), descriptors.Size());
}
inline
TableSignature::TableSignature(std::initializer_list<AttributeId> descriptors)
{
    this->Setup(descriptors.begin(), (SizeT)descriptors.size());
}
inline
TableSignature::TableSignature(AttributeId const* descriptors, SizeT num)
{
    this->Setup(descriptors, num);
}
inline
void
TableSignature::Setup(AttributeId const* componentBuffer, SizeT num)
{
    if (num > 0)
    {
        Util::Array<AttributeId> descriptors(componentBuffer, num);
        descriptors.Sort();
        AttributeId const last = *(descriptors.End() - 1);
        n_assert(last != AttributeId::Invalid());
        uint const largestBit = last.id;
        this->size = (largestBit / 128) + 1;
        this->mask = (__m128i*)Memory::Alloc(Memory::HeapType::ObjectHeap, this->size * 16);
        for (int i = 0; i < this->size; i++)
            this->mask[i] = _mm_setzero_si128();
        alignas(16) uint64_t partialMask[2] = { 0, 0 };
        uint64_t offset = descriptors[0].id / 128;
        uint64_t prevOffset = offset;
        for (AttributeId i : descriptors)
        {
            n_assert(i != AttributeId::Invalid());
            offset = i.id / 128;
            if (offset != prevOffset)
            {
                *(this->mask + prevOffset) = _mm_set_epi64x(partialMask[0], partialMask[1]);
                prevOffset = offset;
                memset(&partialMask, 0, sizeof(partialMask));
            }
            uint32_t mod = (i.id % 128);
            partialMask[mod / 64] |= 1ull << mod;
        }
        *(this->mask + offset) = _mm_set_epi64x(partialMask[0], partialMask[1]);
    }
    else
    {
        this->mask = nullptr;
        this->size = 0;
    }
}
inline
TableSignature::TableSignature(TableSignature const& rhs) :
    size(rhs.size)
{
    if (size > 0)
    {
        this->mask = (__m128i*)Memory::Alloc(Memory::HeapType::ObjectHeap, this->size * 16);
        for (int i = 0; i < this->size; i++)
            this->mask[i] = _mm_load_si128(rhs.mask + i);
    }
    else
    {
        this->mask = nullptr;
    }
};
inline
TableSignature::~TableSignature()
{
    if (this->mask != nullptr)
        Memory::Free(Memory::HeapType::ObjectHeap, this->mask);
}
inline bool const
TableSignature::operator==(TableSignature const& rhs) const
{
    if (this->size != rhs.size || this->size == 0 || rhs.size == 0)
    {
        return false;
    }
    for (int i = 0; i < this->size; i++)
    {
        __m128i temp = _mm_cmpeq_epi32(this->mask[i], rhs.mask[i]);
        if ((_mm_movemask_epi8(temp) != 0xFFFF))
            return false;
    };
    return true;
}
inline bool const
TableSignature::IsSet(AttributeId component) const
{
    int offset = component.id / 128;
    if (offset < this->size)
    {
        alignas(16) uint64_t partialMask[2] = { 0, 0 };
        uint64_t const bit = component.id % 128;
        partialMask[bit / 64] |= 1ull << bit;
        __m128i temp = _mm_set_epi64x(partialMask[0], partialMask[1]);
        int const isSet = !_mm_testz_si128(temp, this->mask[offset]);
        return isSet;
    }
    return false;
}
inline void
TableSignature::FlipBit(AttributeId component)
{
    int offset = component.id / 128;
    n_assert(offset < this->size);
    alignas(16) uint64_t partialMask[2] = { 0, 0 };
    uint64_t bit = component.id % 128;
    partialMask[bit / 64] |= 1ull << bit;
    __m128i temp = _mm_set_epi64x(partialMask[0], partialMask[1]);
    this->mask[offset] = _mm_xor_si128(this->mask[offset], temp);
}
inline void
TableSignature::SetBit(AttributeId component)
{
    int offset = component.id / 128;
    if (offset >= this->size)
    {
        uint8_t newSize = offset + 1;
        __m128i* newMask = (__m128i*)Memory::Alloc(Memory::HeapType::ObjectHeap, newSize * 16);
        if (this->mask != nullptr)
        {
            for (int i = 0; i < this->size; i++)
                newMask[i] = _mm_load_si128(this->mask + i);
            Memory::Free(Memory::HeapType::ObjectHeap, this->mask);
        }
        this->mask = newMask;
        this->size = newSize;
    }
    alignas(16) uint64_t partialMask[2] = {0, 0};
    uint64_t bit = component.id % 128;
    partialMask[bit / 64] |= 1ull << bit;
    __m128i temp = _mm_set_epi64x(partialMask[0], partialMask[1]);
    this->mask[offset] = _mm_or_si128(this->mask[offset], temp);
}
inline void
TableSignature::ClearBit(AttributeId component)
{
    int offset = component.id / 128;
    n_assert(offset < this->size);
    alignas(16) uint64_t partialMask[2] = {0, 0};
    uint64_t bit = component.id % 128;
    partialMask[bit / 64] |= 1ull << bit;
    __m128i temp = _mm_set_epi64x(partialMask[0], partialMask[1]);
    this->mask[offset] = _mm_andnot_si128(temp, this->mask[offset]);
}
inline TableSignature&
TableSignature::operator=(TableSignature const& rhs)
{
    this->size = rhs.size;
    if (this->size > 0)
    {
        this->mask = (__m128i*)Memory::Alloc(Memory::HeapType::ObjectHeap, this->size * 16);
        for (int i = 0; i < this->size; i++)
            this->mask[i] = _mm_load_si128(rhs.mask + i);
    }
    else
    {
        this->mask = nullptr;
    }
    return *this;
}
inline bool const
TableSignature::CheckBits(TableSignature const& src, TableSignature const& mask)
{
    if (src.size == 0 || mask.size == 0)
        return false;
#ifdef NEBULA_DEBUG
    {
        __m128i cmp = _mm_cmpeq_epi32(mask.mask[mask.size - 1], _mm_setzero_si128());
        n_assert(_mm_movemask_epi8(cmp) != 0xFFFF);
    }
#endif
    if (mask.size > src.size)
    {
        return false;
    }
    for (int i = 0; i < mask.size; i++)
    {
        __m128i temp = _mm_and_si128(src.mask[i], mask.mask[i]);
        __m128i cmp = _mm_cmpeq_epi32(temp, mask.mask[i]);
        if ((_mm_movemask_epi8(cmp) != 0xFFFF))
            return false;
    };
    return true;
}
inline bool const
TableSignature::HasAny(TableSignature const& src, TableSignature const& mask)
{
    if (src.size == 0 || mask.size == 0)
        return false;
#ifdef NEBULA_DEBUG
    {
        __m128i cmp = _mm_cmpeq_epi32(mask.mask[mask.size - 1], _mm_setzero_si128());
        n_assert(_mm_movemask_epi8(cmp) != 0xFFFF);
    }
#endif
    const int size = Math::min(mask.size, src.size);
    for (int i = 0; i < size; i++)
    {
        __m128i temp = _mm_and_si128(src.mask[i], mask.mask[i]);
        __m128i cmp = _mm_cmpeq_epi32(temp, _mm_setzero_si128());
        if ((_mm_movemask_epi8(cmp) == 0xFFFF))
            return false;
    };
    return true;
}
}