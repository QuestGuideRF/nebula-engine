//------------------------------------------------------------------------------
#include "foundation/stdneb.h"
#include "assetbatcherapp.h"

//------------------------------------------------------------------------------
/**
*/
int __cdecl
main(int argc, const char** argv)
{
    Util::CommandLineArgs args(argc, argv);
    Toolkit::AssetBatcherApp app;
    app.SetCompanyName("gscept");
    app.SetAppTitle("Дозатор активов NebulaT");
    app.SetCmdLineArgs(args);
    if (app.Open())
    {
        app.Run();
        app.Close();
    }
    app.Exit();
}
