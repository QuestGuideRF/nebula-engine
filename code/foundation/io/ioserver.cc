//------------------------------------------------------------------------------
//  io/ioserver.cc
//  (C) 2006 г. Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "io/ioserver.h"
#include "core/ptr.h"
#include "io/fswrapper.h"
#include "util/stack.h"
#include "util/crc.h"
#include "system/systeminfo.h"
#include "io/assignregistry.h"
#include "io/archfs/archive.h"
#include "io/archfs/archivefilesystem.h"
#include "io/filewatcher.h"
#include "io/filestream.h"
#include <filesystem>
#include "http/httpclientregistry.h"


namespace IO
{
__ImplementClass(IO::IoServer, 'IOSV', Core::RefCounted);
__ImplementSingleton(IO::IoServer);

Threading::CriticalSection IoServer::assignCriticalSection;
Threading::CriticalSection IoServer::schemeCriticalSection;
Threading::CriticalSection IoServer::archiveCriticalSection;
Threading::CriticalSection IoServer::watcherCriticalSection;
bool IoServer::StandardArchivesMounted = false;

using namespace Core;
using namespace Util;

//------------------------------------------------------------------------------
/**
*/
IoServer::IoServer() :
    archiveFileSystemEnabled(true)
{
    __ConstructSingleton;

    // первый IoServer настраивает реестр глобальной схемы
    this->schemeCriticalSection.Enter();
    if (!SchemeRegistry::HasInstance())
    {
        this->schemeRegistry = SchemeRegistry::Create();
        this->schemeRegistry->Setup();
    }
    else
    {
        this->schemeRegistry = SchemeRegistry::Instance();
    }    
    this->schemeCriticalSection.Leave();

    // первый созданный IoServer устанавливает глобальный реестр назначений
    this->assignCriticalSection.Enter();
    if (!AssignRegistry::HasInstance())
    {
        this->assignRegistry = AssignRegistry::Create();
        this->assignRegistry->Setup();
    }
    else
    {
        // глобальный реестр назначений уже существует
        this->assignRegistry = AssignRegistry::Instance();
    }
    this->assignCriticalSection.Leave();

    this->archiveCriticalSection.Enter();
    if (!ArchiveFileSystem::HasInstance())
    {
        n_assert(!StandardArchivesMounted);
        this->archiveFileSystem = ArchiveFileSystem::Create();
        this->archiveFileSystem->Setup();
    }
    else
    {
        this->archiveFileSystem = ArchiveFileSystem::Instance();
    }
    this->archiveCriticalSection.Leave();

    this->watcherCriticalSection.Enter();
    #ifndef __linux__
    if (!FileWatcher::HasInstance())
    {
        this->watcher = FileWatcher::Create();
        this->watcher->Setup();
    }
    else
    {
        this->watcher = FileWatcher::Instance();
    }
    #endif
    this->watcherCriticalSection.Leave();

    this->httpClientRegistry = Http::HttpClientRegistry::Create();
    this->httpClientRegistry->Setup();
    this->streamCache = StreamCache::Create();
}

//------------------------------------------------------------------------------
/**
*/
IoServer::~IoServer()
{
    this->streamCache = nullptr;
    this->httpClientRegistry->Discard();
    this->httpClientRegistry = nullptr;

    this->watcher = nullptr;
    // размонтировать стандартные архивы, если это последний экземпляр
    if (StandardArchivesMounted && (this->archiveFileSystem->GetRefCount() == 1))
    {
        this->UnmountStandardArchives();
    }
    this->archiveFileSystem = nullptr;
    this->assignRegistry = nullptr;
    this->schemeRegistry = nullptr;

    __DestructSingleton;
}

//------------------------------------------------------------------------------
/**
*/
Ptr<Stream>
IoServer::CreateStream(const URI& uri) const
{
    n_assert(!uri.IsEmpty());
    n_assert(SchemeRegistry::Instance()->IsUriSchemeRegistered(uri.Scheme()));
    const Core::Rtti & streamClass = SchemeRegistry::Instance()->GetStreamClassByUriScheme(uri.Scheme());

#if !PUBLIC_BUILD
    if (streamClass == FileStream::RTTI)
    {
        const String path = uri.GetHostAndLocalPath();
        n_assert(path.IsValid());
        if (FSWrapper::FileExists(path))
        {
            Ptr<Stream> stream = (Stream*)streamClass.Create();
            stream->SetURI(uri);
            return stream;
        }
    }
#endif
    // проверьте, указывает ли URI на смонтированный архив
    if (this->IsArchiveFileSystemEnabled())
    {
        URI newUri = ArchiveFileSystem::Instance()->ConvertFileToArchiveURIIfExists(uri);
        Ptr<Stream> stream = (Stream*) schemeRegistry->GetStreamClassByUriScheme(newUri.Scheme()).Create();
        stream->SetURI(newUri);
        return stream;
    }
    else
    {
        Ptr<Stream> stream = (Stream*) schemeRegistry->GetStreamClassByUriScheme(uri.Scheme()).Create();
        stream->SetURI(uri);
        return stream;
    }
}

//------------------------------------------------------------------------------
/**
*/
bool
IoServer::MountArchive(const URI& uri)
{
    Ptr<Archive> archive = this->archiveFileSystem->Mount(uri);
    return archive.isvalid();
}

//------------------------------------------------------------------------------
/**
*/
bool
IoServer::MountEmbeddedArchive(const URI& uri)
{
    Ptr<Archive> archive = this->archiveFileSystem->MountEmbedded(uri, "root:");
    return archive.isvalid();
}


//------------------------------------------------------------------------------
/**
*/
void
IoServer::UnmountArchive(const URI& uri)
{
    this->archiveFileSystem->Unmount(uri);
}

//------------------------------------------------------------------------------
/**
*/
bool
IoServer::IsArchiveMounted(const URI& uri) const
{
    return this->archiveFileSystem->IsMounted(uri);
}

//------------------------------------------------------------------------------
/**
*/
void
IoServer::MountStandardArchives()
{
    this->archiveCriticalSection.Enter();
    n_assert(!StandardArchivesMounted);
    StandardArchivesMounted = true;

    // сначала загрузите архивы, специфичные для платформы, поскольку
    // файлы переопределяют любые общие файлы
    // if the archive files don't exist, no archives will be mounted
    System::SystemInfo systemInfo;  
    String platformString = System::PlatformTypeAsString(System::Platform);
    String platformArchivePath = "root:export";
    
    // установить приоритет папки экспорта перед zip 
    if (!this->DirectoryExists(platformArchivePath))
    {
        this->MountArchive(platformArchivePath);
    }

    this->archiveCriticalSection.Leave();
}

//------------------------------------------------------------------------------
/**
*/
void
IoServer::UnmountStandardArchives()
{
    this->archiveCriticalSection.Enter();
    n_assert(StandardArchivesMounted);
    StandardArchivesMounted = false;

    System::SystemInfo systemInfo;  
    String platformString = System::PlatformTypeAsString(System::Platform);
    String platformArchivePath = "root:export";
    if (this->IsArchiveMounted(platformArchivePath))
    {
        this->UnmountArchive(platformArchivePath);
    }

    this->archiveCriticalSection.Leave();
}

//------------------------------------------------------------------------------
/**
    Этот метод создает все недостающие каталоги в пути.
*/
bool
IoServer::CreateDirectory(const URI& uri) const
{
    n_assert(uri.Scheme() == "file");
    String path = uri.GetHostAndLocalPath();

    // собрать стек несуществующих компонентов каталога
    Stack<String> pathStack;
    while ((!path.IsEmpty()) && (!this->DirectoryExists(path)))
    {
        pathStack.Push(path);
        String nextPath = path.ExtractDirName();
        nextPath.TrimRight("/\\:");
        if ((nextPath == path) || FSWrapper::IsDeviceName(nextPath))
        {
            break;
        }
        path = nextPath;
    }

    // ошибка в исходном пути?
    if (path.IsEmpty())
    {
        return false;
    }

    // создать недостающие компоненты каталога
    while (!pathStack.IsEmpty())
    {
        String curPath = pathStack.Pop();
        if (!FSWrapper::CreateDirectory(curPath))
        {
            return false;
        }
    }
    return true;
}

//------------------------------------------------------------------------------
/**
*/
bool
IoServer::EnsureDirectoriesForFile(const URI& uri) const
{
    n_assert(uri.Scheme() == "file");
    String path = uri.GetHostAndLocalPath();

    String folder = path.ExtractToLastSlash();
    return this->CreateDirectory(folder);
}

//------------------------------------------------------------------------------
/**
    Это копирует файл в другой файл.
*/
bool
IoServer::CopyFile(const URI& fromUri, const URI& toUri) const
{
    // поддержка прозрачного архива...
    URI srcUri;
    if (this->IsArchiveFileSystemEnabled())
    {
        srcUri = ArchiveFileSystem::Instance()->ConvertFileToArchiveURIIfExists(fromUri);
    }
    else
    {
        srcUri = fromUri;
    }

    // if target file exists, remove read-only flag
    if (this->FileExists(toUri))
    {
        this->SetReadOnly(toUri, false);
    }

    // создавать файловые потоки
    Ptr<Stream> srcStream = this->CreateStream(srcUri);
    srcStream->SetAccessMode(Stream::ReadAccess);
    if (srcStream->Open())
    {
        Ptr<Stream> toStream = this->CreateStream(toUri);
        toStream->SetAccessMode(Stream::WriteAccess);
        if (toStream->Open())
        {
            // выделить буфер и скопировать содержимое в цикле
            const int bufSize = 1024 * 1024;
            void* buffer = Memory::Alloc(Memory::ScratchHeap, bufSize);
            while (!srcStream->Eof())
            {
                Stream::Size bytesRead = srcStream->Read(buffer, bufSize);
                toStream->Write(buffer, bytesRead);
            }
            Memory::Free(Memory::ScratchHeap, buffer);
            toStream->Close();
        }
        else
        {
            n_error("IO::IoServer::CopyFile(): не удалось открыть целевой файл '%s'", toUri.AsString().AsCharPtr());
            return false;
        }
        srcStream->Close();
    }
    else
    {
        n_error("IO::IoServer::CopyFile(): не удалось открыть исходный файл '%s'", fromUri.AsString().AsCharPtr());
        return false;
    }
    return true;
}

//------------------------------------------------------------------------------
/**
    Этот метод вычисляет контрольную сумму CRC для файла.
*/
unsigned int
IoServer::ComputeFileCrc(const URI& uri) const
{
    unsigned int result = 0;

    // создать поток
    Ptr<Stream> stream = this->CreateStream(uri);
    stream->SetAccessMode(Stream::ReadAccess);
    if (stream->Open())
    {
        Crc crc;
        crc.Begin();
        const int bufSize = (1<<16);
        unsigned char* buffer = (unsigned char*) Memory::Alloc(Memory::ScratchHeap, bufSize);
        while (!stream->Eof())
        {
            Stream::Size bytesRead = stream->Read(buffer, bufSize);
            crc.Compute(buffer, bytesRead);
        }
        Memory::Free(Memory::ScratchHeap, buffer);
        crc.End();
        result = crc.GetResult();
        stream->Close();
    }
    else
    {
        n_error("IO::IoServer::ComputeFileCrc(): не удалось открыть файл «%s»!", uri.AsString().AsCharPtr());
    }
    return result;
}

//------------------------------------------------------------------------------
/**
*/
bool
IoServer::DeleteFile(const URI& uri) const
{
    n_assert(uri.Scheme() == "file");
    const String path = uri.GetHostAndLocalPath();
    n_assert(path.IsValid());
    return FSWrapper::DeleteFile(path);
}

//------------------------------------------------------------------------------
/**
*/
bool
IoServer::DeleteDirectory(const URI& uri) const
{
    n_assert(uri.Scheme() == "file");
    const String path = uri.GetHostAndLocalPath();
    n_assert(path.IsValid());
    return FSWrapper::DeleteDirectory(path);
}

//------------------------------------------------------------------------------
/**
*/
bool
IoServer::FileExists(const URI& uri) const
{
    // поддержка прозрачного архива
    if (this->IsArchiveFileSystemEnabled())
    {
        Ptr<Archive> archive = ArchiveFileSystem::Instance()->FindArchiveWithFile(uri);
        if (archive.isvalid())
        {
            return true;
        }
    }

    // не существует в виде архива, проверьте обычную файловую систему
    const String path = uri.GetHostAndLocalPath();
    n_assert(path.IsValid());
    return FSWrapper::FileExists(path);
}

//------------------------------------------------------------------------------
/**
*/
bool
IoServer::DirectoryExists(const URI& uri) const
{
    // поддержка прозрачного архива
    if (this->IsArchiveFileSystemEnabled())
    {
        if (uri.Scheme() == "file")
        {
            Ptr<Archive> archive = ArchiveFileSystem::Instance()->FindArchiveWithDir(uri);
            if (archive.isvalid())
            {
                return true;
            }
        }
    }

    // не существует в виде архива, проверьте обычную файловую систему
    const String path = uri.GetHostAndLocalPath();
    n_assert(path.IsValid());
    return FSWrapper::DirectoryExists(path);
}

//------------------------------------------------------------------------------
/**
*/
void
IoServer::SetReadOnly(const URI& uri, bool readOnly) const
{
    n_assert(uri.Scheme() == "file");
    const String path = uri.GetHostAndLocalPath();
    n_assert(path.IsValid());
    FSWrapper::SetReadOnly(path, readOnly);
}

//------------------------------------------------------------------------------
/**
*/
bool
IoServer::IsReadOnly(const URI& uri) const
{
    n_assert(uri.Scheme() == "file");

    // поддержка прозрачной файловой системы архивов
    if (this->IsArchiveFileSystemEnabled())
    {
        URI arcUri = ArchiveFileSystem::Instance()->ConvertFileToArchiveURIIfExists(uri);
        if (arcUri.Scheme() != "file")
        {
            // файл существует в архиве, архивы обычно доступны только для чтения
            return true;
        }
    }

    // файла нет в архиве, обрабатывать обычным способом
    const String path = uri.GetHostAndLocalPath();
    n_assert(path.IsValid());
    return FSWrapper::IsReadOnly(path);
}

//------------------------------------------------------------------------------
/**
*/
void
IoServer::SetFileWriteTime(const URI& uri, FileTime fileTime)
{
    n_assert(uri.Scheme() == "file");
    const String path = uri.GetHostAndLocalPath();
    n_assert(path.IsValid());
    FSWrapper::SetFileWriteTime(path, fileTime);
}

//------------------------------------------------------------------------------
/**
*/
FileTime
IoServer::GetFileWriteTime(const URI& uri) const
{
    n_assert(uri.Scheme() == "file");
    const String path = uri.GetHostAndLocalPath();
    n_assert(path.IsValid());
    return FSWrapper::GetFileWriteTime(path);
}

//------------------------------------------------------------------------------
/**
*/
Array<String>
IoServer::ListFiles(const URI& uri, const String& pattern, bool asFullPath) const
{
    n_assert(pattern.IsValid());
    Array<String> result;

    //FIXME, это должно обрабатываться более общим образом
    if (uri.Scheme() != "file")
    {
        Util::String fileList;
        URI listFile = uri;
        listFile.AppendLocalPath("/_files.lst");
        if (IoServer::ReadFile(listFile, fileList))
        {
            Util::Array<Util::String> files = fileList.Tokenize("\n");
            for (auto const& fileName : files)
            {
                if (String::MatchPattern(fileName, pattern))
                {
                    result.Append(fileName);
                }
            }
            return result;
        }
    }

    // поддержка прозрачной файловой системы архивов
    if (this->IsArchiveFileSystemEnabled())
    {
        Ptr<Archive> archive = ArchiveFileSystem::Instance()->FindArchiveWithDir(uri);
        if (archive.isvalid())
        {
            String pathInArchive = archive->ConvertToPathInArchive(uri.LocalPath());
            result = archive->ListFiles(pathInArchive, pattern);
        }
    }
// добавлять локальные файлы, если они не общедоступны
#if !PUBLIC_BUILD
    Array<String> localresult;
    localresult = FSWrapper::ListFiles(uri.GetHostAndLocalPath(), pattern);
    for (const String& str : localresult)
    {
        if (result.FindIndex(str) == InvalidIndex)
        {
            result.Append(str);
        }
    }
#endif
    if (asFullPath)
    {
        result = this->AddPathPrefixToArray(uri.LocalPath(), result);
    }
    return result;
}

//------------------------------------------------------------------------------
/**
*/
Array<String>
IoServer::ListDirectories(const URI& uri, const String& pattern, bool asFullPath, bool prioritizeArchive) const
{
    n_assert(pattern.IsValid());
    Array<String> result;

    // поддержка прозрачной файловой системы архивов
    if (this->IsArchiveFileSystemEnabled() && prioritizeArchive)
    {
        Ptr<Archive> archive = ArchiveFileSystem::Instance()->FindArchiveWithDir(uri);
        if (archive.isvalid())
        {
            String pathInArchive = archive->ConvertToPathInArchive(uri.LocalPath());
            result = archive->ListDirectories(pathInArchive, pattern);
            if (asFullPath)
            {
                result = this->AddPathPrefixToArray(uri.LocalPath(), result);
            }
            return result;
        }
    }
    
    // провал: не содержится в архиве, обрабатывается обычным способом
    result = FSWrapper::ListDirectories(uri.GetHostAndLocalPath(), pattern);
    if (asFullPath)
    {
        result = this->AddPathPrefixToArray(uri.LocalPath(), result);
    }
    return result;
}

//------------------------------------------------------------------------------
/**
*/
Array<String>
IoServer::AddPathPrefixToArray(const String& prefix, const Array<String>& filenames) const
{
    Array<String> result;
    if (!filenames.IsEmpty())
    {
        result.Reserve(filenames.Size());
        IndexT i;
        for (i = 0; i < filenames.Size(); i++)
        {
            String path = prefix;
            path.Append("/");
            path.Append(filenames[i]);
            result.Append(path);
        }
    }
    return result;
}

//------------------------------------------------------------------------------
/**
*/
bool
IoServer::ReadFile(const URI& path, Util::String& contents)
{
    // создать файловый поток
    Ptr<Stream> stream = IoServer::Instance()->CreateStream(path);

    // открыть файл
    if (stream->Open())
    {
        // прочитать все данные
        void* data = stream->Map();
        Stream::Size size = stream->GetSize();

        // сопоставить строку
        n_assert(size < INT_MAX);
        contents.Set((char*)data, (SizeT)size);

        // закрыть поток
        stream->Close();
        return true;
    }
    return false;
}

//------------------------------------------------------------------------------
/**
*/
bool
IoServer::IsLocked(const URI& uri) const
{
    n_assert(uri.Scheme() == "file");
    n_assert(this->FileExists(uri));
    const String path = uri.GetHostAndLocalPath();
    n_assert(path.IsValid());
    return FSWrapper::IsLocked(path);
}

//------------------------------------------------------------------------------
/**
*/
IO::URI
IoServer::CreateTemporaryFilename(const URI& uri) const
{
    n_assert(uri.Scheme() == "file");   
    const String path = uri.GetHostAndLocalPath();
    n_assert(path.IsValid());   
    return URI(FSWrapper::CreateTemporaryFilename(path));
}

//------------------------------------------------------------------------------
/**
*/
Util::String
IoServer::NativePath(const Util::String& path)
{
#if __cplusplus > 201703L
    std::filesystem::path u8path = std::filesystem::absolute(std::filesystem::path((char8_t*)path.AsCharPtr()));
#else
    std::filesystem::path u8path = std::filesystem::absolute(std::filesystem::u8path(path.AsCharPtr()));
#endif
    return u8path.string().c_str();
}

//------------------------------------------------------------------------------
/**
*/
void 
IoServer::IterateFolders(const URI& dir, const Util::String& pattern, FileMapFunc iter)
{
    Util::Array<Util::String> dirs;
    dirs.Append(dir.LocalPath());

    while (dirs.Size() > 0)
    {
        Util::String currentDir = dirs.PopFront();
        Util::Array<Util::String> files = this->ListFiles(currentDir, pattern, false);
        dirs.AppendArray(this->ListDirectories(currentDir, "*", true));
        for (auto file : files)
        {
            iter(file, currentDir);
        }
    }

}

} // пространство имен ввода-вывода
