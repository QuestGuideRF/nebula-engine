#pragma once
#include "io/archfs/archivebase.h"
#include "minizip/unzip.h"
#include "io/zipfs/zipfileentry.h"
#include "io/zipfs/zipdirentry.h"
namespace IO
{
class ZipArchive : public ArchiveBase
{
    __DeclareClass(ZipArchive);
public:
    ZipArchive();
    virtual ~ZipArchive();
    bool Setup(const URI& uri, const Util::String& rootPath = "");
    void Discard();
    Util::Array<Util::String> ListFiles(const Util::String& dirPathInArchive, const Util::String& pattern) const;
    Util::Array<Util::String> ListDirectories(const Util::String& dirPathInArchive, const Util::String& pattern) const;
    URI ConvertToArchiveURI(const URI& fileURI) const;
    Util::String ConvertToPathInArchive(const Util::String& absPath) const;
private:
    friend class ZipFileSystem;
    friend class ZipFileStream;
    void ParseTableOfContents();
    void AddEntry(const Util::String& path);
    const ZipFileEntry* FindFileEntry(const Util::String& pathInZipArchive) const;
    ZipFileEntry* FindFileEntry(const Util::String& pathInZipArchive);
    const ZipDirEntry* FindDirEntry(const Util::String& pathInZipArchive) const;
    Util::String rootPath;
    unzFile zipFileHandle;
    ZipDirEntry rootEntry;
    Threading::CriticalSection archiveCritSect;
    zlib_filefunc64_def zlibIoFuncs;
};
}