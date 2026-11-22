//------------------------------------------------------------------------------
//  posixfswrapper.cc
//  (C) 2006 г. Радон Лабс ГмбХ
//  (C) 2013–2018 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "foundation/stdneb.h"
#include "io/posix/posixfswrapper.h"
#include "core/sysfunc.h"

#include <errno.h>
#include <dirent.h>
#include <fnmatch.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/mman.h>

#ifdef __APPLE__
namespace CoreFoundation {
    #include <CoreFoundation/CoreFoundation.h>
}
#endif

namespace Posix
{

using namespace Util;
using namespace Core;
using namespace IO;

//------------------------------------------------------------------------------
/**
    Open a file using the Posix function CreateFile(). Returns a handle
    to the file which must be passed to the other PosixFSWrapper file methods.
    If opening the file fails, the function will return 0. The filename
    must be a native Posix path (no assigns, etc...).
*/
PosixFSWrapper::Handle
PosixFSWrapper::OpenFile(const String& path, Stream::AccessMode accessMode, Stream::AccessPattern accessPattern)
{
    const char * mode;
    switch (accessMode)
    {
        case Stream::ReadAccess:
            mode = "r";
            break;

        case Stream::WriteAccess:
            mode = "w";
            break;

        case Stream::ReadWriteAccess:
            mode = "w+";
            break;
        case Stream::AppendAccess:
            mode = "a";
            break;
    }

    // открыть/создать файл
    PosixFSWrapper::Handle handle = fopen(path.AsCharPtr(), mode);
    return handle;
}

//------------------------------------------------------------------------------
/**
    Закрывает файл, открытый PosixFSWrapper::OpenFile().
*/
void
PosixFSWrapper::CloseFile(Handle handle)
{
    n_assert(0 != handle);
    fclose(handle);
}

//------------------------------------------------------------------------------
/**
    Записать данные в файл.
*/
void
PosixFSWrapper::Write(Handle handle, const void* buf, Stream::Size numBytes)
{
    n_assert(0 != handle);
    n_assert(buf != 0);
    n_assert(numBytes > 0);
    Stream::Size bytesWritten = fwrite(buf, 1, numBytes, handle);
    if (bytesWritten != numBytes)
    {
        n_error("PosixFSWrapper: Ошибка записи()\n");
    }
}

//------------------------------------------------------------------------------
/**
    Чтение данных из файла, возвращает количество прочитанных байтов.
*/
Stream::Size
PosixFSWrapper::Read(Handle handle, void* buf, Stream::Size numBytes)
{
    n_assert(0 != handle);
    n_assert(buf != 0);
    n_assert(numBytes > 0);
    Stream::Size bytesRead = fread(buf, 1, numBytes, handle);
    if (bytesRead == 0)
    {
        if(0 != ferror(handle))
        {
            n_error("PosixFSWrapper: Ошибка чтения()!\n");
        }
    }
    return bytesRead;
}

//------------------------------------------------------------------------------
/**
*/
char*
PosixFSWrapper::Map(Handle h, IO::Stream::AccessMode accessMode, Handle& mappedHandle)
{
    int fd = fileno(h);
    n_assert(fd >= 0);

    IO::Stream::Size fileSize = GetFileSize(h);
    n_assert(fileSize > 0);

    void* buffer = mmap(0, fileSize, PROT_READ, MAP_PRIVATE, fd, 0);
    
    // FIXME 
    mappedHandle = (Handle)fileSize;
    return (char*)buffer;
}

//------------------------------------------------------------------------------
/**
*/
void 
PosixFSWrapper::Unmap(Handle mapHandle, char* buf)
{
    IO::Stream::Size mapSize = (int64_t)mapHandle;
    int ret = munmap(buf, mapSize);
    n_assert(ret==0);
}
//------------------------------------------------------------------------------
/**
    Ищите в файле.
*/
void
PosixFSWrapper::Seek(Handle handle, Stream::Offset offset, Stream::SeekOrigin orig)
{
    n_assert(0 != handle);
    int moveMethod;
    switch (orig)
    {
        case Stream::Begin:
            moveMethod = SEEK_SET;
            break;
        case Stream::Current:
            moveMethod = SEEK_CUR;
            break;
        case Stream::End:
            moveMethod = SEEK_END;
            break;
        default:
            // не может случиться
            moveMethod = SEEK_SET;
            break;
    }
    fseeko(handle, offset, moveMethod);
}

//------------------------------------------------------------------------------
/**
    Получить текущую позицию в файле.
*/
Stream::Position
PosixFSWrapper::Tell(Handle handle)
{
    n_assert(0 != handle);
    return ftello(handle);
}

//------------------------------------------------------------------------------
/**
    Сбросить незаписанные данные в файл.
*/
void
PosixFSWrapper::Flush(Handle handle)
{
    n_assert(0 != handle);
    fflush(handle);
}

//------------------------------------------------------------------------------
/**
    Возвращает true, если текущая позиция находится в конце файла.
*/
bool
PosixFSWrapper::Eof(Handle handle)
{
    n_assert(0 != handle);
    if(feof(handle))
    {
        return true;
    }
    return Tell(handle) >= GetFileSize(handle);   
    
}

//------------------------------------------------------------------------------
/**
    Возвращает размер файла в байтах.
*/
Stream::Size
PosixFSWrapper::GetFileSize(Handle handle)
{
    n_assert(0 != handle);
    struct stat s;
    fstat(fileno(handle), &s);
    return s.st_size;
}

//------------------------------------------------------------------------------
/**
    Установить статус файла только для чтения.
*/
void
PosixFSWrapper::SetReadOnly(const String& path, bool readOnly)
{
    n_assert(path.IsValid());
    struct stat s;
    int r = stat(path.AsCharPtr(), &s);
    if (0 != r)
    {
        return;
    }
    mode_t fileAttrs = s.st_mode;
    if (readOnly)
    {
        fileAttrs &= ~S_IWUSR;
    }
    else
    {
        fileAttrs |= S_IWUSR;
    }
    chmod(path.AsCharPtr(), fileAttrs);
}

//------------------------------------------------------------------------------
/**
    Получить статус файла только для чтения.
*/
bool
PosixFSWrapper::IsReadOnly(const String& path)
{
    n_assert(path.IsValid());
    struct stat s;
    int r = stat(path.AsCharPtr(), &s);
    if (0 == r)
    {
        if (s.st_mode & S_IWUSR)
        {
            return true;
        }
        else if ((s.st_gid == getegid()) && (s.st_mode & S_IWGRP))
        {
            return true;
        }
    }
    return false;
}

//------------------------------------------------------------------------------
/**
    try to check for a lock by trying to lock the file. inherently racey, but
    good enough in some situations
*/
bool
PosixFSWrapper::IsLocked(const Util::String& path)
{
    n_assert(path.IsValid());
    int h = open(path.AsCharPtr(), O_RDWR);
    bool locked = 0 == flock(h, LOCK_EX);
    close(h);
    return locked;
}

//------------------------------------------------------------------------------
/**
    Deletes a file. Returns true if the operation was successful. The delete
    will fail if the fail doesn't exist or the file is read-only.
*/
bool
PosixFSWrapper::DeleteFile(const String& path)
{
    n_assert(path.IsValid());
    return (0 == unlink(path.AsCharPtr()));    
}

//------------------------------------------------------------------------------
/**
*/
bool
PosixFSWrapper::ReplaceFile(const Util::String& source, const Util::String& target)
{
    return (0 == rename(source.AsCharPtr(), target.AsCharPtr()));
}

//------------------------------------------------------------------------------
/**
    Удалить пустой каталог. Возвращает true, если операция прошла успешно.
*/
bool
PosixFSWrapper::DeleteDirectory(const String& path)
{
    n_assert(path.IsValid());
    return (0 == rmdir(path.AsCharPtr()));
}

//------------------------------------------------------------------------------
/**
    Верните true, если файл существует.
*/
bool
PosixFSWrapper::FileExists(const String& path)
{
    n_assert(path.IsValid());
    struct stat s;
    int r = stat(path.AsCharPtr(), &s);
    if ((0 == r) && (false == (S_ISDIR(s.st_mode))))
    {
        return true;
    }
    return false;
}

//------------------------------------------------------------------------------
/**
    Верните true, если каталог существует.
*/
bool
PosixFSWrapper::DirectoryExists(const String& path)
{
    n_assert(path.IsValid());
    struct stat s;
    int r = stat(path.AsCharPtr(), &s);
    if ((0 == r) && (true == S_ISDIR(s.st_mode)))
    {
        return true;
    }
    return false;
}
 
 //------------------------------------------------------------------------------
/**
    установить отметку времени доступа к записи файла
*/
void
PosixFSWrapper::SetFileWriteTime(const String& path, IO::FileTime fileTime)
{
  n_assert2(false,"FIXME, SetFileWriteTime не реализовано");
}
    
//------------------------------------------------------------------------------
/**
    Возвращает время последнего доступа для записи в файл.
*/
FileTime
PosixFSWrapper::GetFileWriteTime(const String& path)
{
    n_assert(path.IsValid());
    FileTime fileTime;
    struct stat s;
    int r = stat(path.AsCharPtr(), &s);
    if (0 == r)
    {
        fileTime.time.tv_sec = s.st_mtime;
        fileTime.time.tv_nsec = 0;
    }
    return fileTime;
}

//------------------------------------------------------------------------------
/**
    Создает новый каталог.
*/
bool
PosixFSWrapper::CreateDirectory(const String& path)
{
    n_assert(path.IsValid());
    return (0 == mkdir(path.AsCharPtr(),0700));
}

//------------------------------------------------------------------------------
/**
*/
Util::String
PosixFSWrapper::CreateTemporaryFilename(const Util::String& path)
{
    n_assert(path.IsValid());
    char buf[1024];
    snprintf(buf, 1024, "НЭБ%s.XXXXXX", path.AsCharPtr());
    String res(mkdtemp(buf));
    return res;
}

//------------------------------------------------------------------------------
/**
    Перечисляет все файлы в каталоге, отфильтрованные по шаблону.
*/
Array<String>
PosixFSWrapper::ListFiles(const String& dirPath, const String& pattern)
{
    n_assert(dirPath.IsValid());
    n_assert(pattern.IsValid());
    
    String pathWithPattern = dirPath + "/" + pattern;
    Array<String> fileList;
    DIR * dir = opendir(dirPath.AsCharPtr());
    if (0 != dir)
    {
        struct dirent *result = nullptr;
        do
        {
            if ((result = readdir(dir)) != nullptr)
            {
                if (0 != fnmatch(pattern.AsCharPtr(), result->d_name, FNM_CASEFOLD))
                {
                    continue;
                }
                String fullName = dirPath + "/" + result->d_name;
                struct stat s;
                stat(fullName.AsCharPtr(), &s);
                if (S_ISDIR(s.st_mode))
                {
                    continue;
                }
                fileList.Append(result->d_name);
            }
        } while (result != nullptr);
    }
    return fileList;
}

//------------------------------------------------------------------------------
/**
    Lists all subdirectories in a directory, filtered by a pattern. This will
    not return the special directories ".." and ".".
*/
Array<String>
PosixFSWrapper::ListDirectories(const String& dirPath, const String& pattern)
{
    n_assert(dirPath.IsValid());
    n_assert(pattern.IsValid());
    
    String pathWithPattern = dirPath + "/" + pattern;
    Array<String> dirList;
    DIR * dir = opendir(dirPath.AsCharPtr());
    if (0 != dir)
    {
        
        struct dirent *result = nullptr;
        do
        {
            if ((result = readdir(dir)) != nullptr)
            {
                if (0 != fnmatch(pattern.AsCharPtr(), result->d_name, FNM_CASEFOLD))
                {
                    continue;
                }

                // удалить точку
                if (0 == strcmp(result->d_name, ".") || 0 == strcmp(result->d_name, ".."))
                {
                    continue;
                }
                
                String fullName = dirPath + "/" + result->d_name;
                struct stat s;
                stat(fullName.AsCharPtr(), &s);
                if (!S_ISDIR(s.st_mode))
                {
                    continue;
                }
                dirList.Append(result->d_name);
            }
        } while (result != nullptr);
    }
    return dirList;
}

//------------------------------------------------------------------------------
/**
    Этот метод должен возвращать путь к домашнему каталогу текущего пользователя.
    Это каталог, в который приложение может записывать свои данные. Под
    Windows, это каталог «Мои файлы».
*/
String
PosixFSWrapper::GetUserDirectory()
{
    String result = getenv("HOME");
    result.ConvertBackslashes();
    return String("file:///") + result;
}

//------------------------------------------------------------------------------
/**
    Этот метод должен возвращать каталог для временных файлов с
    доступ на чтение/запись для текущего пользователя.
*/
String
PosixFSWrapper::GetTempDirectory()
{
    return String("file:///tmp/");
}

//------------------------------------------------------------------------------
/**
    Этот метод должен возвращать каталог, в котором находится исполняемый файл приложения.
    находится.
*/
String
PosixFSWrapper::GetBinDirectory()
{
    char buf[MAXPATHLEN];
#ifdef __APPLE__
    CoreFoundation::CFBundleRef mainBundle = CoreFoundation::CFBundleGetMainBundle();
    CoreFoundation::CFURLRef bundleURL = CoreFoundation::CFBundleCopyBundleURL(mainBundle);
    CoreFoundation::CFURLGetFileSystemRepresentation(bundleURL, true, (CoreFoundation::UInt8 *)buf, MAXPATHLEN);
#else    
    char szTmp[32];
    sprintf(szTmp, "/proc/%d/exe", getpid());
    int bytes = MIN(readlink(szTmp, buf, MAXPATHLEN), MAXPATHLEN / 1);
    if (bytes >= 0)
    {
    buf[bytes] = '\0';
    }
#endif /* __APPLE__ */
    
    String result = buf;
    result.ConvertBackslashes();
    result = result.ExtractDirName();
    result.TrimRight("/");
    return String("file:///") + result;
}

//------------------------------------------------------------------------------
/**
    This method should return the installation directory of the
    application. Under Nebula, this is either the directory where the
    executable is located, or 2 levels above the executable (if it is in 
    home:bin/posix).
*/
String
PosixFSWrapper::GetHomeDirectory()
{
    char buf[MAXPATHLEN];
#if NEBULA_DEBUG
#ifdef NEBULA_PROJECT_ROOT
    String EmbeddedHome = NEBULA_PROJECT_ROOT;
    return String("file:///") + EmbeddedHome;
#endif
#endif
#ifdef __APPLE__
    CoreFoundation::CFBundleRef mainBundle = CoreFoundation::CFBundleGetMainBundle();
    CoreFoundation::CFURLRef bundleURL = CoreFoundation::CFBundleCopyBundleURL(mainBundle);
    CoreFoundation::CFURLGetFileSystemRepresentation(bundleURL, true, (CoreFoundation::UInt8 *)buf, MAXPATHLEN);
#else    
    char szTmp[32];
    sprintf(szTmp, "/proc/%d/exe", getpid());
    int bytes = MIN(readlink(szTmp, buf, MAXPATHLEN), MAXPATHLEN / 1);
    if (bytes >= 0)
    {
    buf[bytes] = '\0';
    }
#endif
    String pathToExe(buf);
    pathToExe.ConvertBackslashes();

    // проверьте, находится ли исполняемый файл в каталоге posix
    String pathToDir = pathToExe.ExtractLastDirName();
    if (n_stricmp(pathToDir.AsCharPtr(), "posix") == 0 || n_stricmp(pathToDir.AsCharPtr(), "apple") == 0)
    {
        // обычная структура каталогов home:bin/posix
        // полоска бин/позикс
        String homePath = pathToExe.ExtractDirName();
        homePath = homePath.ExtractDirName();
        homePath = homePath.ExtractDirName();
        homePath.TrimRight("/");
        return String("file:///") + homePath;
    }
    else
    {
        // не в обычной структуре каталогов home:bin/posix, 
        // используйте каталог exe в качестве домашнего пути
        String homePath = pathToExe.ExtractDirName();
        return String("file:///") + homePath;
    }
}

//------------------------------------------------------------------------------
/**
*/
bool
PosixFSWrapper::IsDeviceName(const Util::String& str)
{
    if (str.Length() == 1)
    {
        uchar c = str[0];
        if (((c >= 'A') && (c <= 'Z')) ||
            ((c >= 'a') && (c <= 'z')))
        {
            return true;
        }
    }
    return false;
}

} // пространство имен ввода-вывода
