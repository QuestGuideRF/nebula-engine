#pragma once
#ifndef IO_STREAM_H
#define IO_STREAM_H
#include "core/refcounted.h"
#include "io/uri.h"
#include "threading/criticalsection.h"
#include "io/mediatype.h"
namespace IO
{
class Stream : public Core::RefCounted
{
    __DeclareClass(Stream);
public:
    typedef int64_t Position;
    typedef int64_t Offset;
    typedef int64_t Size;
    enum AccessMode
    {
        ReadAccess,
        WriteAccess,
        AppendAccess,
        ReadWriteAccess,
    };
    enum AccessPattern
    {
        Random,
        Sequential,
    };
    enum SeekOrigin
    {
        Begin,
        Current,
        End,
    };
    Stream();
    virtual ~Stream();
    void SetURI(const URI& u);
    const URI& GetURI() const;
    virtual bool CanRead() const;
    virtual bool CanWrite() const;
    virtual bool CanSeek() const;
    virtual bool CanBeMapped() const;
    virtual void SetSize(Size s);
    virtual Size GetSize() const;
    virtual Position GetPosition() const;
    void SetAccessMode(AccessMode m);
    AccessMode GetAccessMode() const;
    void SetAccessPattern(AccessPattern p);
    AccessPattern GetAccessPattern() const;
    void SetMediaType(const MediaType& t);
    const MediaType& GetMediaType() const;
    virtual bool Open();
    virtual void Close();
    bool IsOpen() const;
    virtual void Write(const void* ptr, Size numBytes);
    virtual Size Read(void* ptr, Size numBytes);
    virtual void Seek(Offset offset, SeekOrigin origin);
    virtual void Flush();
    virtual bool Eof() const;
    virtual void* Map();
    virtual void Unmap();
    virtual void* MemoryMap();
    virtual void MemoryUnmap();
    bool IsMapped() const;
protected:
    URI uri;
    AccessMode accessMode;
    AccessPattern accessPattern;
    bool isOpen;
    bool isMapped;
    MediaType mediaType;
};
}
#endif