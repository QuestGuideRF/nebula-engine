#pragma once
#include "distributedtools/shareddircontrol.h"
#include "util/guid.h"
#include "io/uri.h"
#include "io/textwriter.h"
#include "io/textreader.h"
namespace DistributedTools
{
class SharedDirFTP : public DistributedTools::SharedDirControl
{
__DeclareClass(DistributedTools::SharedDirFTP)
public:
    SharedDirFTP();
    ~SharedDirFTP();
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
    void SetUserName(const Util::String & name);
    void SetPassword(const Util::String & pass);
    void SetExeLocation(const IO::URI & location);
private:
    enum FTPFileType
    {
        File,
        Directory
    };
    struct FTPFile
    {
        FTPFileType type;
        Util::String name;
        Util::Array<FTPFile> content;
    };
    void CreateControlDir();
    void RemoveControlDir();
    FTPFile GetFtpDirectoryFileHierachy(const Util::String & path, bool searchSubdirectories = true);
    void CheckExecutableStatus();
    Ptr<IO::TextWriter> CreateScriptWriter();
    const Ptr<IO::Stream> & CloseWriterAndGetStream(const Ptr<IO::TextWriter> & writer);
    Ptr<IO::Stream> RunScript(const Ptr<IO::Stream> & script);
    void ScriptCreateSubDirectory(const Ptr<IO::TextWriter> & writer, const Util::String & path);
    void ScriptPutFile(const Ptr<IO::TextWriter> & writer, const Util::String & src);
    void ScriptPutFiles(const Ptr<IO::TextWriter> & writer, const Util::Array<Util::String> & srcArray);
    void ScriptPutDirectoryContent(const Ptr<IO::TextWriter> & writer, const Util::String & src,const Util::String & dst);
    void ScriptGetFile(const Ptr<IO::TextWriter> & writer, const Util::String & src,const Util::String & dst);
    void ScriptGetFiles(const Ptr<IO::TextWriter> & writer, const Util::String & src,const Util::String & dst);
    void ScriptGetFileHierachy(const Ptr<IO::TextWriter> & writer, const FTPFile & src,const Util::String & dst);
    void ScriptRemoveFileHierachy(const Ptr<IO::TextWriter> & writer, const FTPFile & src);
    Util::String username;
    Util::String password;
    IO::URI exeLocation;
    bool needVerboseCommand;
};
inline
void
SharedDirFTP::SetUserName(const Util::String & name)
{
    this->username = name;
}
inline
void
SharedDirFTP::SetPassword(const Util::String & pass)
{
    this->password = pass;
}
inline
void
SharedDirFTP::SetExeLocation(const IO::URI & location)
{
    this->exeLocation = location;
    this->CheckExecutableStatus();
}
}