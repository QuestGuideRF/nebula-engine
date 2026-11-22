//------------------------------------------------------------------------------
//  shaderc.cc
//  (C) 2008 г. Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "foundation/stdneb.h"
#include "shadercompilerapp.h"

//------------------------------------------------------------------------------
/**
*/
int __cdecl
main(int argc, const char** argv)
{
    Util::CommandLineArgs args(argc, argv);
    Toolkit::ShaderCompilerApp app;
    app.SetCompanyName("gscept");
    app.SetAppTitle("Шейдер туманности/рамка/компилятор материалов");
    app.SetCmdLineArgs(args);
    if (app.Open())
    {
        app.Run();
        app.Close();
    }
    app.Exit();
}
