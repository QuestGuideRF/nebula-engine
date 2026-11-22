//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
#include "foundation/stdneb.h"
#include "modelbatcher3app.h"

//------------------------------------------------------------------------------
/**
*/
int __cdecl
main(int argc, const char** argv)
{
        Util::CommandLineArgs args(argc, argv);
        Toolkit::ModelBuilder3App app;
        app.SetCompanyName("gscept");
        app.SetAppTitle("Конструктор моделей N3");
        app.SetCmdLineArgs(args);
        if (app.Open())
        {
                app.Run();
                app.Close();
        }
        app.Exit();
}
