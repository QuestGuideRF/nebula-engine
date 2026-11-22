//------------------------------------------------------------------------------
#include "foundation/stdneb.h"
#include "assetconverterapp.h"

//------------------------------------------------------------------------------
/**
*/
int __cdecl
main(int argc, const char** argv)
{
    Util::CommandLineArgs args(argc, argv);
    Toolkit::AssetConverterApp app;
    app.SetCompanyName("gscept");
    app.SetAppTitle("Конвертер облачных активов");
    app.SetCmdLineArgs(args);
    if (app.Open())
    {
        app.Run();
        app.Close();
    }
    app.Exit();
}
