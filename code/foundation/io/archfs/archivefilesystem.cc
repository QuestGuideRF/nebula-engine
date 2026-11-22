//------------------------------------------------------------------------------
//  archivefilesystem.cc
//  (C) 2009 г. Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "io/archfs/archivefilesystem.h"

namespace IO
{
#if __WIN32__ || __linux__
__ImplementClass(IO::ArchiveFileSystem, 'ARFS', IO::ZipFileSystem);
__ImplementInterfaceSingleton(IO::ArchiveFileSystem);
#else
#error "IO::ArchiveFileSystem не реализован на этой платформе!"
#endif

//------------------------------------------------------------------------------
/**
*/
ArchiveFileSystem::ArchiveFileSystem()
{
    __ConstructInterfaceSingleton;
}

//------------------------------------------------------------------------------
/**
*/
ArchiveFileSystem::~ArchiveFileSystem()
{
    __DestructInterfaceSingleton;
}

} // пространство имен ввода-вывода
