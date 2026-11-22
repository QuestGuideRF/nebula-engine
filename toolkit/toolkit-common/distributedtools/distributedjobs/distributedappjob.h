#pragma once
#include "distributedtools/distributedjobs/distributedjob.h"
namespace DistributedTools
{
    class DistributedAppJob : public DistributedTools::DistributedJob
{
__DeclareClass(DistributedTools::DistributedAppJob);
public:
    DistributedAppJob();
    virtual ~DistributedAppJob();
    void UpdateProjectDirectory();
    bool IsAbleToRun();
    void RunJob();
    const Ptr<IO::Stream> & GetOutputStream();
    void SetFileList(const Util::Array<Util::String> & args);
    const Util::Array<Util::String> & GetFileList();
    void SetDirList(const Util::Array<Util::String> & args);
    const Util::Array<Util::String> & GetDirList();
    void SetApplicationPath(const Util::String & path);
    const Util::String & GetApplicationPath();
    void SetArguments(const Util::String & args);
    const Util::String & GetArguments();
    virtual Ptr<DistributedJob> Clone();
    virtual void CopyFrom(const Ptr<DistributedAppJob> & job);
private:
    void WriteXmlContent(const Ptr<IO::XmlWriter> & writer);
    void ReadXmlContent(const Ptr<IO::XmlReader> & reader);
    void UpdateJob();
    IO::URI localListPath;
    Util::Array<Util::String> fileList;
    Util::Array<Util::String> dirList;
    Util::String applicationPath;
    bool appReportedError;
    ToolkitUtil::AppLauncher jobApplication;
    Util::String arguments;
    Ptr<IO::MemoryStream> appOutStream;
};
inline
void
DistributedAppJob::SetFileList(const Util::Array<Util::String> & list)
{
    n_assert(Ready == this->jobState);
    this->fileList.Clear();
    this->fileList.AppendArray(list);
}
inline
const Util::Array<Util::String> &
DistributedAppJob::GetFileList()
{
    return this->fileList;
}
inline
void
DistributedAppJob::SetDirList(const Util::Array<Util::String> & list)
{
    n_assert(Ready == this->jobState);
    this->dirList.Clear();
    this->dirList.AppendArray(list);
}
inline
const Util::Array<Util::String> &
DistributedAppJob::GetDirList()
{
    return this->dirList;
}
inline
void
DistributedAppJob::SetApplicationPath(const Util::String & path)
{
    n_assert(Ready == this->jobState);
    this->applicationPath = path;
}
inline
const Util::String &
DistributedAppJob::GetApplicationPath()
{
    return this->applicationPath;
}
inline
void
DistributedAppJob::SetArguments(const Util::String & args)
{
    n_assert(Ready == this->jobState);
    this->arguments = args;
}
inline
const Util::String &
DistributedAppJob::GetArguments()
{
    return this->arguments;
}
}