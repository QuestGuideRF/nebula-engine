//------------------------------------------------------------------------------
//  main.cc
//  (C) 2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "foundation/stdneb.h"
#include "texgenapp.h"

//------------------------------------------------------------------------------
/**
*/
int __cdecl
main(int argc, const char** argv)
{    
    Util::CommandLineArgs args(argc, argv);
    Toolkit::TexGenApp app;
    app.SetCompanyName("gscept");
    app.SetAppTitle("Генератор текстур туманности");
    app.SetAppVersion("0.1.1");
    app.SetCmdLineArgs(args);
    
    if (app.Open())
    {
        if (app.ParseCmdLineArgs())
        {
            app.Run();
        }
        app.Close();
    }
    app.Exit();
    return app.GetReturnCode();
}
