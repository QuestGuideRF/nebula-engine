#pragma once
#include "core/refcounted.h"
#include "io/uri.h"
#include "io/memorystream.h"
#include "io/xmlwriter.h"
#include "io/xmlreader.h"
#include "toolkit-common/applauncher.h"
#include "util/guid.h"
namespace DistributedTools
{
class DistributedJob : public Core::RefCounted
{
__DeclareClass(DistributedTools::DistributedJob);
public:
    enum DistributedJobState
    {
        Ready,
        ValidProjectPath,
        Updating,
        UpToDate,
        Working,
        Finished,
        JobError
    };
    DistributedJob();
    virtual ~DistributedJob();
    void SetProjectDirectory(const Util::String & dir);
    const Util::String & GetProjectDirectory();
    void SetSVNPath(const Util::String & path);
    const Util::String & GetSVNPath();
    void SetRequiredRevision(const Util::String & revision);
    const Util::String & GetRequiredRevision();
    DistributedJobState GetCurrentState();
    void SetIdentifier(const Util::String & id);
    const Util::String & GetIdentifier();
    const Util::Guid & GetGuid();
    void SetDistributedFlag(bool val);
    void SetRunningOnSlave(bool val);
    bool IsRunningOnSlave();
    void SetSharedDirPath(const Util::String & path);
    void SetSharedDirGuid(const Util::Guid & guid);
    void ValidateProjectPath();
    void SkipProjectDirectoryUpdate();
    Util::String GetCurrentRevision();
    void UpdateProjectDirectory();
    virtual void RunJob();
    void Update();
    void GenerateJobCommand(const Ptr<IO::XmlWriter> & writer);
    void SetupFromXml(const Ptr<IO::XmlReader> & reader);
    virtual bool IsAbleToRun();
    virtual bool HasOutputContent();
    virtual Util::String DequeueOutputContent();
    virtual Ptr<DistributedJob> Clone();
    virtual void CopyFrom(const Ptr<DistributedJob> & job);
protected:
    virtual void WriteXmlContent(const Ptr<IO::XmlWriter> & writer);
    virtual void ReadXmlContent(const Ptr<IO::XmlReader> & reader);
    virtual void UpdateJob();
    Util::String GetStateString();
    void SetStateFromString(const Util::String & str);
    virtual void WriteOutput(Util::String text);
    DistributedJobState jobState;
private:
    void UpdateSVN();
    Util::String projectDirectory;
    Util::String svnPath;
    Util::String requiredRevision;
    bool svnReportedError;
    Ptr<IO::MemoryStream> svnErrStream;
    Ptr<IO::TextWriter> jobOutputWriter;
    ToolkitUtil::AppLauncher svnApplication;
    Util::Guid jobGuid;
    Util::String identifier;
    Util::String sharedDirPath;
    Util::Guid sharedDirGuid;
    bool distributedFlag;
    bool isRunningOnSlave;
};
inline
void
DistributedJob::SetProjectDirectory(const Util::String & dir)
{
    n_assert(Ready == this->jobState);
    this->projectDirectory = dir;
}
inline
const Util::String &
DistributedJob::GetProjectDirectory()
{
    return this->projectDirectory;
}
inline
void
DistributedJob::SetSVNPath(const Util::String & path)
{
    n_assert(Ready == this->jobState);
    this->svnPath = path;
}
inline
const Util::String &
DistributedJob::GetSVNPath()
{
    return this->svnPath;
}
inline
void
DistributedJob::SetRequiredRevision(const Util::String & revision)
{
    n_assert(Ready == this->jobState);
    this->requiredRevision = revision;
}
inline
const Util::String &
DistributedJob::GetRequiredRevision()
{
    return this->requiredRevision;
}
inline
DistributedJob::DistributedJobState
DistributedJob::GetCurrentState()
{
    return this->jobState;
}
inline
void
DistributedJob::SetIdentifier(const Util::String & id)
{
    this->identifier = id;
}
inline
const Util::String &
DistributedJob::GetIdentifier()
{
    return this->identifier;
}
inline
const Util::Guid &
DistributedJob::GetGuid()
{
    return this->jobGuid;
}
inline
void
DistributedJob::SetDistributedFlag(bool val)
{
    this->distributedFlag = val;
}
inline
void
DistributedJob::SetRunningOnSlave(bool val)
{
    this->isRunningOnSlave = val;
}
inline
bool
DistributedJob::IsRunningOnSlave()
{
    return this->isRunningOnSlave;
}
inline
void
DistributedJob::SetSharedDirPath(const Util::String & path)
{
    this->sharedDirPath = path;
}
inline
void
DistributedJob::SetSharedDirGuid(const Util::Guid & guid)
{
    this->sharedDirGuid = guid;
}
}