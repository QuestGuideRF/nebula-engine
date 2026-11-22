#pragma once
#include "io/stream.h"
namespace IO
{
class ZipFileEntry;
class ZipFileStream : public Stream
{
    __DeclareClass(ZipFileStream);
public:
    ZipFileStream();
    virtual ~ZipFileStream();
    virtual bool CanRead() const;
    virtual bool CanWrite() const;
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
    virtual void Unmap();
    virtual void* MemoryMap();
    virtual void MemoryUnmap();
private:
    bool CopyToMap();
    Size size;
    Position position;
    ZipFileEntry *zipFileEntry;
    unsigned char *mapBuffer;
};
}