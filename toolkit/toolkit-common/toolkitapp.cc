//------------------------------------------------------------------------------
//  toolkitapp.cc
//  (C) 2008 г. Радон Лабс ГмбХ
//  (C) 2013–2016 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "foundation/stdneb.h"
#include "io/assignregistry.h"
#include "toolkitapp.h"
#include "system/nebulasettings.h"

namespace ToolkitUtil
{
using namespace IO;
using namespace Util;

//------------------------------------------------------------------------------
/**
*/
ToolkitApp::ToolkitApp() :
    toolkitVersion(NEBULA_TOOLKIT_VERSION),
    waitForKey(false),
#if __WIN32__
    platform(Platform::Win32)
#elif __LINUX__
    platform(Platform::Linux)
#endif
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
bool
ToolkitApp::Open()
{
    if (ConsoleApplication::Open())
    {
        // необходимо отключить файловую систему ZIP в инструментах!
        IoServer::Instance()->SetArchiveFileSystemEnabled(false);

        if (System::NebulaSettings::Exists("gscept", "ToolkitShared", "path"))
        {
            Util::String toolDir = System::NebulaSettings::ReadString("gscept", "ToolkitShared", "path");
            Core::CoreServer::Instance()->SetToolDirectory(toolDir);
            AssignRegistry::Instance()->SetAssign(Assign("tool", toolDir.AsCharPtr()));
        }

        // добавить обработчик инструментария для структурированного ведения журналов
        this->handler = ToolkitUtil::ToolkitConsoleHandler::Create();
        IO::Console::Instance()->AttachHandler(this->handler.cast<IO::ConsoleHandler>());
        return true;
    }
    return false;
}

//------------------------------------------------------------------------------
/**
*/
void
ToolkitApp::Close()
{
    IO::Console::Instance()->RemoveHandler(this->handler.cast<IO::ConsoleHandler>());
    this->handler = nullptr;
    ConsoleApplication::Close();
}

//------------------------------------------------------------------------------
/**
*/
bool
ToolkitApp::ParseCmdLineArgs()
{
    bool help = this->args.GetBoolFlag("-help");
    if (help)
    {
        this->ShowHelp();
        return false;
    }
    if(this->args.HasArg("-platform"))
    {
        this->platform = Platform::FromString(this->args.GetString("-platform"));
    }
    this->waitForKey = this->args.GetBoolFlag("-waitforkey");
    return true;
}

//------------------------------------------------------------------------------
/**
*/
bool
ToolkitApp::SetupProjectInfo()
{
    // прочитайте файл projectinfo.json
    ProjectInfo::Result res = this->projectInfo.Setup();

    switch (res)
    {
    case ProjectInfo::NoProjectInfoFile:
        n_printf("В папке проекта не найдена информация о проекте, если предположить, что используются значения по умолчанию\n");
        AssignRegistry::Instance()->SetAssign(Assign("dst", "proj:export"));
        AssignRegistry::Instance()->SetAssign(Assign("int", "proj:intermediate"));
        return true;

    case ProjectInfo::Success:
        AssignRegistry::Instance()->SetAssign(Assign("dst", this->projectInfo.GetAttr("DestDir")));
        AssignRegistry::Instance()->SetAssign(Assign("int", this->projectInfo.GetAttr("IntermediateDir")));

        return true;
    default:
        n_printf("%s\n", this->projectInfo.GetErrorString(res).AsCharPtr());
        this->SetReturnCode(10);
    }
    return false;
}

//------------------------------------------------------------------------------
/**
*/
void
ToolkitApp::ShowHelp()
{
    n_printf("Текст справки Generic ToolkitApp. ИСПРАВИТЬСЯ!!!\n");
}

} // пространство имен ToolkitUtil
