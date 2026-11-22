#pragma once
#include "core/types.h"
#include "util/string.h"
#include "util/array.h"
#include "io/stream.h"
#include "io/filetime.h"
namespace OSX
{
class OSXFSWrapper
{
public:
    typedef FILE* Handle;
    static Handle OpenFile(const Util::String& path, IO::Stream::AccessMode accessMode, IO::Stream::AccessPattern accessPattern);
    static void CloseFile(Handle h);
    static void Write(Handle h, const void* buf, IO::Stream::Size numBytes);
    static IO::Stream::Size Read(Handle h, void* buf, IO::Stream::Size numBytes);
    static void Seek(Handle h, IO::Stream::Offset offset, IO::Stream::SeekOrigin orig);
    static IO::Stream::Position Tell(Handle h);
    static void Flush(Handle h);
    static bool Eof(Handle h);
    static IO::Stream::Size GetFileSize(Handle h);
    static void SetReadOnly(const Util::String& path, bool readOnly);
    static bool IsReadOnly(const Util::String& path);
    static bool DeleteFile(const Util::String& path);
    static bool DeleteDirectory(const Util::String& path);
    static bool FileExists(const Util::String& path);
    static bool DirectoryExists(const Util::String& path);
    static void SetFileWriteTime(const Util::String& path, IO::FileTime fileTime);
    static IO::FileTime GetFileWriteTime(const Util::String& path);
    static bool CreateDirectory(const Util::String& path);
    static Util::Array<Util::String> ListFiles(const Util::String& dirPath, const Util::String& pattern);
    static Util::Array<Util::String> ListDirectories(const Util::String& dirPath, const Util::String& pattern);
    static Util::String GetUserDirectory();
    static Util::String GetAppDataDirectory();
    static Util::String GetTempDirectory();
    static Util::String GetHomeDirectory();
    static Util::String GetBinDirectory();
    static Util::String GetProgramsDirectory();
    static bool IsDeviceName(const Util::String& str);
    static const char* ConvertPath(const Util::String& str);
};
};