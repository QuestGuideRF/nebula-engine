#pragma once
#include "core/types.h"
#include "util/string.h"
#include "util/array.h"
#include "core/refcounted.h"
#include "core/singleton.h"
#include "core/ptr.h"
#include "io/consolehandler.h"
#include "threading/threadid.h"
#include "threading/criticalsection.h"
namespace IO
{
class Console : public Core::RefCounted
{
    __DeclareClass(Console);
    __DeclareInterfaceSingleton(Console);
public:
    Console();
    virtual ~Console();
    void Open();
    void Close();
    bool IsOpen() const;
    void Update();
    void AttachHandler(const Ptr<ConsoleHandler>& handler);
    void RemoveHandler(const Ptr<ConsoleHandler>& handler);
    Util::Array<Ptr<ConsoleHandler> > GetHandlers() const;
    bool HasInput() const;
    Util::String GetInput() const;
    void __cdecl Print(const char* fmt, ...);
    void __cdecl Print(const char* fmt, va_list argList);
    void Print(const Util::String& s);
    void __cdecl Error(const char* fmt, ...);
    void __cdecl Error(const char* fmt, va_list argList);
    void __cdecl Warning(const char* fmt, ...);
    void __cdecl Warning(const char* fmt, va_list argList);
    void __cdecl Confirm(const char* fmt, ...);
    void __cdecl Confirm(const char* fmt, va_list argList);
    void __cdecl DebugOut(const char* fmt, ...);
    void __cdecl DebugOut(const char* fmt, va_list argList);
protected:
    Threading::ThreadId creatorThreadId;
    Threading::CriticalSection critSect;
    Util::Array<Ptr<ConsoleHandler> > consoleHandlers;
    bool isOpen;
};
inline bool
Console::IsOpen() const
{
    return this->isOpen;
}
inline Util::Array<Ptr<ConsoleHandler> >
Console::GetHandlers() const
{
    this->critSect.Enter();
    Util::Array<Ptr<ConsoleHandler> > handlers = this->consoleHandlers;
    this->critSect.Leave();
    return handlers;
}
}