//------------------------------------------------------------------------------
//  texturebatcher3.cc
//  (C) 2008 г. Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "foundation/stdneb.h"
#include "texturebatcherapp.h"

//------------------------------------------------------------------------------
/**
*/
int __cdecl
main(int argc, const char** argv)
{    
    Util::CommandLineArgs args(argc, argv);
    Toolkit::TextureBatcherApp app;
    app.SetCompanyName("gscept");
    app.SetAppTitle("Пакетный конвертер текстур NebulaT");
    app.SetCmdLineArgs(args);
    if (app.Open())
    {
        app.Run();
        app.Close();
    }
    app.Exit();
}
