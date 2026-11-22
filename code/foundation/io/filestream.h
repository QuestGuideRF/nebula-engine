#pragma once
#include "io/stream.h"
#include "io/fswrapper.h"
namespace IO
{
class FileStream : public Stream
{
    __DeclareClass(FileStream);
public:
    FileStream();
    virtual ~FileStream();
    virtual bool CanRead() const override;
    virtual bool CanWrite() const override;
    virtual bool CanSeek() const override;
    virtual bool CanBeMapped() const override;
    virtual Size GetSize() const override;
    virtual Position GetPosition() const override;
    virtual bool Open() override;
    virtual void Close() override;
    virtual void Write(const void* ptr, Size numBytes) override;
    virtual Size Read(void* ptr, Size numBytes) override;
    virtual void Seek(Offset offset, SeekOrigin origin) override;
    virtual void Flush() override;
    virtual bool Eof() const override;
    virtual void* Map() override;
    virtual void Unmap() override;
    virtual void* MemoryMap() override;
    virtual void MemoryUnmap() override;
protected:
    FSWrapper::Handle handle;
    FSWrapper::Handle mapHandle;
    void* mappedContent;
};
}