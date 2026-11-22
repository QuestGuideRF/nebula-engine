#pragma once
#include "core/refcounted.h"
#include "core/singleton.h"
#include "util/string.h"
#include "ids/id.h"
#include "io/uri.h"
#include "util/arrayallocator.h"
#include "util/dictionary.h"
#include "system/library.h"
#include "nsconfig.h"
#include "assemblyid.h"
#include "util/delegate.h"
namespace Scripting
{
class NSharpServer : public Core::RefCounted
{
    __DeclareClass(NSharpServer);
    __DeclareSingleton(NSharpServer);
public:
	NSharpServer();
    virtual ~NSharpServer();
    bool Open();
    void Close();
	void SetDebuggingEnabled(bool enabled);
	void WaitForDebuggerToConnect(bool enabled);
    AssemblyId LoadAssembly(IO::URI const& uri);
    int ExecUnmanagedCall(AssemblyId assemblyId, Util::String const& function);
	bool const IsOpen();
    void* GetDelegatePointer(
        AssemblyId assemblyId, Util::String const& assemblyQualifiedFunctionName, Util::String const& assemblyQualifiedDelegateName
    );
    void* GetUnmanagedFuncPointer(AssemblyId assemblyId, Util::String const& assemblyQualifiedFunctionName);
private:
    struct Assembly;
    bool LoadHostFxr();
    void CloseHostFxr();
    Util::ArrayAllocator<Assembly*> assemblies;
	Util::Dictionary<Util::String, uint32_t> assemblyTable;
    bool debuggerEnabled;
	bool waitForDebugger;
	bool isOpen;
    System::Library hostfxr;
};
}