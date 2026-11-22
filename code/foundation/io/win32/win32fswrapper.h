#pragma once
#include "core/types.h"
#include "util/string.h"
#include "util/array.h"
#include "io/stream.h"
#include "io/filetime.h"
namespace Win32
{
class Win32FSWrapper
{
public:
    typedef HANDLE Handle;
    static Handle OpenFile(const Util::String& path, IO::Stream::AccessMode accessMode, IO::Stream::AccessPattern accessPattern, DWORD flagsAndAttributes = 0);
    static void CloseFile(Handle h);
    static void Write(Handle h, const void* buf, IO::Stream::Size numBytes);
    static IO::Stream::Size Read(Handle h, void* buf, IO::Stream::Size numBytes);
    static char* Map(Handle h, IO::Stream::AccessMode accessMode, Handle& mappedHandle);
    static void Unmap(Handle mapHandle, char* buf);
    static void Seek(Handle h, IO::Stream::Offset offset, IO::Stream::SeekOrigin orig);
    static IO::Stream::Position Tell(Handle h);
    static void Flush(Handle h);
    static bool Eof(Handle h);
    static IO::Stream::Size GetFileSize(Handle h);
    static void SetReadOnly(const Util::String& path, bool readOnly);
    static bool IsReadOnly(const Util::String& path);
    static bool IsLocked(const Util::String& path);
    static bool DeleteFile(const Util::String& path);
    static bool ReplaceFile(const Util::String& source, const Util::String& target);
    static bool DeleteDirectory(const Util::String& path);
    static bool FileExists(const Util::String& path);
    static bool DirectoryExists(const Util::String& path);
    static void SetFileWriteTime(const Util::String& path, IO::FileTime fileTime);
    static IO::FileTime GetFileWriteTime(const Util::String& path);
    static bool CreateDirectory(const Util::String& path);
    static Util::String CreateTemporaryFilename(const Util::String& path);
    static Util::Array<Util::String> ListFiles(const Util::String& dirPath, const Util::String& pattern);
    static Util::Array<Util::String> ListDirectories(const Util::String& dirPath, const Util::String& pattern);
    static Util::String GetUserDirectory();
    static Util::String GetAppDataDirectory();
    static Util::String GetTempDirectory();
    static Util::String GetHomeDirectory();
    static Util::String GetBinDirectory();
    static Util::String GetProgramsDirectory();
    static Util::String GetCurrentDirectory();
    static bool IsDeviceName(const Util::String& str);
private:
    static void ConvertToWidePath(const Util::String& src, ushort* dst, SizeT dstNumBytes);
};
};