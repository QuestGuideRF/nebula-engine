//------------------------------------------------------------------------------
//  setupapplication.cc
//  (C) 2012-2016 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "foundation/stdneb.h"
#include "setupapplication.h"
#include "system/nebulasettings.h"

using namespace System;
using namespace ToolkitUtil;

namespace Tools
{
//------------------------------------------------------------------------------
/**
*/
SetupApplication::SetupApplication()
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
SetupApplication::~SetupApplication()
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
bool 
SetupApplication::Open()
{
    bool retval = ToolkitApp::Open();
    this->DoWork();
    return retval;
}

//------------------------------------------------------------------------------
/**
*/
void 
SetupApplication::DoWork()
{
    switch (this->platform)
    {
    case Platform::Win32:
    case Platform::Linux:
        {
            NebulaSettings::WriteString("gscept","ToolkitShared", "path", IO::URI("root:").GetHostAndLocalPath());
            NebulaSettings::WriteString("gscept","ToolkitShared", "workdir", IO::URI("root:").GetHostAndLocalPath());           
            break;
        }
    }
}

//------------------------------------------------------------------------------
/**
*/
bool 
SetupApplication::ParseCmdLineArgs()
{
    return ToolkitApp::ParseCmdLineArgs();
}

//------------------------------------------------------------------------------
/**
*/
bool 
SetupApplication::SetupProjectInfo()
{
    return true;
}

//------------------------------------------------------------------------------
/**
*/
void 
SetupApplication::ShowHelp()
{
    n_printf("Приложение настройки NebulaT.\n"
        "(C) Индивидуальные авторы, 2012–2016 гг., см. файл АВТОРОВ.\n");
    n_printf("-help --показать эту справку\n"
        "-nody --nody переопределить путь"
        "-working --рабочая папка Nebula (где расположены контейнеры)\n"
        "-project   --nebula project trunk (if empty, attempts to use registry)\n"
        "-платформа --экспортировать платформу");
}
} // Инструменты пространства имен