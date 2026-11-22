#pragma once
#include "core/refcounted.h"
#include "core/singleton.h"
#include "util/array.h"
#include "util/string.h"
#include "io/filetime.h"
#include "io/schemeregistry.h"
#include "archfs/archivefilesystem.h"
#include "io/cache/streamcache.h"
namespace Http
{
    class HttpClientRegistry;
};
namespace IO
{
class ArchiveFileSystem;
class FileWatcher;
class Stream;
class URI;
class AssignRegistry;
class IoServer : public Core::RefCounted
{
    __DeclareClass(IoServer);
    __DeclareSingleton(IoServer);
public:
    IoServer();
    virtual ~IoServer();
    bool MountArchive(const URI& uri);
    bool MountEmbeddedArchive(const URI& uri);
    void UnmountArchive(const URI& uri);
    bool IsArchiveMounted(const URI& uri) const;
    void SetArchiveFileSystemEnabled(bool b);
    bool IsArchiveFileSystemEnabled() const;
    void MountStandardArchives();
    void UnmountStandardArchives();
    Ptr<Stream> CreateStream(const URI& uri) const;
    bool CreateDirectory(const URI& uri) const;
    bool EnsureDirectoriesForFile(const URI& uri) const;
    bool DeleteDirectory(const URI& path) const;
    bool DirectoryExists(const URI& path) const;
    bool CopyFile(const URI& from, const URI& to) const;
    bool DeleteFile(const URI& path) const;
    bool FileExists(const URI& path) const;
    bool IsLocked(const URI& path) const;
    void SetReadOnly(const URI& path, bool b) const;
    bool IsReadOnly(const URI& path) const;
    unsigned int ComputeFileCrc(const URI& path) const;
    void SetFileWriteTime(const URI& path, FileTime fileTime);
    FileTime GetFileWriteTime(const URI& path) const;
    static bool ReadFile(const URI& path, Util::String & contents);
    static Util::String NativePath(const Util::String& path);
    Util::Array<Util::String> ListFiles(const URI& dir, const Util::String& pattern, bool asFullPath=false) const;
    Util::Array<Util::String> ListDirectories(const URI& dir, const Util::String& pattern, bool asFullPath=false, bool prioritizeArchive=true) const;
    using FileMapFunc = void(const Util::String& file, const Util::String& folder);
    void IterateFolders(const URI& dir, const Util::String& pattern, FileMapFunc iter);
    URI CreateTemporaryFilename(const URI& path) const;
private:
    Util::Array<Util::String> AddPathPrefixToArray(const Util::String& prefix, const Util::Array<Util::String>& filenames) const;
    bool archiveFileSystemEnabled;
    Ptr<ArchiveFileSystem> archiveFileSystem;
    static Threading::CriticalSection archiveCriticalSection;
    static bool StandardArchivesMounted;
    Ptr<Http::HttpClientRegistry> httpClientRegistry;
    Ptr<AssignRegistry> assignRegistry;
    Ptr<SchemeRegistry> schemeRegistry;
    Ptr<FileWatcher> watcher;
    Ptr<StreamCache> streamCache;
    static Threading::CriticalSection assignCriticalSection;
    static Threading::CriticalSection schemeCriticalSection;
    static Threading::CriticalSection watcherCriticalSection;
};
inline void
IoServer::SetArchiveFileSystemEnabled(bool b)
{
    this->archiveFileSystemEnabled = b;
}
inline bool
IoServer::IsArchiveFileSystemEnabled() const
{
    return this->archiveFileSystemEnabled && ArchiveFileSystem::Instance()->HasArchives();
}
}