#pragma once
#include "io/stream.h"
#include "minizip/unzip.h"
#include "util/stringatom.h"
namespace IO
{
class ZipFileEntry
{
public:
    ZipFileEntry();
    ~ZipFileEntry();
    const Util::StringAtom& GetName() const;
    IO::Stream::Size GetFileSize() const;
    bool Open(const Util::String& password = "");
    void Close();
    bool Read(void* buf, IO::Stream::Size bufSize) const;
private:
    friend class ZipArchive;
    void Setup(const Util::StringAtom& name, unzFile zipFileHandle, Threading::CriticalSection* critSect);
    Threading::CriticalSection* archiveCritSect;
    Util::StringAtom name;
    unzFile zipFileHandle;
    unz64_file_pos filePosInfo;
    uint64_t uncompressedSize;
};
inline const Util::StringAtom&
ZipFileEntry::GetName() const
{
    return this->name;
}
inline IO::Stream::Size
ZipFileEntry::GetFileSize() const
{
    return this->uncompressedSize;
}
}