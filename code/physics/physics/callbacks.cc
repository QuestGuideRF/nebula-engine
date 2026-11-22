//------------------------------------------------------------------------------
//  callbacks.cc
//  (C) 2019-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "foundation/stdneb.h"
#include "PxConfig.h"
#include "PxPhysicsAPI.h"
#include "callbacks.h"
#include "util/string.h"

using namespace physx;

namespace Physics
{

//------------------------------------------------------------------------------
/**
*/    
void
ErrorCallback::reportError(PxErrorCode::Enum code, const char* message, const char* file, int line)
{
    const char* errorCode = NULL;

    switch (code)
    {
        case PxErrorCode::eNO_ERROR:
            errorCode = "нет ошибки";
        break;
        case PxErrorCode::eINVALID_PARAMETER:
            errorCode = "неверный параметр";
        break;
        case PxErrorCode::eINVALID_OPERATION:
            errorCode = "недопустимая операция";
        break;
        case PxErrorCode::eOUT_OF_MEMORY:
            errorCode = "не хватает памяти";
        break;
        case PxErrorCode::eDEBUG_INFO:
            errorCode = "info";
        break;
        case PxErrorCode::eDEBUG_WARNING:
            errorCode = "warning";
        break;
        case PxErrorCode::ePERF_WARNING:
            errorCode = "предупреждение о производительности";
        break;
        case PxErrorCode::eABORT:
            errorCode = "abort";
        break;
        case PxErrorCode::eINTERNAL_ERROR:
            errorCode = "внутренняя ошибка";
        break;
        case PxErrorCode::eMASK_ALL:
            errorCode = "неизвестная ошибка";
        break;
    }

    n_assert(errorCode);
    if(errorCode)
    {
        Util::String errorMessage = Util::String::Sprintf("%s (%d): PhysX: %s: %s\n", file, line, errorCode, message);
    
        if(code == PxErrorCode::eABORT)
        {
            n_error(errorMessage.AsCharPtr());
        }
        else
        {
            n_dbgout(errorMessage.AsCharPtr());
        }
    }
}
}
