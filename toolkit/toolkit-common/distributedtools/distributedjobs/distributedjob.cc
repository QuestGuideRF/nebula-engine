//------------------------------------------------------------------------------
//  distributedjob.cc
//  (C) 2009 г. Радон Лабс ГмбХ
//  (C) 2013–2016 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "foundation/stdneb.h"
#include "distributedtools/distributedjobs/distributedjob.h"
#include "distributedtools/shareddircontrol.h"
#include "distributedtools/shareddircreator.h"
#include "io/ioserver.h"
#include "io/textreader.h"
#include "io/textwriter.h"
#include "system/nebulasettings.h"


using namespace Util;
using namespace IO;
using namespace ToolkitUtil;
using namespace System;
//------------------------------------------------------------------------------
namespace DistributedTools
{
__ImplementClass(DistributedTools::DistributedJob,'DTJB',Core::RefCounted);

//------------------------------------------------------------------------------
/**
    Конструктор 
*/
DistributedJob::DistributedJob() :
    jobState(Ready),
    svnReportedError(false),
    distributedFlag(false),
    isRunningOnSlave(false)
{
    this->jobGuid.Generate();
}

//------------------------------------------------------------------------------
/**
    Деструктор  
*/
DistributedJob::~DistributedJob()
{
    if (this->jobOutputWriter.isvalid())
    {
        if (this->jobOutputWriter->IsOpen())
        {
            this->jobOutputWriter->Close();
        }
        this->jobOutputWriter = nullptr;
    }
}
//------------------------------------------------------------------------------
/**
    Проверяет текущий путь проекта в реестре.
*/
void
DistributedJob::ValidateProjectPath()
{
    n_assert(Ready == this->jobState);
    n_assert(JobError != this->jobState);

    String projDirectory;
    if (NebulaSettings::Exists("gscept","ToolkitShared", "workdir"))
    {
        projDirectory = NebulaSettings::ReadString("gscept","ToolkitShared", "workdir");
        if(projDirectory != this->projectDirectory)
        {
            if(!NebulaSettings::WriteString("gscept","ToolkitShared", "workdir",this->projectDirectory))
            {
                this->jobState = JobError;
                return;
            }
        }
        this->jobState = ValidProjectPath;
    }
    else
    {
        this->jobState = JobError;
    }
}
//------------------------------------------------------------------------------
/**
    Пропустите процесс обновления svn. Используйте это только в том случае, если вы на 100% уверены, что
    задание работает в актуальном каталоге проекта
*/
void
DistributedJob::SkipProjectDirectoryUpdate()
{
    n_assert(JobError != this->jobState);
    n_assert(ValidProjectPath == this->jobState);
    this->jobState = UpToDate;
}

//------------------------------------------------------------------------------
/**
    Получает информацию svn текущего каталога проекта и возвращает идентификатор ревизии.
*/
String
DistributedJob::GetCurrentRevision()
{
    n_assert(this->svnPath.IsValid());
    n_assert(this->projectDirectory.IsValid());
    String out;

    AppLauncher svnApp;

    String svnArgs;
    svnArgs.Append("info");
    svnApp.SetArguments(svnArgs);

    URI svnUri;
    svnUri.Set(this->svnPath);
    svnApp.SetExecutable(svnUri);

    URI svnWorkUri;    
    svnWorkUri.Set(this->projectDirectory);
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

    return out;
}

//------------------------------------------------------------------------------
/**
    Обновляет каталог проекта этого задания.
*/
void
DistributedJob::UpdateProjectDirectory()
{
    this->jobState = UpToDate;
    return;
    n_assert(JobError != this->jobState);
    n_assert(ValidProjectPath == this->jobState);
    n_assert(this->requiredRevision.IsValid());
    
    // пропустить процесс обновления, если требуемая ревизия равна
    // текущая версия SVN.
    // это намного быстрее, чем запускать обновление svn для той же версии.
    if(this->GetCurrentRevision() == this->requiredRevision)
    {
        this->SkipProjectDirectoryUpdate();
        return;
    } 

    String svnArgs;
    svnArgs.Append("обновить --non-interactive --force -r ");
    svnArgs.Append(this->requiredRevision);
    this->svnApplication.SetArguments(svnArgs);
    
    URI svnUri;
    svnUri.Set(this->svnPath);
    this->svnApplication.SetExecutable(svnUri);
    
    URI svnWorkUri;    
    svnWorkUri.Set(this->projectDirectory);
    this->svnApplication.SetWorkingDirectory(svnWorkUri);

    this->svnErrStream = MemoryStream::Create();
    this->svnApplication.SetStderrCaptureStream(svnErrStream.upcast<Stream>());
    
    if(this->svnApplication.Launch())
    {
        this->jobState = Updating;
    }
    else
    {
        this->jobState = JobError;
    }

}

//------------------------------------------------------------------------------
/**
    Запускает приложение этого задания
*/
void
DistributedJob::RunJob()
{
    n_assert(JobError != this->jobState);
    n_assert(UpToDate == this->jobState);
    
    this->jobState = Working;

    // сделать что-нибудь
}

//------------------------------------------------------------------------------
/**
    Обновите задание. Проверяет, завершилось ли текущее действие.
    Обновляет выходной поток.
*/
void
DistributedJob::Update()
{
    n_assert(JobError != this->jobState);
    n_assert(Updating == this->jobState || Working == this->jobState);

    if(Updating == this->jobState)
    {
        this->UpdateSVN();
    }
    else if(Working == this->jobState)
    {
        this->UpdateJob();
    }
}
//------------------------------------------------------------------------------
/**
    Создайте команду XML из этого задания.    
*/
void
DistributedJob::GenerateJobCommand(const Ptr<IO::XmlWriter> & writer)
{
    writer->BeginNode("JobCommand");
    this->WriteXmlContent(writer);
    writer->EndNode();
}

//------------------------------------------------------------------------------
/**
    reads xmlnode from given reader and setup itself
*/
void
DistributedJob::SetupFromXml(const Ptr<IO::XmlReader> & reader)
{
    if (reader->HasNode("JobCommand"))
    {
        reader->SetToFirstChild("JobCommand");
        this->ReadXmlContent(reader);
        reader->SetToParent();
    }
}

//------------------------------------------------------------------------------
/**
    записывает основные атрибуты в поток xml. извлеките из этого информацию для дальнейшего содержания
*/
void
DistributedJob::WriteXmlContent(const Ptr<IO::XmlWriter> & writer)
{
    writer->SetString("guid",this->jobGuid.AsString());
    writer->SetString("state",this->GetStateString());
    writer->SetString("identifier",this->identifier);
    writer->SetString("projectdir",this->projectDirectory);
    writer->SetBool("onslavemachine",this->isRunningOnSlave);
    if(this->distributedFlag)
    {
        writer->BeginNode("Distributed");
            writer->SetString("shareddirpath",this->sharedDirPath);
            writer->SetString("shareddirguid",this->sharedDirGuid.AsString());
        writer->EndNode();
    }
    writer->BeginNode("SVNSettings");
        writer->SetString("path",this->svnPath);
        writer->SetString("revision",this->requiredRevision);
    writer->EndNode();
}
//------------------------------------------------------------------------------
/**
    читает основные атрибуты из потока XML. извлеките из этого информацию для дальнейшего содержания
*/
void
DistributedJob::ReadXmlContent(const Ptr<IO::XmlReader> & reader)
{
    if (reader->HasAttr("guid"))
    {
        this->jobGuid = Guid::FromString(reader->GetString("guid"));
    }
    if (reader->HasAttr("state"))
    {
        this->SetStateFromString(reader->GetString("state"));
    }
    if (reader->HasAttr("identifier"))
    {
        this->identifier = reader->GetString("identifier");
    }
    if (reader->HasAttr("projectdir"))
    {
        this->projectDirectory = reader->GetString("projectdir");
    }
    if (reader->HasAttr("onslavemachine"))
    {
        this->isRunningOnSlave = reader->GetBool("onslavemachine");
    }
    this->distributedFlag = reader->HasNode("Distributed");
    if(reader->HasNode("Distributed"))
    {
        reader->SetToFirstChild("Distributed");
        if (reader->HasAttr("shareddirpath"))
        {
            this->sharedDirPath = reader->GetString("shareddirpath");
        }
        if (reader->HasAttr("shareddirguid"))
        {
            this->sharedDirGuid = Guid::FromString(reader->GetString("shareddirguid"));
        }
        reader->SetToParent();
    }
    if(reader->HasNode("SVNSettings"))
    {
        reader->SetToFirstChild("SVNSettings");
        if(reader->HasAttr("path"))
        {
            this->svnPath = reader->GetString("path");
        }
        if(reader->HasAttr("revision"))
        {
            this->requiredRevision = reader->GetString("revision");
        }
        reader->SetToParent();
    }
}

//------------------------------------------------------------------------------
/**
    Выполните несколько проверок файлов, чтобы определить, можно ли выполнить задание под
    текущая среда.
*/
bool
DistributedJob::IsAbleToRun()
{
    bool success = true;
    
    // проверить путь к SVN
    URI svnUri;
    svnUri.Set(this->svnPath);
    if(!IoServer::Instance()->FileExists(svnUri))
    {
        success = false;
    }
    
    // проверить путь проекта
    URI projUri;
    projUri.Set(this->projectDirectory);
    if(!IoServer::Instance()->DirectoryExists(projUri))
    {
        success = false;
    }

    // проверьте действительный выходной путь
    if(this->distributedFlag)
    {
        Ptr<SharedDirControl> sharedDir = SharedDirCreator::Instance()->CreateSharedControlObject(this->sharedDirPath);
        success = sharedDir->ContainsGuidSubDir(this->sharedDirGuid);
        sharedDir = nullptr;
    }

    return success;
}

//------------------------------------------------------------------------------
/**
    Check if svn has finished and updates output stream
*/
void
DistributedJob::UpdateSVN()
{
    this->jobState = UpToDate;
    return;
    n_assert(JobError != this->jobState);
    n_assert(Updating == this->jobState);
    if(!this->svnApplication.IsRunning())
    {
        // проверьте, сообщает ли svn об ошибке через поток ошибок.
        // if an error happens, the job can not continue...
        if (this->svnErrStream->GetSize() > 0)
        {
            Ptr<TextReader> reader = TextReader::Create();
            reader->SetStream(this->svnErrStream.upcast<Stream>());
            reader->Open();
            n_printf("[Задание №%s] ОШИБКА ПРИ ОБНОВЛЕНИИ SVN\nsvn, вывод:\n%s\n",this->identifier.AsCharPtr(),reader->ReadAll().AsCharPtr());
            reader->Close();
            reader = nullptr;
            this->jobState = JobError;
        }
        else
        {
            this->jobState = UpToDate;
        }
        // поток ошибок svn больше не нужен
        this->svnErrStream = nullptr;
    }
}
//------------------------------------------------------------------------------
/**
    Check if job application has finished and updates output stream
*/
void
DistributedJob::UpdateJob()
{
    n_assert(JobError != this->jobState);
    n_assert(Working == this->jobState);

    this->jobState = Finished;
}

//------------------------------------------------------------------------------
/**
    Записывает заданную строку в поток вывода заданий.
*/
void
DistributedJob::WriteOutput(Util::String text)
{
    // создать выходной поток, если он еще не существует
    if (!this->jobOutputWriter.isvalid())
    {
        this->jobOutputWriter = TextWriter::Create();
        this->jobOutputWriter->SetStream(MemoryStream::Create());
    }
        
    if (!this->jobOutputWriter->IsOpen())
    {
        this->jobOutputWriter->Open();
    }
    if (this->jobOutputWriter->IsOpen())
    {
        this->jobOutputWriter->WriteString(text);
    }
}

//------------------------------------------------------------------------------
/**
    Возвращает true, если задание имеет какой-либо результат    
*/
bool
DistributedJob::HasOutputContent()
{
    if (this->jobOutputWriter.isvalid())
    {
        if (this->jobOutputWriter->HasStream())
        {
            if (this->jobOutputWriter->GetStream()->GetSize() > 0)
            {
                return true;
            }
        }
    }
    return false;
}

//------------------------------------------------------------------------------
/**
    Возвращает содержимое вывода заданий  
*/
Util::String
DistributedJob::DequeueOutputContent()
{
    String out;
    if (this->HasOutputContent())
    {
        n_assert(this->jobOutputWriter.isvalid());
        
        // закрыть средство записи вывода
        this->jobOutputWriter->Close();
        
        Ptr<TextReader> reader = TextReader::Create();
        reader->SetStream(this->jobOutputWriter->GetStream());
        if (reader->Open())
        {
            out = reader->ReadAll();
            reader->Close();
        }
        reader = nullptr;
        this->jobOutputWriter = nullptr;
    }
    return out;
}

//------------------------------------------------------------------------------
/**
    clones of this job (with a different guid)
*/
Ptr<DistributedJob>
DistributedJob::Clone()
{
    Ptr<DistributedJob> res = DistributedJob::Create();
    res->SetIdentifier(this->identifier);
    res->SetDistributedFlag(this->distributedFlag);
    res->SetSharedDirPath(this->sharedDirPath);
    res->SetSharedDirGuid(this->sharedDirGuid);
    res->SetProjectDirectory(this->projectDirectory);
    res->SetRequiredRevision(this->requiredRevision);
    res->SetStateFromString(this->GetStateString());
    res->SetSVNPath(this->svnPath);

    res->svnReportedError = this->svnReportedError;
    res->svnErrStream = this->svnErrStream;
    res->svnApplication = this->svnApplication;

    return res;
}
//------------------------------------------------------------------------------
/**
    Скопируйте атрибуты из другого задания в себя, кроме руководства. 
*/
void
DistributedJob::CopyFrom(const Ptr<DistributedJob> & job)
{
    this->SetIdentifier(job->identifier);
    this->SetDistributedFlag(job->distributedFlag);
    this->SetSharedDirPath(job->sharedDirPath);
    this->SetSharedDirGuid(job->sharedDirGuid);
    this->SetProjectDirectory(job->projectDirectory);
    this->SetRequiredRevision(job->requiredRevision);
    this->SetStateFromString(job->GetStateString());
    this->SetSVNPath(job->svnPath);

    this->svnReportedError = job->svnReportedError;
    this->svnErrStream = job->svnErrStream;
    this->svnApplication = job->svnApplication;
}

//------------------------------------------------------------------------------
/**
    возвращает строковое представление текущего состояния задания    
*/
String
DistributedJob::GetStateString()
{
    String output = "JobError";
    if(Ready == this->jobState)
    {
        return "Ready";
    }
    else if(Updating == this->jobState)
    {
        return "Updating";
    }
    else if(UpToDate == this->jobState)
    {
        return "UpToDate";
    }
    else if(Working == this->jobState)
    {
        return "Working";
    }
    else if(Finished == this->jobState)
    {
        return "Finished";
    }
    else if(JobError == this->jobState)
    {
        return "JobError";
    }
    return output;
}

//------------------------------------------------------------------------------
/**
    Устанавливает состояние заданий из заданной строки 
*/
void
DistributedJob::SetStateFromString(const Util::String & str)
{
    if("Ready" == str)
    {
        this->jobState = Ready;
    }
    else if("Updating" == str)
    {
        this->jobState = Updating;
    }
    else if("UpToDate" == str)
    {
        this->jobState = UpToDate;
    }
    else if("Working" == str)
    {
        this->jobState = Working;
    }
    else if("Finished" == str)
    {
        this->jobState = Finished;
    }
    else 
    {
        this->jobState = JobError;
    }
    
}
} // пространство имен DistributedTools