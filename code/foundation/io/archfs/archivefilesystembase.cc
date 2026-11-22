//------------------------------------------------------------------------------
//  archivefilesystembase.cc
//  (C) 2009 г. Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "io/archfs/archivefilesystembase.h"
#include "io/archfs/archive.h"
#include "io/assignregistry.h"

namespace IO
{
__ImplementClass(IO::ArchiveFileSystemBase, 'AFSB', Core::RefCounted);
__ImplementInterfaceSingleton(IO::ArchiveFileSystemBase);

using namespace Util;

//------------------------------------------------------------------------------
/**
*/
ArchiveFileSystemBase::ArchiveFileSystemBase() :
    isValid(false)
{
    __ConstructInterfaceSingleton;
}

//------------------------------------------------------------------------------
/**
*/
ArchiveFileSystemBase::~ArchiveFileSystemBase()
{
    if (this->IsValid())
    {
        // убедитесь, что производный метод вызывается
        this->Discard();
    }    
    __DestructInterfaceSingleton;
}

//------------------------------------------------------------------------------
/**
    Настройте файловую систему архива. Подклассы могут регистрировать свои
    архивируйте классы потоков с помощью SchemeRegistry здесь.
*/
void
ArchiveFileSystemBase::Setup()
{
    n_assert(!this->IsValid());
    this->isValid = true;
}

//------------------------------------------------------------------------------
/**
    Удалить файловую систему архива.
*/
void
ArchiveFileSystemBase::Discard()
{
    n_assert(this->IsValid());

    // размонтировать все смонтированные файловые системы
    while (!this->archives.IsEmpty())
    {
        // убедитесь, что производный метод вызывается
        this->Unmount(this->archives.ValueAtIndex(0));
    }

    this->isValid = false;
}

//------------------------------------------------------------------------------
/**
    This "mounts" an archive file by creating a new Archive object
    and adding it to the archive dictionary. If mounting fails, an invalid
    pointer will be returned!
*/
Ptr<Archive>
ArchiveFileSystemBase::Mount(const URI& uri)
{
    return MountEmbedded(uri, "");
}
//------------------------------------------------------------------------------
/**
    This "mounts" an archive file by creating a new Archive object
    and adding it to the archive dictionary. If mounting fails, an invalid
    pointer will be returned!
*/
Ptr<Archive>
ArchiveFileSystemBase::MountEmbedded(const URI& uri, const Util::String& rootPath)
{
    n_assert(!this->IsMounted(uri));
    String path = AssignRegistry::Instance()->ResolveAssigns(uri).LocalPath();
    Ptr<Archive> newArchive = Archive::Create();
    if (newArchive->Setup(uri, rootPath))
    {
        this->critSect.Enter();
        this->archives.Add(path, newArchive);
        this->critSect.Leave();
    }
    else
    {
        newArchive = nullptr;
    }
    return newArchive;
}


//------------------------------------------------------------------------------
/**
    Unmount a zip archive, this will remove the archive from the internal
    archive registry, and call the Discard() method on it.
*/
void
ArchiveFileSystemBase::Unmount(const Ptr<Archive>& archive)
{
    n_assert(this->IsMounted(archive->GetURI()));
    archive->Discard();
    String path = AssignRegistry::Instance()->ResolveAssigns(archive->GetURI()).LocalPath();

    this->critSect.Enter();
    this->archives.Erase(path);
    this->critSect.Leave();
}

//------------------------------------------------------------------------------
/**
    Размонтируйте архив по URI архива. 
*/
void
ArchiveFileSystemBase::Unmount(const URI& uri)
{
    n_assert(this->IsMounted(uri));
    String path = AssignRegistry::Instance()->ResolveAssigns(uri).LocalPath();
    Ptr<Archive> archive = this->archives[path];
    archive->Discard();

    this->critSect.Enter();
    this->archives.Erase(path);
    this->critSect.Leave();
}

//------------------------------------------------------------------------------
/**
    Вернуть все смонтированные на данный момент архивы.
*/
Array<Ptr<Archive> >
ArchiveFileSystemBase::GetMountedArchives() const
{
    this->critSect.Enter();    
    Array<Ptr<Archive> > archiveArray = this->archives.ValuesAsArray();
    this->critSect.Leave();
    return archiveArray;
}

//------------------------------------------------------------------------------
/**
    Resolve an archive path into an Archive pointer. Returns 0
    if no archive with that name exists. The filename will be resolved into
    an absolute path internally before the lookup happens.
*/
Ptr<Archive>
ArchiveFileSystemBase::FindArchive(const URI& uri) const
{
    String path = AssignRegistry::Instance()->ResolveAssigns(uri).LocalPath();
    Ptr<Archive> result;

    this->critSect.Enter();    
    IndexT index = this->archives.FindIndex(path);
    if (InvalidIndex != index)
    {
        result = this->archives.ValueAtIndex(index);
    }
    this->critSect.Leave();
    return result;
}

//------------------------------------------------------------------------------
/**
*/
bool
ArchiveFileSystemBase::IsMounted(const URI& uri) const
{
    String path = AssignRegistry::Instance()->ResolveAssigns(uri).LocalPath();
    this->critSect.Enter();
    bool result = this->archives.Contains(path);
    this->critSect.Leave();
    return result;
}

//------------------------------------------------------------------------------
/**
*/
bool
ArchiveFileSystemBase::HasArchives() const
{
    return !this->archives.IsEmpty();
}

//------------------------------------------------------------------------------
/**
    Этот метод должен возвращать архив, содержащий предоставленный 
    URI файла. Переопределите этот метод в производном классе!
*/
Ptr<Archive>
ArchiveFileSystemBase::FindArchiveWithFile(const URI& uri) const
{
    return Ptr<Archive>();
}

//------------------------------------------------------------------------------
/**
    Этот метод должен возвращать архив, содержащий
    предоставил URI каталога. Переопределите этот метод в производном классе!
*/
Ptr<Archive>
ArchiveFileSystemBase::FindArchiveWithDir(const URI& uri) const
{
    return Ptr<Archive>();
}

//------------------------------------------------------------------------------
/**
    This method should check if the provided URI is located in any
    of the mounted archives, and if yes, return a converted the URI 
    which describes how the file in the archive is accessed.

    This method must be overriden in a subclass, the base class implementation
    will always return the original URI!
*/
URI
ArchiveFileSystemBase::ConvertFileToArchiveURIIfExists(const URI& uri) const
{
    // убедитесь, что производный метод вызывается
    Ptr<Archive> archive = this->FindArchiveWithFile(uri);
    if (archive.isvalid())
    {
        return archive->ConvertToArchiveURI(uri);
    }
    // провал: нет совпадения, вернуть исходный URI
    return uri;
}

//------------------------------------------------------------------------------
/**
    Этот метод является версией ConvertFileToArchiveURIIfExists() для каталога.
*/
URI
ArchiveFileSystemBase::ConvertDirToArchiveURIIfExists(const URI& uri) const
{
    Ptr<Archive> archive = this->FindArchiveWithDir(uri);
    if (archive.isvalid())
    {
        return archive->ConvertToArchiveURI(uri);
    }
    // провал: нет совпадения, вернуть исходный URI
    return uri;
}

} // пространство имен ввода-вывода
