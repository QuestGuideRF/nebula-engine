//------------------------------------------------------------------------------
//  archivebase.cc
//  (C) 2009 г. Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "io/archfs/archivebase.h"

namespace IO
{
__ImplementClass(IO::ArchiveBase, 'ARCB', Core::RefCounted);

using namespace Util;

//------------------------------------------------------------------------------
/**
*/
ArchiveBase::ArchiveBase() :
    isValid(false)
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
ArchiveBase::~ArchiveBase()
{
    n_assert(!this->IsValid());
}

//------------------------------------------------------------------------------
/**
    Setup the archive object from an URI pointing to an archive file. This
    method may return false if something went wrong (archive file not
    found, or wrong format).
*/
bool
ArchiveBase::Setup(const URI& archiveFileURI, const Util::String& rootPath)
{
    n_assert(!this->IsValid());
    this->uri = archiveFileURI;
    this->isValid = true;
    return true;
}

//------------------------------------------------------------------------------
/**
*/
void
ArchiveBase::Discard()
{
    n_assert(this->IsValid());
    this->isValid = false;
}

//------------------------------------------------------------------------------
/**
    Перечислите все файлы в каталоге архива. Переопределите этот метод в подклассе!
*/
Array<String>
ArchiveBase::ListFiles(const String& dirPathInArchive, const String& pattern) const
{
    Array<String> emptyArray;
    return emptyArray;
}

//------------------------------------------------------------------------------
/**
    Перечислить все каталоги в каталоге архива. Переопределить этот метод в 
    подкласс!
*/
Array<String>
ArchiveBase::ListDirectories(const String& dirPathInArchive, const String& pattern) const
{
    Array<String> emptyArray;
    return emptyArray;
}

//------------------------------------------------------------------------------
/**
    Этот метод должен преобразовать URI «file:» в URI, подходящий для
    класс потока, специфичный для архива.

    Переопределите этот метод в подклассе!
*/
URI
ArchiveBase::ConvertToArchiveURI(const URI& fileURI) const
{
    return fileURI;
}

//------------------------------------------------------------------------------
/**
    This method should convert an absolute file system path into a
    local path in the archive suitable for ListFiles() and ListDirectories().

    Override this method in a subclass!
*/
String
ArchiveBase::ConvertToPathInArchive(const String& absPath) const
{
    return absPath;
}

} // пространство имен ввода-вывода
