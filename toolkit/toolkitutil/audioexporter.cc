//------------------------------------------------------------------------------
//  audioexporter.cc
//  (C) 2008 г. Радон Лабс ГмбХ
//  (C) 2013–2016 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "foundation/stdneb.h"
#include "audioexporter.h"
#include "io/assignregistry.h"
#include "io/ioserver.h"
#include "io/uri.h"
#include "toolkit-common/applauncher.h"
#include "io/memorystream.h"

namespace ToolkitUtil
{
using namespace Util;
using namespace IO;

//------------------------------------------------------------------------------
/**
*/
AudioExporter::AudioExporter() :
    platform(Platform::Win32)
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
AudioExporter::~AudioExporter()
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
bool
AudioExporter::Export()
{
    if ((this->platform == Platform::Win32) || (this->platform == Platform::Linux))
    {
        return this->ExportFmod();
    }    
    else
    {
        n_printf("ВНИМАНИЕ: в AudioExporter не поддерживается платформа «%s»!\n", Platform::ToString(this->platform).AsCharPtr());
        return true;
    }
}

//------------------------------------------------------------------------------
/**
    Функция экспорта аудио для платформ Windows и Xbox360.
*/
bool
AudioExporter::ExportXact()
{
    n_assert(this->toolPath.IsValid());
    n_assert(this->projectFile.IsValid());      
    n_assert(this->dstDir.IsValid());
    IoServer* ioServer = IoServer::Instance();

    // разрешить назначения в пути к файлу проекта
    String resolvedProjFilePath = AssignRegistry::Instance()->ResolveAssignsInString(this->projectFile);
    if (!ioServer->FileExists(resolvedProjFilePath))
    {
        n_printf("ВНИМАНИЕ: файл проекта XACT «%s» не существует!\n", resolvedProjFilePath.AsCharPtr());
        return false;
    }
    
    // разрешить назначения в каталоге назначения и убедиться, что путь существует
    String resolvedDstPath = AssignRegistry::Instance()->ResolveAssignsInString(this->dstDir);
    ioServer->CreateDirectory(this->dstDir);

    // настройте аргументы командной строки и запустите xactbld3.exe
    String args = "/L /X:ЗАГОЛОВОК /X:СПИСОК ВЫЗОВ /X:ОТЧЕТ ";
    if (this->force)
    {
        args.Append("/F ");
    }
    if (Platform::Win32 == this->platform)
    {
        args.Append("/WIN32 ");
    }
    else
    {
        args.Append("/XBOX360 ");
    }
    args.Append("\"");
    args.Append(resolvedProjFilePath);
    args.Append("\" \"");
    args.Append(resolvedDstPath);
    args.Append("\"");

    AppLauncher appLauncher;
    appLauncher.SetExecutable(this->toolPath);
    appLauncher.SetWorkingDirectory(resolvedDstPath);
    appLauncher.SetArguments(args);
    if (!appLauncher.LaunchWait())
    {
        n_printf("ВНИМАНИЕ: не удалось запустить аудиоинструмент «%s»!\n", this->toolPath.AsCharPtr());
        return false;
    }
    return true;
}

//------------------------------------------------------------------------------
/**
    Функция экспорта аудио для платформы Wii.
*/
bool
AudioExporter::ExportWii()
{
    n_assert(this->toolPath.IsValid());
    n_assert(this->srcDir.IsValid());
    n_assert(this->dstDir.IsValid());

    IoServer* ioServer = IoServer::Instance();

    // найдите все файлы проекта SoundMaker в папке audio
    this->FindRspjFiles(this->srcDir);
    
    // конвертировать все файлы проекта SoundMaker (".rspj") в SoundArchives (".brsar")
    AppLauncher convAppLauncher;
    convAppLauncher.SetExecutable(this->toolPath);
    convAppLauncher.SetWorkingDirectory("proj:work");
    IndexT i;
    for (i = 0; i < this->rspjFiles.Size(); i++)
    {
        convAppLauncher.SetArguments(AssignRegistry::Instance()->ResolveAssignsInString(this->rspjFiles[i]));
        if (!convAppLauncher.LaunchWait())
        {
            n_error("Не удалось запустить «%s %s»!\n", this->toolPath.AsCharPtr(), this->rspjFiles[i].AsCharPtr());
            return false;
        }
    }

    // скопировать преобразованные файлы в место назначения
    String localDstPath = AssignRegistry::Instance()->ResolveAssignsInString(this->dstDir);
    for (i = 0; i < this->rspjFiles.Size(); i++)
    {
        // Error 500 (Server Error)!!1500.That’s an error.There was an error. Please try again later.That’s all we know.
        String destFolder = this->rspjFiles[i];
        destFolder.SubstituteString("src:audio", "");
        destFolder.TerminateAtIndex(destFolder.FindStringIndex("/rspj"));
        destFolder = localDstPath + destFolder;
        if (!ioServer->DirectoryExists(destFolder))
        {
            ioServer->CreateDirectory(destFolder);
        }

        // копировать файлы из исходной папки в папку назначения
        String srcFolder = AssignRegistry::Instance()->ResolveAssignsInString(this->srcFolders[i]);
        this->CopyDirectory(srcFolder, destFolder);
    }
    return true;
}

//------------------------------------------------------------------------------
/**
*/
void
AudioExporter::FindRspjFiles(String folder)
{
    IoServer* ioServer = IoServer::Instance();

    // искать файлы RSPJ
    Array<String> rspjFiles = ioServer->ListFiles(folder, "*.rspj");
    IndexT i;
    for (i = 0; i < rspjFiles.Size(); i++)
    {
        this->rspjFiles.Append((folder + "/" + rspjFiles[i]));
        this->srcFolders.Append(folder + "/" + "output/dvddata");
    }

    // пройтись по всем подпапкам
    Array<String> subfolders = ioServer->ListDirectories(folder, "*");
    for (i = 0; i < subfolders.Size(); i++)
    {
        if ((subfolders[i] != "CVS") && (subfolders[i] != ".svn"))
        {
            this->FindRspjFiles((folder + "/" + subfolders[i]));
        }
    }
}

//------------------------------------------------------------------------------
/**
*/
void
AudioExporter::CopyDirectory(Util::String src, Util::String dst)
{   
    IoServer* ioServer = IoServer::Instance();

    // скопируйте все файлы в каталог
    Array<String> files = ioServer->ListFiles(src, "*");
    IndexT i;
    for (i = 0; i < files.Size(); i++)
    {
        ioServer->CopyFile(src + "/" + files[i], dst + "/" + files[i]);
    }

    // скопировать все подкаталоги
    Array<String> directories = ioServer->ListDirectories(src, "*");
    for (i = 0; i < directories.Size(); i++)
    {
        ioServer->CreateDirectory(dst + "/" + directories[i]);
        this->CopyDirectory(src + "/" + directories[i], dst + "/" + directories[i]);
    }
}

//------------------------------------------------------------------------------
/**
*/
void
AudioExporter::FmodFindProjectFiles(const Util::String &folder, Util::Array<Util::String> &projectFiles, const Util::String & pattern) const
{
    IoServer* ioServer = IoServer::Instance();
    projectFiles.Clear();
    Array<String> directories = ioServer->ListDirectories(folder, "*");
    for (IndexT j = 0; j < directories.Size(); j++)
    {
        // найдите файлы проекта fmod в папке
        Util::Array<Util::String> projs = ioServer->ListFiles(folder + "/" + directories[j], pattern);
        IndexT i;
        for (i = 0; i < projs.Size(); i++)
        {
            projectFiles.Append(folder + "/" + directories[j] + "/" + projs[i]);
        }
    }   
}


//------------------------------------------------------------------------------
/**
Функция экспорта аудио для студии fmod.
*/
bool
AudioExporter::ExportFmod()
{
    n_assert(this->toolPath.IsValid());
    n_assert(this->srcDir.IsValid());
    n_assert(this->dstDir.IsValid());
    IoServer* ioServer = IoServer::Instance();


    // 1. создать каталог экспорта аудио
    // разрешить назначения в каталоге назначения и убедиться, что путь существует
    const String resolvedDstPath = AssignRegistry::Instance()->ResolveAssignsInString(this->dstDir);
    const String resolvedSrcPath = AssignRegistry::Instance()->ResolveAssignsInString(this->srcDir);
    ioServer->CreateDirectory(this->dstDir);

    // 2. собрать все файлы *.fspro (файл проекта студии fmod)
    Util::Array<Util::String> projects;
    this->FmodFindProjectFiles(resolvedSrcPath, projects, "*.fspro");
    if (projects.Size() == 0)
    {
        n_printf("аудиопроекты не найдены\n");
        return false;
    }
    n_assert2(projects.Size() == 1, "Обнаружено несколько файлов fsproj, в настоящее время поддерживается только один");

    Util::String baseArgs = "-build ";
    switch (this->platform)
    {
    case Platform::Win32:
    case Platform::Linux:       
        break;
    default:
        n_error("платформа не поддерживается\n");
        break;
    }
    
    
    String projectName;
                
    String args = baseArgs + "\"" + projects[0] + "\"";

    // поскольку у студии fmod нет полезного способа использования своего инструмента командной строки, нам придется поколдовать здесь
    Array<String> paths = ioServer->ListDirectories(this->toolPath, "ФМОД Студия 1*");
    n_assert2(paths.Size() > 0, "Каталог приложения FMOD Studio не найден.");

    String executable = this->toolPath + "/" + paths[paths.Size() - 1] + "/fmodstudiocl.exe";
    AppLauncher appLauncher;
    appLauncher.SetExecutable(executable);
    appLauncher.SetWorkingDirectory(resolvedDstPath);
    appLauncher.SetArguments(args);
    Ptr<IO::MemoryStream> ss = IO::MemoryStream::Create();
    appLauncher.SetStderrCaptureStream(ss.cast<IO::Stream>());

    if (!appLauncher.LaunchWait())
    {
        n_printf("ВНИМАНИЕ: не удалось запустить аудиоинструмент «%s»!\n", executable.AsCharPtr());
        return false;
    }
    
    // скопировать файлы сборки в выходную папку
    Array<String> files = ioServer->ListFiles(projects[0].ExtractDirName() + "/Build/Desktop/","*.банк");

    for (int i = 0; i < files.Size(); i++)
    {
        ioServer->CopyFile(projects[0].ExtractDirName() + "/Build/Desktop/" + files[i], resolvedDstPath + "/" + files[i]);
    }

    //////////////////////////////////////
    // FIXME: удалить файлы, которые нам больше не нужны (сборка .cache)  
    return true;
}

} // пространство имен ToolkitUtil
