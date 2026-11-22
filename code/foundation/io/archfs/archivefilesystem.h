#pragma once
#if __WIN32__ || __linux__
#include "io/zipfs/zipfilesystem.h"
namespace IO
{
class ArchiveFileSystem : public ZipFileSystem
{
    __DeclareClass(ArchiveFileSystem);
    __DeclareInterfaceSingleton(ArchiveFileSystem);
public:
    ArchiveFileSystem();
    virtual ~ArchiveFileSystem();
};
}
#else
#error "IO::ArchiveFileSystem не реализован на этой платформе!"
#endif