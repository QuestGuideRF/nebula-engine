#pragma once
#include "core/config.h"
#include "io/stream.h"
namespace IO
{
class StreamCache;
class CachedStream : public IO::Stream
{
public:
    CachedStream();
    virtual ~CachedStream();
    virtual bool CanRead() const;
    virtual bool CanSeek() const;
    virtual bool CanBeMapped() const;
    virtual Size GetSize() const;
    virtual Position GetPosition() const;
    virtual bool Open();
    virtual void Close();
    virtual Size Read(void* ptr, Size numBytes);
    virtual void Seek(Offset offset, SeekOrigin origin);
    virtual bool Eof() const;
    virtual void* Map();
    virtual void* MemoryMap();
    virtual void Unmap();
    void* GetRawPointer() const;
protected:
    friend class IO::StreamCache;
    virtual Core::Rtti const& GetParentRtti() = 0;
    Ptr<IO::Stream> parentStream;
    Size size;
    Position position;
    unsigned char* buffer;
};
}