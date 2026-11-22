//------------------------------------------------------------------------------
//  audiobatcherapp.cc
//  (C) 2008 г. Радон Лабс ГмбХ
//  (C) 2013–2016 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "foundation/stdneb.h"
#include "audiobatcherapp.h"

namespace Toolkit
{

//------------------------------------------------------------------------------
/**
*/
bool
AudioBatcherApp::ParseCmdLineArgs()
{
    if (ToolkitApp::ParseCmdLineArgs())
    {
        this->audioExporter.SetForceFlag(this->args.GetBoolFlag("-force"));
        return true;
    }
    return false;
}

//------------------------------------------------------------------------------
/**
*/
bool
AudioBatcherApp::SetupProjectInfo()
{
    if (ToolkitApp::SetupProjectInfo())
    {
        this->audioExporter.SetPlatform(this->platform);
        if (this->projectInfo.HasAttr("AudioTool"))
        {
            this->audioExporter.SetToolPath(this->projectInfo.GetPathAttr("AudioTool"));
        }
        if (this->projectInfo.HasAttr("AudioProjectFile"))
        {
            this->audioExporter.SetProjectFile(this->projectInfo.GetAttr("AudioProjectFile"));
        }
        if (this->projectInfo.HasAttr("AudioSrcDir"))
        {
            this->audioExporter.SetSrcDir(this->projectInfo.GetAttr("AudioSrcDir"));
        }
        this->audioExporter.SetDstDir(this->projectInfo.GetAttr("AudioDstDir"));
        return true;
    }
    return false;
}

//------------------------------------------------------------------------------
/**
*/
void
AudioBatcherApp::ShowHelp()
{
    n_printf("Пакетный компилятор аудио NebulaT.\n"
             "(C) Отдельные авторы, см. файл АВТОРЫ.\n"
             "-help -- отобразить эту справку\n"
             "-platform -- выбор платформы (win32, xbox360, wii, ps3)\n"
             "-fmod -- использовать fmod в качестве аудио-движка\n"
             "-waitforkey -- дождаться ключа после завершения\n"
             "-force -- принудительно перестроить\n");
}

//------------------------------------------------------------------------------
/**
*/
void
AudioBatcherApp::Run()
{
    // анализировать аргументы командной строки
    if (!this->ParseCmdLineArgs())
    {
        return;
    }

    // настроить объект информации о проекте
    if (!this->SetupProjectInfo())
    {
        return;
    }

    // выполнить процесс экспорта
    if (!this->audioExporter.Export())
    {
        this->SetReturnCode(-1);
        return;
    }
}

} // Инструментарий пространства имен
