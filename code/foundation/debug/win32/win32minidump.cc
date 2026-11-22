//------------------------------------------------------------------------------
//  win32minidump.cc
//  (C) 2007 Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "debug/win32/win32minidump.h"

namespace Win32
{
using namespace Util;

//------------------------------------------------------------------------------
/**
    Этот статический метод регистрирует наш собственный обработчик исключений в Windows.
*/
void
Win32MiniDump::Setup()
{
    SetUnhandledExceptionFilter(Win32MiniDump::ExceptionCallback);
}

//------------------------------------------------------------------------------
/**
    This method is called by n_assert() and n_error() to write out
    a minidump file.
*/
bool
Win32MiniDump::WriteMiniDump()
{
    return Win32MiniDump::WriteMiniDumpInternal(0);
}

//------------------------------------------------------------------------------
/**
    Private method to write a mini-dump with extra exception info. This 
    method is either called from the public WriteMiniDump() method or
    from the ExceptionCallback() function.
*/
bool
Win32MiniDump::WriteMiniDumpInternal(EXCEPTION_POINTERS* exceptionInfo)
{
#if (NEBULA_ENABLE_MINIDUMPS)
    String dumpFilename = BuildMiniDumpFilename();
    if (dumpFilename.IsValid())
    {
        HANDLE hFile = CreateFile(dumpFilename.AsCharPtr(), // lpFileName
                                  GENERIC_WRITE,            // dwDesiredAccess
                                  FILE_SHARE_READ,          // dwShareMode
                                  0,                        // lpSecurityAttributes
                                  CREATE_ALWAYS,            // dwCreationDisposition,
                                  FILE_ATTRIBUTE_NORMAL,    // dwFlagsAndAttributes
                                  NULL);                    // hTemplateFile
        if (NULL != hFile)
        {
            HANDLE hProc = GetCurrentProcess();
            DWORD procId = GetCurrentProcessId();
            BOOL res = FALSE;
            if (NULL != exceptionInfo)
            {
                // доступна расширенная информация об исключении
                MINIDUMP_EXCEPTION_INFORMATION extInfo = { 0 };
                extInfo.ThreadId = GetCurrentThreadId();
                extInfo.ExceptionPointers = exceptionInfo;
                extInfo.ClientPointers = TRUE;
                res = MiniDumpWriteDump(hProc, procId, hFile, MiniDumpNormal, &extInfo, NULL, NULL);
            }
            else
            {
                // расширенная информация об исключении недоступна
                res = MiniDumpWriteDump(hProc, procId, hFile, MiniDumpNormal, NULL, NULL, NULL);
            }
            CloseHandle(hFile);
            return true;
        }
    }
#endif
    return false;
}

//------------------------------------------------------------------------------
/**
*/
String
Win32MiniDump::BuildMiniDumpFilename()
{
    String dumpFilename;

    // получить имя файла нашего модуля прямо из Windows
    TCHAR buf[512];
    Memory::Clear(buf, sizeof(buf));
    DWORD numBytes = GetModuleFileName(NULL, buf, sizeof(buf) - 1);
    if (numBytes > 0)
    {
        String modulePath(buf);
        String moduleName = modulePath.ExtractFileName();
        moduleName.StripFileExtension();
        modulePath = modulePath.ExtractToLastSlash();

        // получить текущее время календаря
        SYSTEMTIME t;
        ::GetLocalTime(&t);
        String timeStr;
        timeStr.Format("%04d-%02d-%02d_%02d-%02d-%02d", t.wYear, t.wMonth, t.wDay, t.wHour, t.wMinute, t.wSecond);
        
        // создать имя файла дампа
        dumpFilename.Format("%s%s_%s.dmp", modulePath.AsCharPtr(), moduleName.AsCharPtr(), timeStr.AsCharPtr());
    }
    return dumpFilename;
}

//------------------------------------------------------------------------------
/**
    Функция обработчика исключений, вызываемая Windows, когда что-то
    случается неожиданное.
*/
LONG WINAPI
Win32MiniDump::ExceptionCallback(EXCEPTION_POINTERS* exceptionInfo)
{
    Win32MiniDump::WriteMiniDumpInternal(exceptionInfo);
    return EXCEPTION_CONTINUE_SEARCH;
}

} // Отладка пространства имен
