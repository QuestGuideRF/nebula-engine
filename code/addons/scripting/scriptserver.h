#pragma once
#include "core/refcounted.h"
#include "core/singleton.h"
#include "io/uri.h"
#include "util/variant.h"
#include "util/delegate.h"
namespace Scripting
{
using ScriptModuleInit = Util::Delegate<void()>;
class ScriptServer : public Core::RefCounted
{
    __DeclareClass(ScriptServer);
    __DeclareSingleton(ScriptServer);
public:
    ScriptServer();
    virtual ~ScriptServer();
    virtual bool Open();
    virtual void Close();
    bool IsOpen() const;
    void SetDebug(const bool b);
    virtual void AddModulePath(const IO::URI & folder) { n_assert(false); }
    virtual bool Eval(const Util::String& str) { return false; }
    virtual bool EvalFile(const IO::URI& file) { return false; }
    static void RegisterModuleInit(const ScriptModuleInit& init);
protected:
    static Util::Array<ScriptModuleInit> initFuncs;
    bool isOpen;
    bool debug;
};
inline
bool
ScriptServer::IsOpen() const
{
    return this->isOpen;
}
inline void
ScriptServer::SetDebug(const bool b)
{
    this->debug = b;
}
inline
void
ScriptServer::RegisterModuleInit(const ScriptModuleInit& init)
{
    initFuncs.Append(init);
}
}