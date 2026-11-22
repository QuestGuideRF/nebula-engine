//------------------------------------------------------------------------------
//  osxfswrapper.cc
//  (C) 2010 Радон Лабс ГмбХ
//  (C) 2013–2018 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "stdneb.h"
#include "osxfswrapper.h"
#include "core/sysfunc.h"

namespace OSX
{
using namespace Util;
using namespace Core;
using namespace IO;
    
//------------------------------------------------------------------------------
/**
*/
OSXFSWrapper::Handle
OSXFSWrapper::OpenFile(const String& path, Stream::AccessMode accessMode, Stream::AccessPattern accessPattern)
{
    n_error("OSXFSWrapper::OpenFile(): РЕАЛИЗУЙТЕ МЕНЯ!\n");
    return 0;
}
    
//------------------------------------------------------------------------------
/**
*/
void
OSXFSWrapper::CloseFile(Handle handle)
{
    n_error("OSXFSWrapper::CloseFile(): РЕАЛИЗУЙТЕ МЕНЯ!\n");
}
    
//------------------------------------------------------------------------------
/**
*/
void
OSXFSWrapper::Write(Handle handle, const void* buf, Stream::Size numBytes)
{
    n_error("OSXFSWrapper::Write(): РЕАЛИЗУЙТЕ МЕНЯ!\n");
}
    
//------------------------------------------------------------------------------
/**
*/
Stream::Size
OSXFSWrapper::Read(Handle handle, void* buf, Stream::Size numBytes)
{
    n_error("OSXFSWrapper::Read(): РЕАЛИЗУЙТЕ МЕНЯ!\n");
    return 0;
}
    
//------------------------------------------------------------------------------
/**
*/
void
OSXFSWrapper::Seek(Handle handle, Stream::Offset offset, Stream::SeekOrigin orig)
{
    n_error("OSXFSWrapper::Seek(): РЕАЛИЗУЙТЕ МЕНЯ!\n");
}
    
//------------------------------------------------------------------------------
/**
*/
Stream::Position
OSXFSWrapper::Tell(Handle handle)
{
    n_error("OSXFSWrapper::Tell(): РЕАЛИЗУЙТЕ МЕНЯ!\n");
    return 0;
}
    
//------------------------------------------------------------------------------
/**
*/
void
OSXFSWrapper::Flush(Handle handle)
{
    n_error("OSXFSWrapper::Flush(): РЕАЛИЗУЙТЕ МЕНЯ!\n");
}
    
//------------------------------------------------------------------------------
/**
*/
bool
OSXFSWrapper::Eof(Handle handle)
{
    n_error("OSXFSWrapper::Eof(): РЕАЛИЗУЙТЕ МЕНЯ!\n");
    return false;
}
    
//------------------------------------------------------------------------------
/**
*/
Stream::Size
OSXFSWrapper::GetFileSize(Handle handle)
{
    n_error("OSXFSWrapper::GetFileSize(): РЕАЛИЗУЙТЕ МЕНЯ!\n");
    return 0;
}
    
//------------------------------------------------------------------------------
/**
*/
void
OSXFSWrapper::SetReadOnly(const String& path, bool readOnly)
{
    n_error("OSXFSWrapper::SetReadOnly(): РЕАЛИЗУЙТЕ МЕНЯ!\n");
}

//------------------------------------------------------------------------------
/**
*/
bool
OSXFSWrapper::IsReadOnly(const String& path)
{
    n_error("OSXFSWrapper::IsReadOnly(): РЕАЛИЗУЙТЕ МЕНЯ!\n");
    return false;
}
    
//------------------------------------------------------------------------------
/**
 */
bool
OSXFSWrapper::DeleteFile(const String& path)
{
    n_error("OSXFSWrapper::DeleteFile(): РЕАЛИЗУЙТЕ МЕНЯ!\n");
    return false;
}
    
//------------------------------------------------------------------------------
/**
*/
bool
OSXFSWrapper::DeleteDirectory(const String& path)
{
    n_error("OSXFSWrapper::DeleteDirectory(): РЕАЛИЗУЙТЕ МЕНЯ!\n");
    return false;
}
    
//------------------------------------------------------------------------------
/**
*/
bool
OSXFSWrapper::FileExists(const String& path)
{
    n_error("OSXFSWrapper::FileExists(): РЕАЛИЗУЙТЕ МЕНЯ!\n");
    return false;
}
    
//------------------------------------------------------------------------------
/**
*/
bool
OSXFSWrapper::DirectoryExists(const String& path)
{
    n_error("OSXFSWrapper::DirectoryExists(): РЕАЛИЗУЙТЕ МЕНЯ!\n");
    return false;
}
    
//------------------------------------------------------------------------------
/**
*/
void
OSXFSWrapper::SetFileWriteTime(const String& path, FileTime fileTime)
{
    n_error("OSXFSWrapper::SetFileWriteTime() не реализовано!");
}
    
//------------------------------------------------------------------------------
/**
*/
FileTime
OSXFSWrapper::GetFileWriteTime(const String& path)
{
    n_error("OSXFSWrapper::GetFileWriteTime() не реализовано!");
    return FileTime();
}
    
//------------------------------------------------------------------------------
/**
*/
bool
OSXFSWrapper::CreateDirectory(const String& path)
{
    n_error("OSXFSWrapper::CreateDirectory(): НЕ РЕАЛИЗОВАНО!");
    return false;
}
    
//------------------------------------------------------------------------------
/**
*/
Array<String>
OSXFSWrapper::ListFiles(const String& path, const String& pattern)
{
    n_error("OSXFSWrapper::ListFiles(): НЕ РЕАЛИЗОВАНО!");
    return Array<String>();
}
    
//------------------------------------------------------------------------------
/**
 */
Array<String>
OSXFSWrapper::ListDirectories(const String& path, const String& pattern)
{
    n_error("OSXFSWrapper::ListDirectories(): НЕ РЕАЛИЗОВАНО!");
    return Array<String>();
}
    
//------------------------------------------------------------------------------
/**
 */
String
OSXFSWrapper::GetUserDirectory()
{
    n_error("OSXFSWrapper::GetUserDirectory(): НЕ РЕАЛИЗОВАНО!");
    return "";
}
    
//------------------------------------------------------------------------------
/**
 */
String
OSXFSWrapper::GetAppDataDirectory()
{  
    n_error("OSXFSWrapper::GetAppDataDirectory(): НЕ РЕАЛИЗОВАНО!");
    return "";
}
    
//------------------------------------------------------------------------------
/**
 */
String 
OSXFSWrapper::GetProgramsDirectory()
{  
    n_error("OSXFSWrapper::GetProgramsDirectory(): НЕ РЕАЛИЗОВАНО!");
    return "";
}
    
//------------------------------------------------------------------------------
/**
 */
String
OSXFSWrapper::GetTempDirectory()
{  
    n_error("OSXFSWrapper::GetTempDirectory(): НЕ РЕАЛИЗОВАНО!");
    return "";
}
    
//------------------------------------------------------------------------------
/**
 */
String
OSXFSWrapper::GetBinDirectory()
{
    n_error("OSXFSWrapper::GetBinDirectory(): НЕ РЕАЛИЗОВАНО!");
    return "";
}
    
//------------------------------------------------------------------------------
/**
 */
String
OSXFSWrapper::GetHomeDirectory()
{
    n_error("OSXFSWrapper::GetHomeDirectory(): НЕ РЕАЛИЗОВАНО!");
    return "";
}
    
//------------------------------------------------------------------------------
/**
 */
bool
OSXFSWrapper::IsDeviceName(const Util::String& str)
{
    if (str == "OSX") return true;
    else return false;
}
    
//------------------------------------------------------------------------------
/**
 */
const char*
OSXFSWrapper::ConvertPath(const Util::String& str)
{
    const char* ptr = str.AsCharPtr();
    n_assert((ptr[0] == 'O') && (ptr[1] == 'S') && (ptr[2] == 'X') && (ptr[3] == ':'));
    return &(ptr[4]);
}

} // пространство имен OSX