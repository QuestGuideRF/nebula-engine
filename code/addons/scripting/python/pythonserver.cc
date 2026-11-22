//------------------------------------------------------------------------------
//  pythonserver.cc
//  (C) 2018-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#define NOMINMAX
#include "foundation/stdneb.h"
#include "scripting/python/pythonserver.h"
#include "nanobind/nanobind.h"
#include "conversion.h"
#include "PyLogHook.h"
#include "io/ioserver.h"
#include "io/textreader.h"

using namespace IO;

namespace Scripting
{
__ImplementClass(Scripting::PythonServer, 'PYSS', Scripting::ScriptServer);
__ImplementSingleton(Scripting::PythonServer);

using namespace Util;
using namespace IO;

//------------------------------------------------------------------------------
/**
*/
PythonServer::PythonServer() 
{
    __ConstructSingleton;
}

//------------------------------------------------------------------------------
/**
*/
PythonServer::~PythonServer()
{
    if (this->IsOpen())
    {
        this->Close();
    }
    __DestructSingleton;
}

//------------------------------------------------------------------------------
/**
*/
bool
PythonServer::Open()
{
    //FIXME уродливый как черт...
    static Util::String linebuffer;
    static Util::String errorbuffer;
    n_assert(!this->IsOpen());    
    if (ScriptServer::Open())
    {
        Python::RegisterNebulaModules();

        for (const Scripting::ScriptModuleInit& init : initFuncs)
        {
            init();
        }
        Py_Initialize();
        
        nanobind::detail::init(nullptr);
        tyti::pylog::redirect_stdout([](const char* msg) 
        {            
            // собирать, пока не получим новую строку
            if (Util::String::StrChr(msg, '\n'))
            {
                n_printf("%s%s", linebuffer.AsCharPtr(), msg);
                linebuffer.Clear();                
            }
            else
            {
                linebuffer.Append(msg);
            }            
        });
        tyti::pylog::redirect_stderr([](const char* msg) 
        {
            // собирать, пока не получим новую строку
            if (Util::String::StrChr(msg, '\n'))
            {
                n_warning("%s%s", errorbuffer.AsCharPtr(), msg);
                errorbuffer.Clear();
            }
            else
            {
                errorbuffer.Append(msg);
            }
        });
        return true;
    }
    return false;
}

//------------------------------------------------------------------------------
/**
*/
void
PythonServer::Close()
{
    n_assert(this->IsOpen());    
    
    // это отменит регистрацию всех команд
    ScriptServer::Close();

    // закрыть питон
    Py_Finalize();
}


//------------------------------------------------------------------------------
/**
    добавить путь к модулю
*/
void
PythonServer::AddModulePath(const IO::URI & folder)
{
    n_assert(this->IsOpen());
    Util::String exec;
    exec.Format("sys.path.insert(0,\"%s\")\n", folder.LocalPath().AsCharPtr());
    this->Eval(exec);
}

//------------------------------------------------------------------------------
/**
    Оценивает фрагмент кода Python в строке.
*/
bool
PythonServer::Eval(const String& str)
{
    n_assert(this->IsOpen());    
    if (!str.IsValid())
    {
        return false;
    }

    return 0 != PyRun_SimpleString(str.AsCharPtr());
}


//------------------------------------------------------------------------------
/**
    Оценить скрипт в файле
*/
bool
PythonServer::EvalFile(const IO::URI& file)
{
    n_assert(this->IsOpen());
    auto stream = IO::IoServer::Instance()->CreateStream(file);
    Ptr<TextReader> reader = IO::TextReader::Create();
    reader->SetStream(stream);
    if (reader->Open())
    {
        Util::String str = reader->ReadAll();
        reader->Close();
        return this->Eval(str);
    }
    return false;
}



} // пространство имен
