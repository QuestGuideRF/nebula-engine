#pragma once
#include "scripting/scriptserver.h"
#include "util/string.h"
namespace Scripting
{
class PythonServer : public ScriptServer
{
    __DeclareClass(PythonServer);
    __DeclareSingleton(PythonServer);
public:
    PythonServer();
    virtual ~PythonServer();
    bool Open();
    void Close();
    void AddModulePath(const IO::URI & folder);
    bool Eval(const Util::String& str);
    bool EvalFile(const IO::URI& file);
private:
};
}