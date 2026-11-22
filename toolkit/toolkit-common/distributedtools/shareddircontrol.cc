//------------------------------------------------------------------------------
//  shareddircontrol.cc
//  (C) 2009 г. РадонЛабс ГмбХ
//  (C) 2013–2016 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "foundation/stdneb.h"
#include "shareddircontrol.h"

using namespace IO;
using namespace Util;

namespace DistributedTools
{
    __ImplementClass(DistributedTools::SharedDirControl,'DSDC',Core::RefCounted)
//------------------------------------------------------------------------------
/**
    Конструктор 
*/
SharedDirControl::SharedDirControl()
{
}

//------------------------------------------------------------------------------
/**
    Деструктор  
*/
SharedDirControl::~SharedDirControl()
{
}

//------------------------------------------------------------------------------
/**
    set up shared directory (creates subdir from guid)
    and init private 'sharedDir' member
*/
void
SharedDirControl::SetUp()
{
    this->sharedDir = this->path;
    this->sharedDir.AppendLocalPath(this->guid.AsString());
    this->CreateControlDir();
}

//------------------------------------------------------------------------------
/**
   очистить общий каталог (удалить все внутри)
*/
void
SharedDirControl::CleanUp()
{
    this->RemoveDirectoryContent(this->sharedDir.AsString());
    this->RemoveControlDir();
}

//------------------------------------------------------------------------------
/**
    удалить все файлы и каталоги указанного
    каталог (относительно управляющего каталога) рекурсивно 
*/
void
SharedDirControl::RemoveDirectoryContent(const Util::String & dir)
{
    // пусто, переопределить в подклассе
}

//------------------------------------------------------------------------------
/**
*/
void
SharedDirControl::CopyFileToSharedDir(const Util::String & src, const Util::String & dst)
{
    // пусто, переопределить в подклассе
}

//------------------------------------------------------------------------------
/**
*/
void
SharedDirControl::CopyFilesToSharedDir(const Util::String & src, const Util::String & dst)
{
    // пусто, переопределить в подклассе
}

//------------------------------------------------------------------------------
/**
*/
void
SharedDirControl::CopyDirectoryContentToSharedDir(const Util::String & src)
{
    // пусто, переопределить в подклассе
}

//------------------------------------------------------------------------------
/**
*/
void
SharedDirControl::CopyFileFromSharedDir(const Util::String & src, const Util::String & dst)
{
    // пусто, переопределить в подклассе
}

//------------------------------------------------------------------------------
/**
*/
void
SharedDirControl::CopyFilesFromSharedDir(const Util::String & src, const Util::String & dst)
{
    // пусто, переопределить в подклассе
}

//------------------------------------------------------------------------------
/**
*/
void
SharedDirControl::CopyDirectoryContentFromSharedDir(const Util::String & dst)
{
    // пусто, переопределить в подклассе
}

//------------------------------------------------------------------------------
/**
*/
void
SharedDirControl::RemoveFileInSharedDir(const Util::String & filepath)
{
    // пусто, переопределить в подклассе
}
    
//------------------------------------------------------------------------------
/**
*/
void
SharedDirControl::RemoveDirectoryInSharedDir(const Util::String & dirpath)
{
    // пусто, переопределить в подклассе
}

//------------------------------------------------------------------------------
/**
*/
bool
SharedDirControl::ContainsGuidSubDir(const Util::Guid & guid)
{
    // переопределить в подклассе
    return false;
}


//------------------------------------------------------------------------------
/**
*/
bool
SharedDirControl::DirIsEmpty(const Util::String & dir)
{
    // переопределить в подклассе
    return false;
}
    
//------------------------------------------------------------------------------
/**
*/
Util::Array<Util::String>
SharedDirControl::ListDirectories(const Util::String & dir)
{
    // переопределить в подклассе
    Array<String> res;
    return res;
}

//------------------------------------------------------------------------------
/**
*/
Util::Array<Util::String>
SharedDirControl::ListFiles(const Util::String & dir)
{
    // переопределить в подклассе
    Array<String> res;
    return res;
}
 
//------------------------------------------------------------------------------
/**
*/
void
SharedDirControl::CreateControlDir()
{
    // пусто, переопределить в подклассе
}
    
//------------------------------------------------------------------------------
/**
*/
void
SharedDirControl::RemoveControlDir()
{
    // пусто, переопределить в подклассе
}

} // пространство имен DistributedTools
