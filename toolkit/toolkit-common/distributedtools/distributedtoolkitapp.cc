//------------------------------------------------------------------------------
//  distributedtoolkitapp.cc
//  (C) 2009 г. Радон Лабс ГмбХ
//  (C) 2013–2016 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "foundation/stdneb.h"
#include "distributedtools/distributedtoolkitapp.h"
#include "distributedtools/distributedjobs/distributedappjob.h"
#include "distributedtools/distributedjobscheduler.h"
#include "distributedtools/remotejobservice.h"
#include "io/assignregistry.h"
#include "io/ioserver.h"
#include "io/textreader.h"
#include "io/uri.h"
#include "io/memorystream.h"
#include "timing/timer.h"
#include "system/nebulasettings.h"

using namespace Util;
using namespace IO;
using namespace System;
//------------------------------------------------------------------------------
namespace DistributedTools
{
//------------------------------------------------------------------------------
/**
    Конструктор 
*/
DistributedToolkitApp::DistributedToolkitApp() :
    forceArg(false),
    slaveArg(false),
    masterMode(false),
    jobsArg(1),
    processorsArg(1)   
{
    // empty
}

//------------------------------------------------------------------------------
/**
    Деструктор  
*/
DistributedToolkitApp::~DistributedToolkitApp()
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
bool
DistributedToolkitApp::Open()
{
    if (!ToolkitApp::Open())
    {
        return false;
    }
    this->sharedDirCreator = SharedDirCreator::Create();
    return true;
}

//------------------------------------------------------------------------------
/**
*/
void
DistributedToolkitApp::Close()
{
    // удалить все содержимое каталога назначения, если
    // приложение находилось в подчиненном режиме на удаленном компьютере
    if (this->distributedArg && this->slaveArg)
    {
        this->DeleteDirectoryContent(URI("dst:"));
        IoServer::Instance()->DeleteDirectory(URI("dst:"));
    }
    // удалить создателя общего каталога
    this->sharedDirCreator = nullptr;
    // if shared dir control is valid , dispose it
    if(this->sharedDirControl.isvalid())
    {
        this->sharedDirControl = nullptr;
    }
    ToolkitApp::Close();
}

//------------------------------------------------------------------------------
/**
    Разберите аргументы командной строки и настройте параметры приложения.
*/
bool
DistributedToolkitApp::ParseCmdLineArgs()
{
    if(!ToolkitApp::ParseCmdLineArgs())
    {
        return false;
    }      
    
    // настроить объект информации о проекте
    if (!this->SetupProjectInfo())
    {
        return false;
    }
    this->sharedDirCreator->SetProjectInfo(this->projectInfo);

    if(this->args.HasArg("-jobs"))
    {
        // if a jobs arg was given, this class switch to master mode
        this->masterMode = true;
        this->jobsArg = this->args.GetInt("-jobs",1);
    }
    this->processorsArg  = this->args.GetInt("-processors",1);
    this->projectdirArg  = this->args.GetString("-projectdir",URI("proj:").LocalPath());
    this->dirArg         = this->args.GetString("-dir","");
    this->fileArg        = this->args.GetString("-file","");
    this->listfileArg    = this->args.GetString("-listfile","");
    this->distributedArg = this->args.GetBoolFlag("-distributed");
    this->shareddirArg   = this->args.GetString("-shareddir","");
    this->slaveArg       = this->args.GetBoolFlag("-slave");
    this->forceArg       = this->args.GetBoolFlag("-force");
    this->verboseFlag    = this->args.GetBoolFlag("-verbose");

    // if a list arg is given, the dir and file args are invalid
    if(
        this->listfileArg.IsValid() &&
        (this->dirArg.IsValid() || this->fileArg.IsValid()))
    {
        n_printf("если задан файл списка, каталог или аргумент файла не разрешены\n\n");
        this->ShowHelp();
        return false;
    }
    // if a file arg is given, a dir arg have to exist
    if(this->fileArg.IsValid() && !this->dirArg.IsValid())
    {
        n_printf("отсутствует аргумент каталога\n\n");
        this->ShowHelp();
        return false;
    }
    // if the distributed arg is set, we need a shared directory
    if(this->args.HasArg("-distributed") && !this->args.HasArg("-shareddir"))
    {
        n_printf("отсутствует аргумент общего каталога\n\n");
        this->ShowHelp();
        return false;
    }
    // if the slave and distributed arg is set, we need also a shared guid
    if(
        this->args.HasArg("-slave") &&
        this->args.HasArg("-distributed") &&
        !this->args.HasArg("-sharedguid"))
    {
        n_printf("отсутствует аргумент общего руководства\n\n");
        this->ShowHelp();
        return false;
    }
    // Аргумент подчиненного устройства никогда не следует использовать вместе с аргументом задания.
    if(this->args.HasArg("-slave") && this->args.HasArg("-jobs"))
    {
        n_printf("для подчиненного устройства не разрешен аргумент jobs\n\n");
        this->ShowHelp();
        return false;
    }

    if(
        this->args.HasArg("-jobs") &&
        this->args.HasArg("-distributed") &&
        !this->args.HasArg("-slave"))
    {
        // создать общийDirControl для мастера
        Guid guid;
        guid.Generate();
        this->sharedDirControl =
            this->sharedDirCreator->CreateSharedControlObject(this->shareddirArg, guid);
    }
    if(
        this->slaveArg &&
        this->distributedArg &&
        this->args.HasArg("-sharedguid")
        )
    {
        Guid guid = Guid::FromString(this->args.GetString("-sharedguid",""));
        // создайте общийDirControl для подчиненного устройства. Общий каталог должен иметь дополнительный
        // каталог, названный в честь того же идентификатора из аргументаsharedguid
        this->sharedDirControl =
            this->sharedDirCreator->CreateSharedControlObject(this->shareddirArg, guid);
        // if that's not the case, the job should have failed way before
        n_assert(this->sharedDirControl->ContainsGuidSubDir(guid));
        this->sharedDirControl->SetUp();

        // переназначить каталог назначения, если он находится в подчиненном режиме
        n_assert(this->projectInfo.HasAttr("SlaveDstDir"));
        Guid subFolderGuid;
        subFolderGuid.Generate();
        URI slaveDst(this->projectInfo.GetPathAttr("SlaveDstDir"));
        slaveDst.AppendLocalPath(subFolderGuid.AsString());
        AssignRegistry::Instance()->SetAssign(Assign("dst",slaveDst.LocalPath()));
        
        // Создайте новый каталог назначения. Он очищается и удаляется при закрытии приложения.
        int tries = 0;
        while (!IoServer::Instance()->DirectoryExists(URI("dst:")))
        {
            IoServer::Instance()->CreateDirectory(URI("dst:"));
            if (!IoServer::Instance()->DirectoryExists(URI("dst:")))
            {
                tries++;
                if (tries > 3)
                {
                    // по крайней мере, нам нужен выходной каталог...
                    Console::Instance()->Error(
                        "Не удалось создать целевой каталог:\n%s\n",
                        URI("dst:").LocalPath().AsCharPtr());
                }
                else
                {
                    Console::Instance()->Warning(
                        "Не удалось создать целевой каталог:\n%s\nПовторите попытку через 3 секунды...\n",
                        URI("dst:").LocalPath().AsCharPtr());
                    n_sleep(3);
                }
            }
        }
    }
    return true;
}

//------------------------------------------------------------------------------
/**
    Получите строку, содержащую описание стандартных аргументов командной строки для
    распределенное приложение с набором инструментов
*/
String
DistributedToolkitApp::GetArgumentDescriptionString()
{
    String output;
    output.Format(
        "-help -- отобразить эту справку\n"
        "-waitforkey -- дождаться нажатия клавиши после завершения\n"
        "-projectdir -- абсолютный путь к каталогу проекта\n"
        "-dir -- путь к одному каталогу, который должен обрабатывать инструмент\n"
        "-file — имя отдельного файла, который инструмент должен обработать [требуется -dir]\n"
        "-listfile -- путь к текстовому файлу со строками файлов для обработки\n"
        "                (если указан аргумент -listfile, аргументы -dir и -file недействительны)\n"
        "-jobs -- сколько параллельных заданий следует использовать\n"
        "-processors -- количество локальных используемых процессоров\n"
        "-shareddir -- абсолютный путь к общему выходному каталогу\n"
        "                (должен быть общий сетевой ресурс)\n"
        "-distributed -- включить распределенный режим [требуется -shareddir]\n"
        "-slave — приложение будет работать в подчиненном режиме.\n"
        );
    return output;
}

//------------------------------------------------------------------------------
/**
    Run the application.
    - If the "-jobs" parameter was given to the application
      the master mode is enabled and this class will only split the task of the
      application in jobs. These jobs are added to a job scheduler.
    - Else the overwritten DoWork() method of the subclasses is called.
*/
void
DistributedToolkitApp::Run()
{
    if(this->ParseCmdLineArgs())
    {
        if(!this->slaveArg)
        {
            // этот метод выполнит код конкретного приложения
            this->OnBeforeRunLocal();
        }

        if(this->distributedArg)
        {
            if (this->masterMode)
            {
                // вещи, которые должен делать только мастер перед выполнением
                // (например, настройка общего каталога и т. д.)
                this->OnBeforeRunDistMaster();
            }
            if (this->slaveArg)
            {
                // вещи, которые должен делать каждый раб, если только нет распределения
                // например создание локального выходного каталога
                this->OnBeforeRunDistSlave();
            }
        }

        // фактическая обработка
        if (this->masterMode)
        {           
            // управлять распределением и обрабатывать вновь созданные рабочие места
            this->CreateAndProcessJobs();
        }
        else
        {
            // выполнить действие конкретного приложения
            this->DoWork();
        }

        if(this->distributedArg)
        {
            if (this->slaveArg)
            {
                // вещи, которые должен делать каждый раб, если только нет распределения
                // например копирование всех данных из локального выходного каталога в общий каталог
                this->OnAfterRunDistSlave();
            }
            if (this->masterMode)
            {
                // вещи, которые должен делать только мастер после выполнения
                // (например, копирование всех данных из общего каталога в экспортный)
                // и очистка общего каталога)
                this->OnAfterRunDistMaster();
            }
        }

        if(!this->slaveArg)
        {
            // этот метод выполнит код конкретного приложения
            this->OnAfterRunLocal();
            this->WaitForKey();
        }
    }
}

//------------------------------------------------------------------------------
/**
    if waitForKey flag is set, wait until console has input, else do nothing.
*/
void
DistributedToolkitApp::WaitForKey()
{
    if (this->waitForKey)
    {
        n_printf("Нажмите <Enter>, чтобы продолжить!\n");
        while (!Console::Instance()->HasInput())
        {
            Timing::Sleep(0.01);
        }
    }
}

//------------------------------------------------------------------------------
/**
    Analyze input parameters and create a list of files.
    Then split them into jobs. Add the jobs to a 
    job scheduler and wait until the scheduler has processed all jobs.
*/
void
DistributedToolkitApp::CreateAndProcessJobs()
{
    n_assert(this->projectdirArg.IsValid());
    
    // Создать строковый список всех файлов
    Array<String> fileList = this->CreateFileList();
        
#if 0
    n_printf("Получить текущую версию...\n");
    // Получить текущую версию svn каталога проекта
    String revision = this->GetCurrentRevision();
    if(!revision.IsValid())
    {
        n_printf("FAILED\n");
        this->SetReturnCode(10);
        return;
    }
    n_printf("ОК – версия %s\n",revision.AsCharPtr());
#endif
    // Создайте задания и разделите список файлов, чтобы каждое задание
    // получает почти такое же количество файлов
    Array<Ptr<DistributedAppJob>> jobs;
    Array<Array<String>> fileLists;
    IndexT jobIndex;
    SizeT jobCount = this->jobsArg;

    // ограничьте количество файлов на задание до maxJobCount, добавив больше заданий
    const int maxJobCount = 64;
    if(fileList.Size() / jobCount > maxJobCount)
    {
        Console::Instance()->Print("Достигнут максимальный лимит файлов (%i) на одно задание\nСоздание дополнительных заданий...",maxJobCount);
        jobCount = (SizeT)ceilf(((float)fileList.Size())/maxJobCount);
        Console::Instance()->Print("Сейчас используется %i заданий.\n\n",jobCount);
    }

    for (jobIndex = 0; jobIndex < jobCount; jobIndex++)
    {
        Ptr<DistributedAppJob> job = DistributedAppJob::Create();
        
        String exeName = this->args.GetCmdName().ExtractFileName();
#if __WIN32__
        // рассматривать дело, это заявление было
        // вызывается без расширения .exe
        if(exeName.GetFileExtension().Length() == 0)
        {
            exeName.Append(".exe");
        }
#endif
        
        String appPath;
        appPath.Append("bin:" + exeName);
        
        URI appUri(appPath);
        // исполняемый файл не существует по данному пути,
        // возможно, он находился не в каталоге bin/win32.
        // Попробуйте стандартный дом: в таком случае назначьте...
        if (!IoServer::Instance()->FileExists(appUri))
        {
            appUri.Set("home:");
            appUri.AppendLocalPath(exeName);
            if (!IoServer::Instance()->FileExists(appUri))
            {
                n_printf("Недопустимый путь приложения для заданий: %s\n",appUri.AsString().AsCharPtr());
                this->SetReturnCode(10);
                return;
            }
        }
        
        // Для совместимости с 64-битной машиной проверьте, находится ли путь к приложению внутри.
        // программы: назначить. В этом случае отправьте назначение вместо полного пути.
        String localAppPath = appUri.AsString();
        String programPath =  AssignRegistry::Instance()->ResolveAssignsInString("programs:");
        localAppPath.ToLower();
        programPath.ToLower();
        IndexT idx = localAppPath.FindStringIndex(programPath);
        if (0 == idx)
        {
            String subPath = localAppPath.ExtractToEnd(programPath.Length());
            subPath.TrimLeft("/\\");
            String remoteAppPath;
            remoteAppPath.Format("программы:%s",subPath.AsCharPtr());
            job->SetApplicationPath(remoteAppPath);
        }
        else
        {
            job->SetApplicationPath(localAppPath);
        }
        
        // Настройка аргументов командной строки для задания приложения
        String argString;
        argString.Append("-projectdir ");
        argString.Append(this->projectdirArg);
        // В распределенном режиме запущенное приложение имеет
        // чтобы знать, куда копировать результаты на подчиненной машине.
        if (this->distributedArg)
        {
            argString.Append(" -distributed ");
            String guidArg;
            guidArg.Format(" %s \"%s\"", "-sharedguid",
                this->sharedDirControl->GetGuid().AsString().AsCharPtr());
            argString.Append(guidArg);
            argString.Append(" -shareddir ");
            argString.Append(this->sharedDirControl->GetPath().AsString());
        }
        // Списки файлов являются абсолютными. В любом случае принудительный экспорт.
        argString.Append(this->forceArg ? " -force" : "");
        // Добавьте дополнительные аргументы, не связанные с распределенными инструментами.
        argString.Append(this->GetAdditionalArguments());
        
        job->SetArguments(argString);
        //job->SetSVNPath(this->projectInfo.GetPathAttr("SVNToolPath"));
        job->SetProjectDirectory(this->projectdirArg);
       // задание->SetRequiredRevision(ревизия);
        job->SetIdentifier(String::FromInt(jobIndex));
        job->SetDistributedFlag(this->distributedArg);
        if (this->distributedArg)
        {
            job->SetSharedDirGuid(this->sharedDirControl->GetGuid());
            job->SetSharedDirPath(this->sharedDirControl->GetPath().AsString());
        }
        jobs.Append(job);

        // подготовить список файлов для работы. Он заполняется позже.
        Array<String> list;
        fileLists.Append(list);
    }
    
    // Добавляйте файлы из списка к заданиям.
    IndexT fileIndex;
    for (fileIndex = 0; fileIndex < fileList.Size(); fileIndex++)
    {
        fileLists[fileIndex%fileLists.Size()].Append(fileList[fileIndex]);
    }

    // добавить задания в планировщик
    DistributedJobScheduler scheduler;
    if (scheduler.Open())
    {  
        // настройка планировщика для распределенной сборки
        scheduler.UseRemoteServices(this->distributedArg);
        scheduler.SetMaxParallelLocalJobs(this->processorsArg);
        if (this->projectInfo.HasAttr("BuildSlaves"))
        {
            String attr = this->projectInfo.GetAttr("BuildSlaves");
            Array<String> addresses = attr.Tokenize(";");
            IndexT a;
            for(a = 0; a < addresses.Size(); a++)
            {
                Array<String> addressParts = addresses[a].Tokenize(":");
                if(addressParts.Size()==2)
                {
                    Ptr<RemoteJobService> service = RemoteJobService::Create();
                    Net::IpAddress ipAddress;
                    ipAddress.SetHostName(addressParts[0]);
                    ipAddress.SetPort(addressParts[1].AsInt());
                    service->SetIpAddress(ipAddress);
                    service->SetVerboseFlag(this->verboseFlag);
                    scheduler.AttachRemoteJobService(service);
                }
            }
        }
        // прикрепить список файлов к каждому заданию и добавить задание в планировщик
        for (jobIndex = 0; jobIndex < jobCount; jobIndex++)
        {
            jobs[jobIndex]->SetFileList(fileLists[jobIndex]);
            scheduler.AttachJob(jobs[jobIndex].upcast<DistributedJob>());
        }
        
        // прикрепить задания инициализации/завершения к планировщику
        Array<Ptr<DistributedJob>> initJobs = this->GenerateInitializeJobList();
        IndexT j;
        for (j = 0; j < initJobs.Size(); j++)
        {
            scheduler.AppendInitializeJob(initJobs[j]);
        }
        Array<Ptr<DistributedJob>> finJobs = this->GenerateFinalizeJobList();
        for (j = 0; j < finJobs.Size(); j++)
        {
            scheduler.AppendFinalizeJob(finJobs[j]);
        }
        
        // запускать все задания, прикрепленные к планировщику
        scheduler.RunJobs();
        // подождите, пока планировщик завершит работу...
        while (scheduler.HasActiveJobs())
        {
            scheduler.Update();
            Timing::Sleep(1);
        }
        scheduler.Close();
    }
}

//------------------------------------------------------------------------------
/**
    Checks input arguments and build a string list of all files
    which should processed by the application. Override this in
    subclasses if you need another way of creation for this filelist.
*/
Array<String>
DistributedToolkitApp::CreateFileList()
{
    Array<String> arr;
    
    if (this->listfileArg.IsValid())
    {
        // прочитать строки файла и добавить их в список
        URI listUri;
        listUri.Set(this->listfileArg);
        
        Ptr<Stream> readStream = IoServer::Instance()->CreateStream(listUri);
        readStream->SetAccessMode(Stream::ReadAccess);
        readStream->Open();
        
        Ptr<TextReader> reader = TextReader::Create();
        reader->SetStream(readStream);
        reader->Open();
        while(!reader->Eof())
        {
            String line = reader->ReadLine();
            line.Trim(" \r\n");
            arr.Append(line);
        }
        reader->Close();
        reader = nullptr;

        readStream->Close();
        readStream = nullptr;
    }
    else if (this->dirArg.IsValid())
    {
        if(this->fileArg.IsValid())
        {
            // просто добавьте этот файл в список
            String str;
            str.Append(this->projectdirArg);
            str.Append("/");
            str.Append(this->dirArg);
            str.Append("/");
            str.Append(this->fileArg);
            arr.Append(str);
        }
        else
        {
            // добавить все файлы в указанном каталоге в список
            URI dirUri;
            dirUri.Set(this->projectdirArg);
            dirUri.AppendLocalPath(this->dirArg);
            
            Array<String> fileList = IoServer::Instance()->ListFiles(dirUri,"*.*");
            
            IndexT i;
            for (i=0;i<fileList.Size();i++)
            {
                String str;
                str.Append(this->projectdirArg);
                str.Append("/");
                str.Append(this->dirArg);
                str.Append("/");
                str.Append(fileList[i].ExtractFileName());
                arr.Append(str);
            }
        }
    }
    return arr;
}
//------------------------------------------------------------------------------
/**
    получает информацию svn текущего каталога проекта и возвращает идентификатор ревизии
*/
String
DistributedToolkitApp::GetCurrentRevision()
{
    String out;
    
    int tries = 0;
    do
    {
        out.Clear();

        ToolkitUtil::AppLauncher svnApp;

        String svnArgs;
        svnArgs.Append("info");
        svnApp.SetArguments(svnArgs);

        URI svnUri;
        svnUri.Set(this->projectInfo.GetPathAttr("SVNToolPath"));
        svnApp.SetExecutable(svnUri);

        URI svnWorkUri;    
        svnWorkUri.Set(this->projectdirArg);
        svnApp.SetWorkingDirectory(svnWorkUri);

        Ptr<MemoryStream> stream = MemoryStream::Create();
        svnApp.SetStdoutCaptureStream(stream.upcast<Stream>());
        svnApp.LaunchWait();

        Ptr<TextReader> reader = TextReader::Create();
        reader->SetStream(stream.upcast<Stream>());
        reader->Open();
        const String searchFor = "Revision: ";
        while (!reader->Eof())
        {
            String line;
            line = reader->ReadLine();
            IndexT result = line.FindStringIndex(searchFor);
            if(result!=InvalidIndex)
            {
                out.Append(line.ExtractRange(result+searchFor.Length(),line.Length()-(result+searchFor.Length())));
                out.Trim(" \r\n");
                break;
            }
        }
        reader->Close();

        if(!out.IsValid())
        {
            Console::Instance()->Print("Не удалось получить текущую версию SVN. Повторите попытку через 15 секунд...\n");
            n_sleep(15);
            tries++;
        }
    } while (!out.IsValid() && tries < 3);
    if (!out.IsValid())
    {
        Console::Instance()->Error("Не удалось получить версию svn.\n");
    }
    return out;
}

//------------------------------------------------------------------------------
/**
    Выполняйте работу приложения (переопределяйте в подклассах!)    
*/
void
DistributedToolkitApp::DoWork()
{
    // empty
}

//------------------------------------------------------------------------------
/**
    Subclasses may define their own command line arguments.
    This method analyzes all command line arguments and
    return the ones, which are not needed by this class.
    The filtered string will shared to slave applications for sure.
*/
String
DistributedToolkitApp::GetAdditionalArguments()
{
    String output;
    SizeT argCount = this->args.GetNumArgs();
    IndexT argIndex = 0;
    while (argIndex < argCount)
    {
        String arg = this->args.GetStringAtIndex(argIndex);
        if(
            arg == "-jobs" ||
            arg == "-projectdir" ||
            arg == "-dir" ||
            arg == "-file" ||
            arg == "-listfile" ||
            arg == "-shareddir" ||
            arg == "-processors"
            )
        {
            // игнорировать следующий аргумент, потому что это значение
            argIndex++;
        }
        else if (
            arg != "-help" &&
            arg != "-waitforkey" &&
            arg != "-slave" &&
            arg != "-force"
            )
        {
            // этот аргумент должен быть дополнительным аргументом
            output.Append(" ");
            output.Append(arg);
        }
        argIndex++;
    }
    return output;
}




//------------------------------------------------------------------------------
/**
    вещи, которые должен сделать каждый раб перед казнью,
    если только оно не распространяется
*/
void
DistributedToolkitApp::OnBeforeRunDistSlave()
{
    Console::Instance()->Print("Создать/очистить каталог экспорта\n");
    // Создайте каталог назначения, если он не существует,
    // else clean it up, by deleting all content in it.
    URI dst("dst:");
    if (!IoServer::Instance()->DirectoryExists(dst))
    {
        if (!IoServer::Instance()->CreateDirectory(dst))
        {
            Console::Instance()->Error("Не удалось создать целевой каталог: %s\n", dst.AsString().AsCharPtr());
        }
    }
    else
    {
        this->DeleteDirectoryContent(dst);
    }
}

//------------------------------------------------------------------------------
/**
    вещи, которые должен делать каждый раб после казни,
    если только оно не распространяется
*/
void
DistributedToolkitApp::OnAfterRunDistSlave()
{
    // скопировать данные из целевого каталога в общий каталог
    Console::Instance()->Print("Скопировать файлы в общий каталог...\n");
    URI dst("dst:");
    this->sharedDirControl->CopyDirectoryContentToSharedDir(dst.LocalPath());
    // удалить целевой каталог
    this->DeleteDirectoryContent(dst);
    Console::Instance()->Print("Done\n");
}

//------------------------------------------------------------------------------
/**
    вещи, которые должен делать только мастер перед выполнением
    если только оно не распространяется
*/
void
DistributedToolkitApp::OnBeforeRunDistMaster()
{
    // master создает подкаталог внутри общего каталога из guid.
    // после завершения задания его данные будут записаны из
    // каталог назначения в этот подкаталог
    this->sharedDirControl->SetUp();
}

//------------------------------------------------------------------------------
/**
    вещи, которые должен делать только мастер после выполнения
    если только оно не распространяется
*/
void
DistributedToolkitApp::OnAfterRunDistMaster()
{
    // скопируйте все из общего каталога в локальный экспорт
    Console::Instance()->Print("Скопируйте файлы из общего каталога для экспорта\n");
    this->sharedDirControl->CopyDirectoryContentFromSharedDir(URI("dst:").LocalPath());
    // finally remove shared dir content (like guid subfolder, etc.)
    this->sharedDirControl->CleanUp();
    Console::Instance()->Print("Done\n");
}

//------------------------------------------------------------------------------
/**
    Действия, выполняемые на локальном компьютере перед какой-либо работой (переопределить в подклассах) 
*/
void
DistributedToolkitApp::OnBeforeRunLocal()
{
}
//------------------------------------------------------------------------------
/**
    Действия, выполняемые на локальном компьютере после любой работы (переопределить в подклассах)  
*/
void
DistributedToolkitApp::OnAfterRunLocal()
{
}
//------------------------------------------------------------------------------
/**
    Создайте список заданий, которые добавляются в планировщик как задания,
    обрабатывается до запуска группы параллельных заданий на хосте
*/
Array<Ptr<DistributedJob>>
DistributedToolkitApp::GenerateInitializeJobList()
{
    return Array<Ptr<DistributedJob>>();
}
//------------------------------------------------------------------------------
/**
    Создайте список заданий, которые добавляются в планировщик как задания,
    обрабатывается после завершения группы параллельных заданий
*/
Array<Ptr<DistributedJob>>
DistributedToolkitApp::GenerateFinalizeJobList()
{
    return Array<Ptr<DistributedJob>>();
}

//------------------------------------------------------------------------------
/**
    рекурсивно удаляет все содержимое данного каталога 
*/
void
DistributedToolkitApp::DeleteDirectoryContent(const IO::URI & path)
{
    Array<String> files = IoServer::Instance()->ListFiles(path, "*");
    IndexT fileIndex;
    for (fileIndex = 0; fileIndex < files.Size(); fileIndex++)
    {
        URI file(path);
        file.AppendLocalPath(files[fileIndex]);
        IoServer::Instance()->DeleteFile(file);
    }
    Array<String> dirs = IoServer::Instance()->ListDirectories(path, "*");
    IndexT dirIndex;
    for (dirIndex = 0; dirIndex < dirs.Size(); dirIndex++)
    {
        URI dir(path);
        dir.AppendLocalPath(dirs[dirIndex]);
        this->DeleteDirectoryContent(dir);
        IoServer::Instance()->DeleteDirectory(dir);
    }
}

} // пространство имен DistributedTools
