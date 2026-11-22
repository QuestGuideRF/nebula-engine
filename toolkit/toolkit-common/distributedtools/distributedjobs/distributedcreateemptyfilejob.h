#pragma once
#include "distributedtools/distributedjobs/distributedjob.h"
namespace DistributedTools
{
class DistributedCreateEmptyFileJob : public DistributedJob
{
__DeclareClass(DistributedTools::DistributedCreateEmptyFileJob);
public:
    DistributedCreateEmptyFileJob();
    virtual ~DistributedCreateEmptyFileJob();
    void SetFilePath(const IO::URI & path);
    const IO::URI & GetFilePath();
    void RunJob();
    bool IsAbleToRun();
    Ptr<DistributedJob> Clone();
    virtual void CopyFrom(const Ptr<DistributedCreateEmptyFileJob> & job);
private:
    void WriteXmlContent(const Ptr<IO::XmlWriter> & writer);
    void ReadXmlContent(const Ptr<IO::XmlReader> & reader);
    void UpdateJob();
    IO::URI filePath;
};
inline
void
DistributedCreateEmptyFileJob::SetFilePath(const IO::URI & path)
{
    n_assert(Ready == this->jobState);
    this->filePath = path;
}
inline
const IO::URI &
DistributedCreateEmptyFileJob::GetFilePath()
{
    return this->filePath;
}
}