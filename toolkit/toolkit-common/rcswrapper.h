#pragma once
#include "core/types.h"
#include "util/string.h"
#include "logger.h"
#include "io/stream.h"
namespace ToolkitUtil
{
class RCSWrapper
{
public:
    RCSWrapper();
    ~RCSWrapper();
    void SetSVNToolLocation(const Util::String& path);
    const Util::String& GetSVNToolLocation() const;
    void SetCommitTimeStampDirectories(const Util::Array<Util::String>& paths);
    const Util::Array<Util::String>& GetCommitTimeStampDirectories() const;
    void SetUpdateTimeStampDirectories(const Util::Array<Util::String>& paths);
    const Util::Array<Util::String>& GetUpdateTimeStampDirectories() const;
    bool Update(Logger& logger);
    bool GetModifiedFiles(Logger& logger, const Util::String& directory, Util::Array<Util::String>& outFiles) const;
private:
    void WriteTempSVNConfigFiles();
    void CopyDirectory(const Util::String& fromDir, const Util::String& toDir);
    bool UpdateDirectory(Logger& logger, const Util::String& directory, bool commitTimeStampMode);
    bool IsSVNDirectory(Logger& logger, const Util::String& directory) const;
    bool IsSVNCommandLineConfigured(Logger& logger) const;
    bool ParseSVNStatusOutput(const Ptr<IO::Stream>& stream, const Util::String& directory, Util::Array<Util::String>& outFiles) const;
    Util::String svnToolLocation;
    Util::Array<Util::String> updateTimeStampDirs;
    Util::Array<Util::String> commitTimeStampDirs;
};
inline void
RCSWrapper::SetSVNToolLocation(const Util::String& path)
{
    this->svnToolLocation = path;
}
inline const Util::String&
RCSWrapper::GetSVNToolLocation() const
{
    return this->svnToolLocation;
}
inline void
RCSWrapper::SetCommitTimeStampDirectories(const Util::Array<Util::String>& dirs)
{
    this->commitTimeStampDirs = dirs;
}
inline const Util::Array<Util::String>&
RCSWrapper::GetCommitTimeStampDirectories() const
{
    return this->commitTimeStampDirs;
}
inline void
RCSWrapper::SetUpdateTimeStampDirectories(const Util::Array<Util::String>& dirs)
{
    this->updateTimeStampDirs = dirs;
}
inline const Util::Array<Util::String>&
RCSWrapper::GetUpdateTimeStampDirectories() const
{
    return this->updateTimeStampDirs;
}
}