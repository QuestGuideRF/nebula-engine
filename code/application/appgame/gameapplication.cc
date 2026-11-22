//------------------------------------------------------------------------------
//  gameapplication.cc
//  (C) 2007 Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "appgame/gameapplication.h"

#include "core/sysfunc.h"
#include "options.h"
#include "core/debug/corepagehandler.h"
#include "threading/debug/threadpagehandler.h"
#include "memory/debug/memorypagehandler.h"
#include "io/debug/iopagehandler.h"
#include "io/logfileconsolehandler.h"
#include "io/debug/consolepagehandler.h"
#include "messaging/messagecallbackhandler.h"
#include "system/nebulasettings.h"
#include "io/fswrapper.h"
#include "jobs2/jobs2.h"
#include "input/inputserver.h"
#include "basegamefeature/basegamefeatureunit.h"
#include "nflatbuffer/flatbufferinterface.h"
#include "profiling/profiling.h"

namespace App
{
__ImplementSingleton(App::GameApplication);
IndexT GameApplication::FrameIndex = -1;
bool GameApplication::editorEnabled = false;

using namespace Util;
using namespace Core;
using namespace IO;
using namespace Http;
using namespace Debug;

//------------------------------------------------------------------------------
/**
*/
GameApplication::GameApplication() :
    exitHandler(this)
#if __NEBULA_HTTP__
    ,defaultTcpPort(2100)
#endif
{
    __ConstructSingleton;
}

//------------------------------------------------------------------------------
/**
*/
GameApplication::~GameApplication()
{
    n_assert(!this->IsOpen());
    __DestructSingleton;
}

//------------------------------------------------------------------------------
/**
*/
bool
GameApplication::Open()
{
    n_assert(!this->IsOpen());
    if (Application::Open())
    {
        // настройка из аргументов строки cmd
        this->SetupAppFromCmdLineArgs();

        // настроить базовую систему выполнения Nebula
        this->coreServer = CoreServer::Create();
        this->coreServer->SetCompanyName(Application::Instance()->GetCompanyName());
        this->coreServer->SetAppName(Application::Instance()->GetAppTitle());

        Util::String root = IO::FSWrapper::GetHomeDirectory();

#if PUBLIC_BUILD
        if(System::NebulaSettings::Exists(Application::Instance()->GetCompanyName(),Application::Instance()->GetAppTitle(),"path"))
        {
            root = System::NebulaSettings::ReadString(Application::Instance()->GetCompanyName(),Application::Instance()->GetAppTitle(),"path");
        }
#else
        if(System::NebulaSettings::Exists("gscept", "ToolkitShared", "workdir"))
        {
            root = System::NebulaSettings::ReadString("gscept", "ToolkitShared", "workdir");
        }
        if (System::NebulaSettings::Exists("gscept", "ToolkitShared", "path"))
        {
            this->coreServer->SetToolDirectory(System::NebulaSettings::ReadString("gscept", "ToolkitShared", "path"));
        }
#endif
        this->coreServer->SetRootDirectory(root);
        this->coreServer->Open();

        // настроить сервер игрового контента
        this->gameContentServer = GameContentServer::Create();
        this->gameContentServer->SetTitle(this->GetAppTitle());
        this->gameContentServer->SetTitleId(this->GetAppID());
        this->gameContentServer->SetVersion(this->GetAppVersion());
        this->gameContentServer->Setup();

        // настройка подсистемы ввода-вывода
        this->ioServer = IoServer::Create();
        this->ioServer->MountStandardArchives();
        this->ioInterface = IoInterface::Create();
        this->ioInterface->Open();

        Flat::FlatbufferInterface::Init();

        Options::InitOptions();

        Jobs2::JobSystemInitInfo jobSystemInfo;
        jobSystemInfo.numThreads = System::NumCpuCores;
        jobSystemInfo.name = "JobSystem";
        jobSystemInfo.scratchMemorySize = 16_MB;
        Jobs2::JobSystemInit(jobSystemInfo);

        this->resourceServer = Resources::ResourceServer::Create();
        this->resourceServer->Open();


#if NEBULA_ENABLE_PROFILING
        Profiling::ProfilingRegisterThread();
#endif

        // прикрепите обработчик консоли файла журнала
#if __WIN32__
        Ptr<LogFileConsoleHandler> logFileHandler = LogFileConsoleHandler::Create();
        Console::Instance()->AttachHandler(logFileHandler.upcast<ConsoleHandler>());
#endif

#if __NEBULA_HTTP__
        // настроить http-подсистему
        this->httpInterface = Http::HttpInterface::Create();
        this->httpInterface->SetTcpPort(this->defaultTcpPort);
        this->httpInterface->Open();
        this->httpServerProxy = Http::HttpServerProxy::Create();
        this->httpServerProxy->Open();
        this->httpServerProxy->AttachRequestHandler(Debug::CorePageHandler::Create());
        this->httpServerProxy->AttachRequestHandler(Debug::ThreadPageHandler::Create());
        this->httpServerProxy->AttachRequestHandler(Debug::MemoryPageHandler::Create());
        this->httpServerProxy->AttachRequestHandler(Debug::ConsolePageHandler::Create());
        this->httpServerProxy->AttachRequestHandler(Debug::IoPageHandler::Create());
        //this->httpServerProxy->AttachRequestHandler(Debug::GamePageHandler::Create());

        // настроить подсистему отладки
        this->debugInterface = DebugInterface::Create();
        this->debugInterface->Open();
#endif

        // создаем наш игровой сервер и открываем его
        this->gameServer = Game::GameServer::Create();
        this->gameServer->SetCmdLineArgs(this->GetCmdLineArgs());

        // всегда добавляйте функцию базовой игры
        this->baseGameFeature = BaseGameFeature::BaseGameFeatureUnit::Create();
        this->gameServer->AttachGameFeature(this->baseGameFeature);

        // создавать и добавлять новые игровые функции
        this->SetupGameFeatures();
        // открыть игровой сервер
        this->gameServer->Open();
        // начать игру
        this->gameServer->Start();
        // настроить профилирование
        _setup_grouped_timer(GameApplicationFrameTimeAll, "Игровая подсистема");


        return true;
    }
    return false;
}

//------------------------------------------------------------------------------
/**
*/
void
GameApplication::Close()
{
    n_assert(this->IsOpen());

    //_discard_timer (GameApplicationFrameTimeAll);

    // выключение базовой среды выполнения Nebula
    this->gameServer->Stop();
    this->gameServer->CleanupWorld(Game::GetWorld(WORLD_DEFAULT));

    this->gameServer->Close();

    this->CleanupGameFeatures();
    this->gameServer->RemoveGameFeature(this->baseGameFeature);
    this->baseGameFeature = nullptr;

    this->gameServer = nullptr;

    this->gameContentServer->Discard();
    this->gameContentServer = nullptr;

    this->resourceServer->Close();
    this->resourceServer = nullptr;

#if __NEBULA_HTTP__
    this->debugInterface->Close();
    this->debugInterface = nullptr;

    this->httpServerProxy->Close();
    this->httpServerProxy = nullptr;
    this->httpInterface->Close();
    this->httpInterface = nullptr;
#endif

    this->ioInterface->Close();
    this->ioInterface = nullptr;
    this->ioServer = nullptr;

    this->coreServer->Close();
    this->coreServer = nullptr;

    Jobs2::JobSystemUninit();

    Application::Close();
}




//------------------------------------------------------------------------------
/**
    Запустите приложение. Этот метод вернется, когда приложение пожелает
    выйти.
*/
void
GameApplication::Run()
{
    Input::InputServer* inputServer = Input::InputServer::Instance();
    while (!inputServer->IsQuitRequested())
    {
        this->StepFrame();
    }
}

//------------------------------------------------------------------------------
/**
*/
void
GameApplication::StepFrame()
{
    _start_timer(GameApplicationFrameTimeAll);

#if __NEBULA_HTTP__
    this->httpServerProxy->HandlePendingRequests();
#endif

#if NEBULA_ENABLE_PROFILING
    Profiling::ProfilingNewFrame();
#endif

    Jobs2::JobNewFrame();

    // триггер главного сервера
    this->coreServer->Trigger();

    // обновить ресурсы
    this->resourceServer->Update(GameApplication::FrameIndex);

    // запускать начало кадра для функциональных блоков
    this->gameServer->OnBeginFrame();

    // триггерный кадр для функциональных блоков
    this->gameServer->OnFrame();

    // вызовите метод Run() приложения
    Application::Run();

    // триггер конца кадра для функциональных блоков
    this->gameServer->OnEndFrame();

    GameApplication::FrameIndex++;

    _stop_timer(GameApplicationFrameTimeAll);
}

//------------------------------------------------------------------------------
/**
    Setup new game features which should be used by this application.
    Overwride for all features which have to be used.

    Make sure that features are setup ONLY in this method, since other
    systems might not expect otherwise.
*/
void
GameApplication::SetupGameFeatures()
{
    // создавать любые функции в производном классе
}

//------------------------------------------------------------------------------
/**
    Очистка всех добавленных функций игры.
*/
void
GameApplication::CleanupGameFeatures()
{
    // очистите свои функции в производном классе
}

//------------------------------------------------------------------------------
/**
*/
void
GameApplication::SetupAppFromCmdLineArgs()
{
    // разрешить переименование приложения
    const Util::CommandLineArgs& args = this->GetCmdLineArgs();
    if (args.HasArg("-appname"))
    {
        this->SetAppTitle(args.GetString("-appname"));
    }
    editorEnabled = args.GetBoolFlag("-editor");
}

} // Приложение пространства имен
