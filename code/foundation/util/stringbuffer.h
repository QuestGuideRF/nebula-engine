#pragma once
#include "core/types.h"
#include "util/array.h"
namespace Util
{
class StringBuffer
{
public:
    StringBuffer();
    ~StringBuffer();
    void Setup(SizeT size);
    void Discard();
    bool IsValid() const;
    const char* AddString(const char* str);
    const char* NextString(const char* prev);
    SizeT GetNumChunks() const;
private:
    void AllocNewChunk();
    Util::Array<char*> chunks;
    SizeT chunkSize;
    char* curPointer;
};
inline bool
StringBuffer::IsValid() const
{
    return (0 != this->curPointer);
}
inline SizeT
StringBuffer::GetNumChunks() const
{
    return this->chunks.Size();
}
}