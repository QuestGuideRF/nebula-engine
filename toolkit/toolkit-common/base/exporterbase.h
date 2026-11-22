#pragma once
#include "core/refcounted.h"
#include "io/uri.h"
#include "toolkit-common/platform.h"
#include "toolkit-common/logger.h"
#include "net/socket/socket.h"
#include "io/console.h"
typedef void (*ExporterProgressCallback) (float progress, const Util::String& status);
typedef void (*ExporterMinMaxCallback) (int min, int max);
namespace Base
{
class ExporterBase : public Core::RefCounted
{
    __DeclareClass(ExporterBase);
public:
    enum ExportFlag
    {
        All,
        Dir,
        File
    };
    ExporterBase();
    virtual ~ExporterBase();
    virtual void Open();
    virtual void Close();
    bool IsOpen() const;
    void WriteIntermediateFile(const IO::URI& sourceFile, Util::Array<IO::URI> const& output);
    virtual void ExportFile(const IO::URI& file);
    virtual void ExportDir(const Util::String& category);
    virtual void ExportAll();
    void SetHasErrors(bool flag);
    const bool HasErrors() const;
    void SetCategory(const Util::String& category);
    void SetFile(const Util::String& file);
    void SetPlatform(ToolkitUtil::Platform::Code platform);
    void SetExportFlag(ExportFlag parseFlag);
    void SetForce(bool force);
    void SetRemote(bool remote);
    void SetProgressCallback(ExporterProgressCallback callback);
    void SetMinMaxCallback(ExporterMinMaxCallback callback);
    void SetLogger(ToolkitUtil::Logger* logger);
    void Progress(float progress, const Util::String& status);
    void SetProgressMinMax(int min, int max);
    int CountExports(const Util::String& dir, const Util::String& ext);
    void SetProgressPrecision(int precision);
protected:
    void ReportError(const char* error, ...);
    bool NeedsConversion(const Util::String& src, const Util::String& dst);
    void ValidateIntermediateFile(Util::String const& filePath);
    void RecurseValidateIntermediates(Util::String const& dir);
    Util::String category;
    Util::String file;
    int precision;
    Ptr<Net::Socket> socket;
    ToolkitUtil::Platform::Code platform;
    ExportFlag exportFlag;
    ExporterProgressCallback progressCallback;
    ExporterMinMaxCallback  minMaxCallback;
    ToolkitUtil::Logger* logger;
    bool hasErrors;
    bool force;
    bool isOpen;
    bool remote;
};
inline bool
ExporterBase::IsOpen() const
{
    return this->isOpen;
}
inline void
ExporterBase::SetHasErrors( bool flag )
{
    this->hasErrors = flag;
}
inline const bool
ExporterBase::HasErrors() const
{
    return this->hasErrors;
}
inline void
ExporterBase::SetProgressPrecision( int precision )
{
    this->precision = precision;
}
inline void
ExporterBase::SetCategory( const Util::String& category )
{
    this->category = category;
}
inline void
ExporterBase::SetFile( const Util::String& file )
{
    this->file = file;
    this->file.StripFileExtension();
}
inline void
ExporterBase::SetPlatform( ToolkitUtil::Platform::Code platform )
{
    this->platform = platform;
}
inline void
ExporterBase::SetExportFlag( ExportFlag exportFlag )
{
    this->exportFlag = exportFlag;
}
inline void
ExporterBase::SetForce( bool force )
{
    this->force = force;
}
inline void
ExporterBase::SetRemote( bool remote )
{
    this->remote = remote;
}
inline void
ExporterBase::SetProgressCallback( ExporterProgressCallback callback )
{
    this->progressCallback = callback;
}
inline void
ExporterBase::SetMinMaxCallback( ExporterMinMaxCallback callback )
{
    this->minMaxCallback = callback;
}
inline void
ExporterBase::SetLogger(ToolkitUtil::Logger* logger)
{
    this->logger = logger;
}
inline void
ExporterBase::ReportError( const char* error, ... )
{
    va_list argList;
    va_start(argList, error);
    if (this->exportFlag == ExporterBase::File)
    {
        IO::Console::Instance()->Error(error, argList);
    }
    else
    {
        IO::Console::Instance()->Warning(error, argList);
    }
    va_end(argList);
}
}