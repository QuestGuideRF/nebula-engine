//------------------------------------------------------------------------------
//  distributedappjob.cc
//  (C) 2009 г. Радон Лабс ГмбХ
//  (C) 2013–2016 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "foundation/stdneb.h"
#include "distributedtools/distributedjobs/distributedappjob.h"
#include "io/ioserver.h"
#include "io/textreader.h"
#include "io/textwriter.h"

using namespace Util;
using namespace IO;
using namespace ToolkitUtil;
using namespace System;
//------------------------------------------------------------------------------
namespace DistributedTools
{
__ImplementClass(DistributedAppJob,'DTAJ',DistributedTools::DistributedJob);

//------------------------------------------------------------------------------
/**
    Конструктор 
*/
DistributedAppJob::DistributedAppJob() :
    appReportedError(false)
{

}

//------------------------------------------------------------------------------
/**
    Деструктор  
*/
DistributedAppJob::~DistributedAppJob()
{
}

//------------------------------------------------------------------------------
/**
    Запустите задание. В этом случае запустите приложение
*/
void
DistributedAppJob::RunJob()
{
    n_assert(JobError != this->jobState);
    n_assert(UpToDate == this->jobState);
    
    // создать временный файл списка в хост-системе заданий
    this->localListPath.Set("temp:distributedtools/"+this->GetGuid().AsString()+".txt");   
    if(!IoServer::Instance()->DirectoryExists(this->localListPath.LocalPath().ExtractDirName()))
    {
        IoServer::Instance()->CreateDirectory(this->localListPath.LocalPath().ExtractDirName());
    }
    
    // записать временный файл списка
    Ptr<TextWriter> writer = TextWriter::Create();
    writer->SetStream(IoServer::Instance()->CreateStream(this->localListPath));
    writer->Open();
    IndexT fileIndex;
    for (fileIndex = 0; fileIndex < this->fileList.Size(); fileIndex++)
    {
        writer->WriteString(this->fileList[fileIndex]);   
        writer->WriteString("\n");
    }
    IndexT dirIndex;
    for (dirIndex = 0; dirIndex < this->dirList.Size(); dirIndex++)
    {
        Array<String> dirFiles = IoServer::Instance()->ListFiles(this->dirList[dirIndex], "*");
        for (fileIndex = 0; fileIndex < dirFiles.Size(); fileIndex++)
        {
            String path;
            path.Format("%s/%s", this->dirList[dirIndex].AsCharPtr(), dirFiles[fileIndex].AsCharPtr());
            writer->WriteString(path);
            writer->WriteString("\n");
        }
    }

    writer->Close();
    writer = nullptr;

    String jobArgs;
    jobArgs.Append(this->arguments);
    jobArgs.Append(" -listfile ");
    jobArgs.Append(this->localListPath.LocalPath());

    // if the job is runnign on a local machine,
    // приложение должно знать это, чтобы вести себя по-другому.
    if (this->IsRunningOnSlave())
    {
        jobArgs.Append(" -slave");
    }

    this->jobApplication.SetArguments(jobArgs);
    
    URI appUri = URI(this->applicationPath);
    this->jobApplication.SetExecutable(appUri.LocalPath());
    this->jobApplication.SetWorkingDirectory(appUri.LocalPath().ExtractDirName());
    
    this->appOutStream = MemoryStream::Create();
    this->jobApplication.SetStdoutCaptureStream(appOutStream.upcast<Stream>());
    
    if(this->jobApplication.Launch())
    {
        this->jobState = Working;
    }
    else
    {
        this->jobState = JobError;
    }
}

//------------------------------------------------------------------------------
/**
*/
void
DistributedAppJob::WriteXmlContent(const Ptr<IO::XmlWriter> & writer)
{
    // написать основные атрибуты
    DistributedJob::WriteXmlContent(writer);
    // написать специфичные для приложения атрибуты
    writer->BeginNode("AppSettings");
        writer->SetString("path",this->applicationPath);
        writer->SetString("arguments",this->arguments);
    writer->EndNode();
    writer->BeginNode("FileList");
    IndexT file;
    for(file = 0; file < this->fileList.Size(); file++)
    {
        writer->BeginNode("File");
            writer->SetString("path",this->fileList[file]);
        writer->EndNode();
    }
    writer->EndNode();
    writer->BeginNode("DirList");
    IndexT dir;
    for(dir = 0; dir < this->dirList.Size(); dir++)
    {
        writer->BeginNode("Dir");
            writer->SetString("path",this->dirList[dir]);
        writer->EndNode();
    }
    writer->EndNode();
}
//------------------------------------------------------------------------------
/**
*/
void
DistributedAppJob::ReadXmlContent(const Ptr<IO::XmlReader> & reader)
{
    // прочитать основные атрибуты
    DistributedJob::ReadXmlContent(reader);
    // прочитать определенные атрибуты приложения
    if(reader->HasNode("AppSettings"))
    {
        reader->SetToFirstChild("AppSettings");
        if(reader->HasAttr("path"))
        {
            this->applicationPath = reader->GetString("path");
        }
        if(reader->HasAttr("arguments"))
        {
            this->arguments = reader->GetString("arguments");
        }
        reader->SetToParent();
    }
    if(reader->HasNode("FileList"))
    {
        reader->SetToFirstChild("FileList");
        if(reader->HasNode("File"))
        {
            reader->SetToFirstChild("File");
            do 
            {
                if(reader->HasAttr("path"))
                {
                    this->fileList.Append(reader->GetString("path"));
                }
            } while (reader->SetToNextChild("File"));
        }
        reader->SetToParent();
    }
    if(reader->HasNode("DirList"))
    {
        reader->SetToFirstChild("DirList");
        if(reader->HasNode("Dir"))
        {
            reader->SetToFirstChild("Dir");
            do 
            {
                if(reader->HasAttr("path"))
                {
                    this->dirList.Append(reader->GetString("path"));
                }
            } while (reader->SetToNextChild("Dir"));
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
DistributedAppJob::IsAbleToRun()
{
    bool success = DistributedJob::IsAbleToRun();
    
    // проверьте путь приложения
    if(!IoServer::Instance()->FileExists(this->applicationPath))
    {
        success = false;
    }
    
    return success;
}

//------------------------------------------------------------------------------
/**
    Check if job application has finished and updates output stream
*/
void
DistributedAppJob::UpdateJob()
{
    n_assert(JobError != this->jobState);
    n_assert(Working == this->jobState);

    if(!this->jobApplication.IsRunning())
    {
        // check if a list file was created and remove it from disk
        if(IoServer::Instance()->FileExists(this->localListPath))
        {
            IoServer::Instance()->DeleteFile(this->localListPath);
        }
        // распечатать результат задания
        Ptr<TextReader> reader = TextReader::Create();
        reader->SetStream(this->appOutStream.upcast<Stream>());
        reader->Open();
        String line;
        while (!reader->Eof())
        {
            line.Format("[#%s] %s\n",this->GetIdentifier().AsCharPtr(),reader->ReadLine().AsCharPtr());
            this->WriteOutput(line);
        }
        reader->Close();
        reader = nullptr;
        this->appOutStream = nullptr;

        this->jobState = Finished;
    }
    else
    {
        // очистить выходной поток, если он станет большим
        if(this->appOutStream->GetSize() > 2048)
        {
            this->appOutStream->Close();
            this->appOutStream->SetAccessMode(Stream::ReadAccess);
            // распечатать результат задания
            Ptr<TextReader> reader = TextReader::Create();
            reader->SetStream(this->appOutStream.upcast<Stream>());
            reader->Open();
            String line;
            while (!reader->Eof())
            {
                line.Format("[#%s] %s\n",this->GetIdentifier().AsCharPtr(),reader->ReadLine().AsCharPtr());
                this->WriteOutput(line);
            }
            reader->Close();
            reader = nullptr;
            
            this->appOutStream->SetSize(0);
            this->appOutStream->SetAccessMode(Stream::WriteAccess);
            this->appOutStream->Open();
        }
    }

}

//------------------------------------------------------------------------------
/**
    clones of this job (with a different guid)
*/
Ptr<DistributedJob>
DistributedAppJob::Clone()
{
    Ptr<DistributedAppJob> res = DistributedAppJob::Create();
    res->CopyFrom(this);
    
    res->SetApplicationPath(this->applicationPath);
    res->SetArguments(this->arguments);
    res->SetFileList(this->fileList);
    res->SetDirList(this->dirList);

    res->appReportedError = this->appReportedError;
    res->appOutStream = this->appOutStream;
    res->jobApplication = this->jobApplication;
    res->localListPath = this->localListPath;

    return res.upcast<DistributedJob>();
}

//------------------------------------------------------------------------------
/**
    Скопируйте атрибуты из другого задания в себя, кроме руководства. 
*/
void
DistributedAppJob::CopyFrom(const Ptr<DistributedAppJob> & job)
{
    DistributedJob::CopyFrom(job.upcast<DistributedJob>());
    this->SetApplicationPath(job->applicationPath);
    this->SetArguments(job->arguments);
    this->SetFileList(job->fileList);
    this->SetDirList(job->dirList);

    this->appReportedError = job->appReportedError;
    this->appOutStream = job->appOutStream;
    this->jobApplication = job->jobApplication;
    this->localListPath = job->localListPath;
}

} // пространство имен DistributedTools