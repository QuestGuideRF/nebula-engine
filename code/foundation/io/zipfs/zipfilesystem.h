#pragma once
#include "io/archfs/archivefilesystembase.h"
namespace IO
{
class ZipArchive;
class ZipFileSystem : public ArchiveFileSystemBase
{
    __DeclareClass(ZipFileSystem);
    __DeclareInterfaceSingleton(ZipFileSystem);
public:
    ZipFileSystem();
    virtual ~ZipFileSystem();
    void Setup();
    void Discard();
    Ptr<Archive> FindArchiveWithFile(const URI& fileUri) const;
    Ptr<Archive> FindArchiveWithDir(const URI& dirUri) const;
};
}