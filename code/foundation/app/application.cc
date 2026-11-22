//------------------------------------------------------------------------------
//  application.cc
//  (C) 2006 г. Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "app/application.h"
#include "core/sysfunc.h"
#include "core/cvar.h"

using namespace Util;

namespace App
{
__ImplementSingleton(App::Application);

//------------------------------------------------------------------------------
/**
*/
Application::Application() :
    companyName("gscept"),
    appName("Приложение Небула"),
    appID("RLTITLEID"), // формат идентификатора приложения/заголовка зависит от используемой платформы
    appVersion("1.00"),
    isOpen(false),
    returnCode(0)
{
    __ConstructSingleton;
    Core::SysFunc::Setup();
}

//------------------------------------------------------------------------------
/**
*/
Application::~Application()
{
    n_assert(!this->isOpen);
}

//------------------------------------------------------------------------------
/**
*/
bool
Application::Open()
{
    n_assert(!this->isOpen);
    this->isOpen = true;

    const Dictionary<String, String>& pairs = this->args.GetPairs();
    for (const auto& item : pairs)
    {
        const String name = item.Key();
        const String value = item.Value();

        const auto GuessType = [](const String& v)
            {
                if (v.IsValidFloat()) return Core::CVar_Float;
                if (v.IsValidInt()) return Core::CVar_Int;
                return Core::CVar_String;
            };
        Core::CVarCreateInfo info;
        info.defaultValue = "";
        info.name = name.c_str();
        info.description = name.c_str();
        info.type = GuessType(value);

        Core::CVar* newCVar = Core::CVarCreate(info);
        n_assert(newCVar != nullptr);
        switch (info.type)
        {
            case Core::CVar_Float:
                Core::CVarWriteFloat(newCVar, value.AsFloat());
                break;
            case Core::CVar_Int:
                Core::CVarWriteInt(newCVar, value.AsInt());
                break;
            case Core::CVar_String:
                Core::CVarWriteString(newCVar, value.c_str());
        }
    }
    
    return true;
}

//------------------------------------------------------------------------------
/**
*/
void
Application::Close()
{
    n_assert(this->isOpen);
    this->isOpen = false;
}

//------------------------------------------------------------------------------
/**
*/
void
Application::Run()
{
    // пусто, переопределить в подклассе
}

//------------------------------------------------------------------------------
/**
    This method must be called right before the main() function's end.
    It will properly cleanup the Nebula runtime, its static objects,
    private heaps and finally produce a refcount leak and mem leak report
    (debug builds only).
*/
void
Application::Exit()
{
    Core::SysFunc::Exit(this->returnCode);
}

} // Приложение пространства имен