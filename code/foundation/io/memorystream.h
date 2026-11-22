#pragma once
#include "io/stream.h"
namespace IO
{
class MemoryStream : public Stream
{
    __DeclareClass(MemoryStream);
public:
    MemoryStream();
    virtual ~MemoryStream();
    virtual bool CanRead() const;
    virtual bool CanWrite() const;
    virtual bool CanSeek() const;
    virtual bool CanBeMapped() const;
    virtual void SetSize(Size s);
    virtual Size GetSize() const;
    virtual Position GetPosition() const;
    virtual bool Open();
    virtual void Close();
    virtual void Write(const void* ptr, Size numBytes);
    virtual Size Read(void* ptr, Size numBytes);
    virtual void Seek(Offset offset, SeekOrigin origin);
    virtual bool Eof() const;
    virtual void* Map();
    virtual void* MemoryMap();
    virtual void Unmap();
    void* GetRawPointer() const;
protected:
    void Realloc(Size s);
    bool HasRoom(Size numBytes) const;
    void MakeRoom(Size numBytes);
    static const Size InitialSize = 256;
    Size capacity;
    Size size;
    Position position;
    unsigned char* buffer;
};
}