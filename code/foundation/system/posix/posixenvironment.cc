//------------------------------------------------------------------------------
//  posixenvironment.cc
//  (C) 2013–2018 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "foundation/stdneb.h"
#include "system/posix/posixenvironment.h"

namespace Posix
{
using namespace Util;

//------------------------------------------------------------------------------
/**
*/
bool
PosixEnvironment::Exists(const String& envVarName)
{
    return NULL != getenv(envVarName.AsCharPtr());    
}

//------------------------------------------------------------------------------
/**
*/
String
PosixEnvironment::Read(const String& envVarName)
{
    char * env = getenv(envVarName.AsCharPtr());
    if(NULL == env)
    {
        n_error("Не удалось прочитать переменную среды %s\n.",envVarName.AsCharPtr());
    }
    Util::String str;
    str.SetCharPtr(env);
    return env;    
}

} // пространство имен Posix