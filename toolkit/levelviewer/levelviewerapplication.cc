//------------------------------------------------------------------------------
//  levelviewerapplication.cc
//  (C) 2013–2016 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "foundation/stdneb.h"
#include "levelviewerapplication.h"
#include "graphics/graphicsprotocol.h"
#include "messaging/message.h"
#include "input/keyboard.h"
#include "input/gamepad.h"
#include "managers/factorymanager.h"
#include "managers/entitymanager.h"
#include "gamestates/viewergamestate.h"
#include "scriptingfeature/scriptingfeature.h"
#include "physicsfeature/physicsprotocol.h"
#include "graphicsfeature/graphicsfeatureprotocol.h"
#include "basegamefeature/basegameprotocol.h"
#include "scriptingfeature/scriptingcommands.h"
#include "gamestates/reloadstate.h"
#include "effects/effectscommands.h"
#include "faudio/audioprotocol.h"
#include "ui/uicommands.h"
#include "levelviewer/NIDL/levelviewercommands.h"
#include "posteffect/posteffectprotocol.h"


namespace Tools
{

using namespace QtRemoteInterfaceAddon;
using namespace Graphics;
using namespace Math;
using namespace Input;
using namespace BaseGameFeature;
using namespace Game;

//------------------------------------------------------------------------------
/**
*/
LevelViewerGameStateApplication::LevelViewerGameStateApplication(void)
{
#if __NEBULA3_HTTP__
    this->defaultTcpPort = 2103;
#endif
}

//------------------------------------------------------------------------------
/**
*/
LevelViewerGameStateApplication::~LevelViewerGameStateApplication(void)
{
    if (this->IsOpen())
    {
        this->Close();
    }
}

//------------------------------------------------------------------------------
/**
*/
bool LevelViewerGameStateApplication::Open()
{
    n_assert(!this->IsOpen());
    if (GameApplication::Open())
    {
        return true;
    }
    return false;
}

//------------------------------------------------------------------------------
/**
*/
void LevelViewerGameStateApplication::Close()
{
    GameApplication::Close();
}


//------------------------------------------------------------------------------
/**
*/
void 
LevelViewerGameStateApplication::SetupStateHandlers()
{
    Ptr<LevelViewerGameState> gameState = LevelViewerGameState::Create();
    if (this->args.HasArg("-level"))
    {
        // перезаписать установленный уровень, используйте это
        gameState->SetLevelName(this->args.GetString("-level"));
        gameState->SetSetupMode(GameStateHandler::LoadLevel);
    }
    else
    {
        gameState->SetLevelName("Empty");
        gameState->SetSetupMode(GameStateHandler::NewGame);
    }   
    gameState->SetName("LevelViewerGameState");
    this->viewerState = gameState;

    Ptr<ReloadState> loadState = ReloadState::Create();
    loadState->SetSetupMode(GameStateHandler::EmptyWorld);
    loadState->SetName("Reload");
    this->AddStateHandler(loadState.get());
    

    this->AddStateHandler(gameState.get());

    this->SetState("LevelViewerGameState");


}

//------------------------------------------------------------------------------
/**
*/
void 
LevelViewerGameStateApplication::SetupGameFeatures()
{
    GameApplication::SetupGameFeatures();

    // настройка функции ввода
    this->inputFeature = InputFeature::InputFeatureUnit::Create();
    this->gameServer->AttachGameFeature(this->inputFeature.upcast<Game::FeatureUnit>());
    // настроить удаленный сервер
    this->remoteServer = QtRemoteServer::Create();
    this->remoteServer->SetPort(2104);
    this->remoteServer->Open();

    // настроить удаленный клиент
    this->remoteClient = QtRemoteClient::Create();
    this->remoteClient->SetPort(2103);

    // create and attach default graphic features
    this->graphicsFeature = GraphicsFeature::GraphicsFeatureUnit::Create();
    this->graphicsFeature->SetCmdLineArgs(this->GetCmdLineArgs());
    this->graphicsFeature->SetRenderDebug(true);
    this->graphicsFeature->SetupDisplay();

    // особенности игры
    this->baseGameFeature = BaseGameFeatureUnit::Create();
    Ptr<LevelViewer::LevelViewerFactoryManager> factory = LevelViewer::LevelViewerFactoryManager::Create();    
    this->baseGameFeature->SetFactoryManager(factory.cast<FactoryManager>());

    // создавать и прикреплять основные функции
    this->physicsFeature = PhysicsFeature::PhysicsFeatureUnit::Create(); 
    this->physicsFeature->SetRenderDebug(true);

    // создавать сценарии
    this->scriptingFeature = ScriptingFeature::ScriptingFeatureUnit::Create();

    // создать пользовательский интерфейс
    this->uiFeature = UI::UiFeatureUnit::Create();

    // создать эффект
    this->effectFeature = EffectsFeature::EffectsFeatureUnit::Create();

    // создать пост-эффект
    this->postEffectFeature = PostEffect::PostEffectFeatureUnit::Create();

    this->navigationFeature = Navigation::NavigationFeatureUnit::Create();

    // прикрепить функции  
    this->gameServer->AttachGameFeature(this->baseGameFeature.upcast<Game::FeatureUnit>());    
    this->gameServer->AttachGameFeature(this->effectFeature.cast<Game::FeatureUnit>());
    this->gameServer->AttachGameFeature(this->graphicsFeature.cast<Game::FeatureUnit>());
    this->gameServer->AttachGameFeature(this->scriptingFeature.upcast<Game::FeatureUnit>());
    this->gameServer->AttachGameFeature(this->physicsFeature.upcast<Game::FeatureUnit>());        
    this->gameServer->AttachGameFeature(this->navigationFeature.cast<Game::FeatureUnit>());
    
    // настроить промежуточный графический интерфейс
    this->imgui = Dynui::ImguiAddon::Create();
    this->imgui->Setup();   

    Commands::ScriptingCommands::Register();
    Commands::PhysicsProtocol::Register();
    Commands::GraphicsFeatureProtocol::Register();
    Commands::BaseGameProtocol::Register();
    Commands::EffectsCommands::Register();
    Commands::AudioCommands::Register();
    Commands::UICommands::Register();   
    Commands::LevelviewerCommands::Register();
    Commands::PostEffectCommands::Register();

    this->gameServer->AttachGameFeature(this->uiFeature.cast<Game::FeatureUnit>());
    this->gameServer->AttachGameFeature(this->postEffectFeature.cast<Game::FeatureUnit>());

    // создать консоль
    this->console = Dynui::ImguiConsole::Create();
    this->console->Setup();
    this->consoleHandler = Dynui::ImguiConsoleHandler::Create();
    this->consoleHandler->Setup();

    this->uiFeature->LoadAllFonts("bin:../../data/");
    if (IO::IoServer::Instance()->FileExists("bin:../../data/levelviewer/levellist.rml"))
    {
        this->uiFeature->CreateLayout("_levellist", "bin:../../data/levelviewer/levellist.rml");
        this->uiFeature->CreateLayout("_layoutlist", "bin:../../data/levelviewer/layoutlist.rml");
    }   
    Util::String script = "bin:../../data/levelviewer/levellist.lua";
    if (IO::IoServer::Instance()->FileExists(script))
    {
        Scripting::ScriptServer::Instance()->EvalScript(script);
        if (Scripting::ScriptServer::Instance()->HasError())
        {
            n_warning("Ошибка при оценке сценария просмотра уровней:\n%s\n", Scripting::ScriptServer::Instance()->GetError().AsCharPtr());
        }
    }
    else
    {
        n_warning(("Не удалось загрузить файл сценария: " + script).AsCharPtr());
    }

}

//------------------------------------------------------------------------------
/**
*/
void 
LevelViewerGameStateApplication::CleanupGameFeatures()
{
    this->remoteServer->Close();
    this->remoteServer = 0;

    // закройте удаленный клиент, если необходимо
    if (this->remoteClient->IsOpen())
    {
        this->remoteClient->Close();
    }

    this->consoleHandler->Discard();
    this->consoleHandler = 0;
    this->console->Discard();
    this->console = 0;

    // закрыть промежуточный графический интерфейс
    this->imgui->Discard();
    this->imgui = 0;

    this->remoteClient = 0;
    this->gameServer->RemoveGameFeature(this->navigationFeature.upcast<Game::FeatureUnit>());
    this->navigationFeature = 0;
    this->gameServer->RemoveGameFeature(this->postEffectFeature.upcast<Game::FeatureUnit>());
    this->postEffectFeature = 0;
    this->gameServer->RemoveGameFeature(this->uiFeature.upcast<Game::FeatureUnit>());
    this->uiFeature = 0;
    this->gameServer->RemoveGameFeature(this->scriptingFeature.upcast<Game::FeatureUnit>());
    this->scriptingFeature = 0;
    this->gameServer->RemoveGameFeature(this->effectFeature.upcast<Game::FeatureUnit>());
    this->effectFeature = 0;
    this->gameServer->RemoveGameFeature(this->physicsFeature.upcast<Game::FeatureUnit>());
    this->physicsFeature = 0;
    this->gameServer->RemoveGameFeature(this->graphicsFeature.upcast<Game::FeatureUnit>());
    this->graphicsFeature = 0;
    this->gameServer->RemoveGameFeature(this->baseGameFeature.upcast<Game::FeatureUnit>());
    this->baseGameFeature = 0;
    this->gameServer->RemoveGameFeature(this->inputFeature.upcast<Game::FeatureUnit>());
    this->inputFeature = 0;

    GameApplication::CleanupGameFeatures();
}

//------------------------------------------------------------------------------
/**
*/
Ptr<Tools::LevelViewerGameState> &
LevelViewerGameStateApplication::GetViewerState()
{
    return this->viewerState;
}

}
