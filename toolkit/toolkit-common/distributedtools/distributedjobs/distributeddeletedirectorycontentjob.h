#pragma once
#include "distributedtools/distributedjobs/distributedjob.h"
namespace DistributedTools
{
class DistributedDeleteDirectoryContentJob : public DistributedJob
{
__DeclareClass(DistributedTools::DistributedDeleteDirectoryContentJob);
public:
    DistributedDeleteDirectoryContentJob();
    virtual ~DistributedDeleteDirectoryContentJob();
    void SetDelDirectoryPath(const IO::URI & path);
    const IO::URI & GetDelDirectoryPath();
    void RunJob();
    bool IsAbleToRun();
    Ptr<DistributedJob> Clone();
    virtual void CopyFrom(const Ptr<DistributedDeleteDirectoryContentJob> & job);
private:
    void WriteXmlContent(const Ptr<IO::XmlWriter> & writer);
    void ReadXmlContent(const Ptr<IO::XmlReader> & reader);
    void UpdateJob();
    IO::URI delDirPath;
};
inline
void
DistributedDeleteDirectoryContentJob::SetDelDirectoryPath(const IO::URI & path)
{
    n_assert(Ready == this->jobState);
    this->delDirPath = path;
}
inline
const IO::URI &
DistributedDeleteDirectoryContentJob::GetDelDirectoryPath()
{
    return this->delDirPath;
}
}