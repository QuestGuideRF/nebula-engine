//------------------------------------------------------------------------------
//  levelviewer.cc
//  (C) 2013–2016 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "foundation/stdneb.h"
#include "levelviewerapplication.h"

ImplementNebulaApplication()

using namespace Tools;
using namespace Util;

//------------------------------------------------------------------------------
/**
*/
void
NebulaMain(const CommandLineArgs& args)
{
    LevelViewerGameStateApplication app;
    app.SetCompanyName("LTU - Технологический университет Лулео");
    app.SetAppTitle("Просмотр уровня");    
    app.SetCmdLineArgs(args);
    if (app.Open())
    {
        app.Run();
        app.Close();
    }
    app.Exit();
}