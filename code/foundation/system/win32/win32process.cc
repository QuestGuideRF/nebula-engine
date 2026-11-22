//------------------------------------------------------------------------------
//  win32process.cc
//  (C) 2008 г. Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "win32process.h"
#include <Tlhelp32.h>

namespace Win32
{
using namespace Util;
using namespace IO;

//------------------------------------------------------------------------------
/**
*/
Win32Process::Win32Process() :   
    lauchedProcessInfo(PROCESS_INFORMATION())
{ 
    // empty
}

//------------------------------------------------------------------------------
/**
*/
bool
Win32Process::LaunchWait() const
{
    n_assert(this->exePath.IsValid());

    DWORD creationFlags = 0;
    if (this->noConsoleWindow)
    {
        creationFlags |= CREATE_NO_WINDOW;
    }

    // построить командную строку
    String cmdLine = this->exePath.LocalPath();
    cmdLine.Append(" ");
    cmdLine.Append(this->args);

    if(this->stdoutCaptureStream.isvalid() || this->stderrCaptureStream.isvalid())
    {
        return this->LaunchWithOutputCapturing(creationFlags, cmdLine);
    }
    else
    {
        return this->LaunchWithoutOutputCapturing(creationFlags, cmdLine);
    }
}

//------------------------------------------------------------------------------
/**
    Launch the application process and returns immediately. The state of the launched
    process can be checked by calling IsRunning().
*/
bool
Win32Process::Launch() 
{
    n_assert(this->exePath.IsValid());

    DWORD creationFlags = 0;
    if (this->noConsoleWindow)
    {
        creationFlags |= CREATE_NO_WINDOW;
    }

    // построить командную строку
    String cmdLine = this->exePath.LocalPath();
    cmdLine.Append(" ");
    cmdLine.Append(this->args);

    
    STARTUPINFO startupInfo = { sizeof(STARTUPINFO), 0 };
    
    // настроить дополнительную информацию о запуске для потоковой передачи stdout/stderr
    if(this->stdoutCaptureStream.isvalid() || this->stderrCaptureStream.isvalid())
    {
        startupInfo.dwFlags |= STARTF_USESTDHANDLES;
        startupInfo.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
        if(this->stdoutCaptureStream.isvalid())
        {
            if(!this->CreateStdoutCapturePipe((PHANDLE)&this->stdoutAsyncRead, (PHANDLE)&this->stdoutAsyncWrite))
            {
                return false;
            }
            startupInfo.hStdOutput = this->stdoutAsyncWrite;
        }
        else
        {
            startupInfo.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
        }
        if(this->stderrCaptureStream.isvalid())
        {
            if(!this->CreateStdoutCapturePipe((PHANDLE)&this->stderrAsyncRead, (PHANDLE)&this->stderrAsyncWrite))
            {
                return false;
            }
            startupInfo.hStdError = this->stderrAsyncWrite;   
        }
        else
        {
            startupInfo.hStdError = GetStdHandle(STD_ERROR_HANDLE);
        }
    }
    
    // создайте процесс, верните false, если это не удастся
    if (!CreateProcess(NULL,                        // lpApplicationName
        (LPSTR) cmdLine.AsCharPtr(),                // lpCommandLine
        NULL,                                       // lpProcessAttributes
        NULL,                                       // lpThreadAttributes
        TRUE,                                       // bInheritsHandle
        creationFlags,                              // dwCreationFlags
        NULL,                                       // lpEnvironment
        this->workingDir.LocalPath().AsCharPtr(),   // lpCurrentDirectory
        &startupInfo,                               // lpStartupInfo
        (LPPROCESS_INFORMATION)&(this->lauchedProcessInfo))) // lpProcessInformation
        {
            if(this->stdoutCaptureStream.isvalid())
            {
                CloseHandle(this->stdoutAsyncRead);
                CloseHandle(this->stdoutAsyncWrite);
            }
            if(this->stderrCaptureStream.isvalid())
            {
                CloseHandle(this->stderrAsyncRead);
                CloseHandle(this->stderrAsyncWrite);
            }
            return false;
        }
    
    // проверьте, открыт ли дополнительный поток stdout/stderr, иначе вернитесь с false
    if(this->stdoutCaptureStream.isvalid())
    {
        this->stdoutCaptureStream->SetAccessMode(Stream::WriteAccess);
        if (!this->stdoutCaptureStream->Open())
        {
            CloseHandle(this->stdoutAsyncRead);
            CloseHandle(this->stdoutAsyncWrite);
            return false;
        }
    }
    if(this->stderrCaptureStream.isvalid())
    {
        this->stderrCaptureStream->SetAccessMode(Stream::WriteAccess);
        if (!this->stderrCaptureStream->Open())
        {
            CloseHandle(this->stderrAsyncRead);
            CloseHandle(this->stderrAsyncWrite);
            return false;
        }
    }

    this->isRunning = true;
    return true;
}

//------------------------------------------------------------------------------
/**
    Получает состояние приложения.
*/
bool
Win32Process::IsRunning()
{
    if(this->isRunning)
    {
        DWORD exitCode = 0;
        GetExitCodeProcess(this->lauchedProcessInfo.hProcess, &exitCode);
        if (exitCode != STILL_ACTIVE)
        {
            this->UpdateStdoutStream();
            // cleanup
            if(this->stdoutCaptureStream.isvalid())
            {
                this->stdoutCaptureStream->Close();
                CloseHandle(this->stdoutAsyncRead);
                CloseHandle(this->stdoutAsyncWrite);
            }
            if(this->stderrCaptureStream.isvalid())
            {
                this->stderrCaptureStream->Close();
                CloseHandle(this->stderrAsyncRead);
                CloseHandle(this->stderrAsyncWrite);
            }
            CloseHandle(this->lauchedProcessInfo.hProcess);
            CloseHandle(this->lauchedProcessInfo.hThread);  

            this->isRunning = false;
        }
        this->UpdateStdoutStream();
    }
    
    return this->isRunning;
}
//------------------------------------------------------------------------------
/**
    Reads all arrived data from stdout since the last call of this method and puts it to
    the stream.
*/
void
Win32Process::UpdateStdoutStream()
{
    if(this->stdoutCaptureStream.isvalid())
    {
        n_assert(0 != this->stdoutAsyncRead);
        n_assert(0 != this->lauchedProcessInfo.hProcess);

        numAsyncBytesRead = 0;
        numAsyncBytesAvailable = 0;
        Memory::Clear(asyncBuffer, asyncBufferSize);

        // посмотреть, есть ли какие-нибудь данные для чтения
        PeekNamedPipe(this->stdoutAsyncRead, asyncBuffer, asyncBufferSize, &numAsyncBytesRead, &numAsyncBytesAvailable, NULL);

        if (numAsyncBytesRead != 0)
        {
            if (numAsyncBytesAvailable > asyncBufferSize)
            {
                while (numAsyncBytesRead >= asyncBufferSize)
                {
                    ReadFile(this->stdoutAsyncRead, asyncBuffer, asyncBufferSize, &numAsyncBytesRead, NULL);
                    this->stdoutCaptureStream->Write(asyncBuffer, numAsyncBytesRead);
                }
            }
            else
            {
                ReadFile(this->stdoutAsyncRead, asyncBuffer, asyncBufferSize, &numAsyncBytesRead, NULL);
                this->stdoutCaptureStream->Write(asyncBuffer, numAsyncBytesRead);
            }
        }
    }
    if(this->stderrCaptureStream.isvalid())
    {
        n_assert(0 != this->stderrAsyncRead);
        n_assert(0 != this->lauchedProcessInfo.hProcess);

        numAsyncBytesRead = 0;
        numAsyncBytesAvailable = 0;
        Memory::Clear(asyncBuffer, asyncBufferSize);

        // посмотреть, есть ли какие-нибудь данные для чтения
        PeekNamedPipe(this->stderrAsyncRead, asyncBuffer, asyncBufferSize, &numAsyncBytesRead, &numAsyncBytesAvailable, NULL);

        if (numAsyncBytesRead != 0)
        {
            if (numAsyncBytesAvailable > asyncBufferSize)
            {
                while (numAsyncBytesRead >= asyncBufferSize)
                {
                    ReadFile(this->stderrAsyncRead, asyncBuffer, asyncBufferSize, &numAsyncBytesRead, NULL);
                    this->stderrCaptureStream->Write(asyncBuffer, numAsyncBytesRead);
                }
            }
            else
            {
                ReadFile(this->stderrAsyncRead, asyncBuffer, asyncBufferSize, &numAsyncBytesRead, NULL);
                this->stderrCaptureStream->Write(asyncBuffer, numAsyncBytesRead);
            }
        }
    }
}

//------------------------------------------------------------------------------
/**
*/
bool
Win32Process::LaunchWithoutOutputCapturing(DWORD creationFlags, const String& cmdLine) const
{
    STARTUPINFO startupInfo = { sizeof(STARTUPINFO), 0 };
    PROCESS_INFORMATION processInfo = { 0 };

    if (!CreateProcess(NULL,                            // lpApplicationName
                       (LPSTR) cmdLine.AsCharPtr(),     // lpCommandLine
                       NULL,                            // lpProcessAttributes
                       NULL,                            // lpThreadAttributes
                       FALSE,                           // bInheritsHandle
                       creationFlags,                   // dwCreationFlags
                       NULL,                            // lpEnvironment
                       this->workingDir.LocalPath().AsCharPtr(),    // lpCurrentDirectory
                       &startupInfo,                    // lpStartupInfo
                       &processInfo))                   // lpProcessInformation
    {
        return false;
    }

    // дождитесь завершения процесса
    WaitForSingleObject(processInfo.hProcess, INFINITE);
    
    // cleanup
    CloseHandle(processInfo.hProcess);
    CloseHandle(processInfo.hThread);

    return true;
}

//------------------------------------------------------------------------------
/**
*/
bool
Win32Process::LaunchWithOutputCapturing(DWORD creationFlags, const String& cmdLine) const
{ 
    HANDLE stdoutRead = 0;
    HANDLE stdoutWrite = 0;
    HANDLE stderrRead = 0;
    HANDLE stderrWrite = 0;

    // создать информацию о запуске
    STARTUPINFO startupInfo = { sizeof(STARTUPINFO), 0 };
    startupInfo.dwFlags |= STARTF_USESTDHANDLES;
    startupInfo.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    
    // создать стандартный вывод
    if(this->stdoutCaptureStream.isvalid())
    {
        if(!this->CreateStdoutCapturePipe(&stdoutRead, &stdoutWrite))
        {
            return false;
        }
        startupInfo.hStdOutput = stdoutWrite;
    }
    else
    {
        startupInfo.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    }

    // создать вывод stderr
    if(this->stderrCaptureStream.isvalid())
    {
        if(!this->CreateStdoutCapturePipe(&stderrRead, &stderrWrite))
        {
            return false;
        }
        startupInfo.hStdError = stderrWrite;
    }
    else
    {
        startupInfo.hStdError = GetStdHandle(STD_ERROR_HANDLE);
    }
    
    // создать процесс
    PROCESS_INFORMATION processInfo = { 0 };

    if (!CreateProcess(NULL,                            // lpApplicationName
                       (LPSTR) cmdLine.AsCharPtr(),     // lpCommandLine
                       NULL,                            // lpProcessAttributes
                       NULL,                            // lpThreadAttributes
                       TRUE,                            // bInheritsHandle
                       creationFlags,                   // dwCreationFlags
                       NULL,                            // lpEnvironment
                       this->workingDir.LocalPath().AsCharPtr(),    // lpCurrentDirectory
                       &startupInfo,                    // lpStartupInfo
                       &processInfo))                   // lpProcessInformation
    {
        // очистка в случае неудачи
        if(this->stdoutCaptureStream.isvalid())
        {
            CloseHandle(stdoutRead);
            CloseHandle(stdoutWrite);
        }
        if(this->stderrCaptureStream.isvalid())
        {
            CloseHandle(stderrRead);
            CloseHandle(stderrWrite);
        }
        return false;
    }
    
    // try to open stdout stream
    if(this->stdoutCaptureStream.isvalid())
    {
        this->stdoutCaptureStream->SetAccessMode(Stream::WriteAccess);
        if (!this->stdoutCaptureStream->Open())
        {
            CloseHandle(stdoutRead);
            CloseHandle(stdoutWrite);
            return false;
        }
    }

    // try to open stderr stream
    if(this->stderrCaptureStream.isvalid())
    {
        this->stderrCaptureStream->SetAccessMode(Stream::WriteAccess);
        if (!this->stderrCaptureStream->Open())
        {
            CloseHandle(stderrRead);
            CloseHandle(stderrWrite);
            return false;
        }
    }
    
    // захват вывода
    this->CaptureOutput(stdoutRead, stderrRead, processInfo.hProcess);

    // cleanup
    CloseHandle(processInfo.hProcess);
    CloseHandle(processInfo.hThread);
    if(this->stdoutCaptureStream.isvalid())
    {
        this->stdoutCaptureStream->Close();
        CloseHandle(stdoutRead);
        CloseHandle(stdoutWrite);
    }
    if(this->stderrCaptureStream.isvalid())
    {
        this->stderrCaptureStream->Close();
        CloseHandle(stderrRead);
        CloseHandle(stderrWrite);
    }
    return true;
}

//------------------------------------------------------------------------------
/**
*/
bool
Win32Process::CreateStdoutCapturePipe(PHANDLE stdoutRead, PHANDLE stdoutWrite) const
{
    // настроить атрибуты безопасности канала
    SECURITY_ATTRIBUTES security;
    security.bInheritHandle = TRUE;
    security.lpSecurityDescriptor = 0;
    security.nLength = sizeof(SECURITY_ATTRIBUTES);
    
    // создать канал для стандартного вывода дочернего процесса
    if (!CreatePipe(stdoutRead, stdoutWrite, &security, 4096))
    {
        return false;
    }

    // убедитесь, что дескриптор чтения канала для стандартного вывода не унаследован.
    SetHandleInformation((*stdoutRead), HANDLE_FLAG_INHERIT, 0);
    return true;
}

//------------------------------------------------------------------------------
/**
*/
void
Win32Process::CaptureOutput(HANDLE stdoutRead, HANDLE stderrRead, HANDLE childProcess) const
{
    n_assert(0 != childProcess);
    
    DWORD numBytesRead = 0;
    DWORD numBytesAvailable = 0;
    DWORD exitCode = 0;
    const int bufferSize = 4096;
    char buffer[bufferSize];
    Memory::Clear(buffer, bufferSize);

    // прочитать все с конца канала чтения и записать в поток...
    while(true)
    {
        if(this->stdoutCaptureStream.isvalid())
        {
            n_assert(0 != stdoutRead);
            // посмотреть, есть ли какие-нибудь данные для чтения
            PeekNamedPipe(stdoutRead, buffer, bufferSize, &numBytesRead, &numBytesAvailable, NULL);
            
            if (numBytesRead != 0)
            {
                if (numBytesAvailable > bufferSize)
                {
                    while (numBytesRead >= bufferSize)
                    {
                        ReadFile(stdoutRead, buffer, bufferSize, &numBytesRead, NULL);
                        this->stdoutCaptureStream->Write(buffer, numBytesRead);
                    }
                }
                else
                {
                    ReadFile(stdoutRead, buffer, bufferSize, &numBytesRead, NULL);
                    this->stdoutCaptureStream->Write(buffer, numBytesRead);
                }
            }
        }
        if(this->stderrCaptureStream.isvalid())
        {
            n_assert(0 != stderrRead);
            // посмотреть, есть ли какие-нибудь данные для чтения
            PeekNamedPipe(stderrRead, buffer, bufferSize, &numBytesRead, &numBytesAvailable, NULL);

            if (numBytesRead != 0)
            {
                if (numBytesAvailable > bufferSize)
                {
                    while (numBytesRead >= bufferSize)
                    {
                        ReadFile(stderrRead, buffer, bufferSize, &numBytesRead, NULL);
                        this->stderrCaptureStream->Write(buffer, numBytesRead);
                    }
                }
                else
                {
                    ReadFile(stderrRead, buffer, bufferSize, &numBytesRead, NULL);
                    this->stderrCaptureStream->Write(buffer, numBytesRead);
                }
            }
        }

        // ... пока запущен дочерний процесс
        GetExitCodeProcess(childProcess, &exitCode);
        if (exitCode != STILL_ACTIVE)
        {
            break;
        }
    }
}
 
//------------------------------------------------------------------------------
/**
*/
bool
Win32Process::CheckIfExists()
{
    return CheckIfExists(this->exePath);
}

//------------------------------------------------------------------------------
/**
*/
bool
Win32Process::CheckIfExists(const IO::URI & program)
{
    bool exists = false;
    PROCESSENTRY32 entry;
    entry.dwSize = sizeof(PROCESSENTRY32);
    Util::String procname = program.LocalPath().ExtractFileName();
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
    if (Process32First(snapshot, &entry))
        while (Process32Next(snapshot, &entry))
            if (procname == entry.szExeFile)
                exists = true;
    CloseHandle(snapshot);
    return exists;
}

} // пространство имен ToolkitUtil