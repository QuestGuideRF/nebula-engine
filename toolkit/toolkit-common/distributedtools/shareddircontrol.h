#pragma once
#include "core/refcounted.h"
#include "util/guid.h"
#include "io/uri.h"
namespace DistributedTools
{
class SharedDirControl : public Core::RefCounted
{
__DeclareClass(DistributedTools::SharedDirControl)
public:
    SharedDirControl();
    virtual ~SharedDirControl();
    virtual void CopyFileToSharedDir(const Util::String & src, const Util::String & dst);
    virtual void CopyFilesToSharedDir(const Util::String & src, const Util::String & dst);
    virtual void CopyDirectoryContentToSharedDir(const Util::String & src);
    virtual void CopyFileFromSharedDir(const Util::String & src, const Util::String & dst);
    virtual void CopyFilesFromSharedDir(const Util::String & src, const Util::String & dst);
    virtual void CopyDirectoryContentFromSharedDir(const Util::String & dst);
    virtual void RemoveFileInSharedDir(const Util::String & filepath);
    virtual void RemoveDirectoryInSharedDir(const Util::String & dirpath);
    virtual void RemoveDirectoryContent(const Util::String & dir);
    virtual bool DirIsEmpty(const Util::String & dir);
    virtual bool ContainsGuidSubDir(const Util::Guid & guid);
    virtual Util::Array<Util::String> ListDirectories(const Util::String & dir);
    virtual Util::Array<Util::String> ListFiles(const Util::String & dir);
    void SetUp();
    void CleanUp();
    bool ControlFileEquals(const Util::String & path);
    const Util::Guid & GetGuid();
    void SetGuid(const Util::Guid & guid);
    const IO::URI & GetPath();
    void SetPath(const IO::URI & path);
protected:
    virtual void CreateControlDir();
    virtual void RemoveControlDir();
    Util::Guid guid;
    IO::URI sharedDir;
    IO::URI path;
private:
};
inline
const Util::Guid &
SharedDirControl::GetGuid()
{
    return this->guid;
}
inline
void
SharedDirControl::SetGuid(const Util::Guid & guid)
{
    this->guid = guid;
}
inline
const IO::URI &
SharedDirControl::GetPath()
{
    return this->path;
}
inline
void
SharedDirControl::SetPath(const IO::URI & path)
{
    this->path = path;
}
}