#pragma once
#include "distributedtools/shareddircontrol.h"
#include "util/guid.h"
#include "io/uri.h"
namespace DistributedTools
{
class SharedDirFileSystem : public DistributedTools::SharedDirControl
{
__DeclareClass(DistributedTools::SharedDirFileSystem)
public:
    SharedDirFileSystem();
    ~SharedDirFileSystem();
    void CopyFileToSharedDir(const Util::String & src, const Util::String & dst);
    void CopyFilesToSharedDir(const Util::String & src, const Util::String & dst);
    void CopyDirectoryContentToSharedDir(const Util::String & src);
    void CopyFileFromSharedDir(const Util::String & src, const Util::String & dst);
    void CopyFilesFromSharedDir(const Util::String & src, const Util::String & dst);
    void CopyDirectoryContentFromSharedDir(const Util::String & dst);
    void RemoveFileInSharedDir(const Util::String & filepath);
    void RemoveDirectoryInSharedDir(const Util::String & dirpath);
    void RemoveDirectoryContent(const Util::String & dir);
    bool ContainsGuidSubDir(const Util::Guid & guid);
    bool DirIsEmpty(const Util::String & dir);
    Util::Array<Util::String> ListDirectories(const Util::String & dir);
    Util::Array<Util::String> ListFiles(const Util::String & dir);
private:
    void CreateControlDir();
    void RemoveControlDir();
    void CopyDirectoryContent(const Util::String & src, const Util::String & dst);
};
}