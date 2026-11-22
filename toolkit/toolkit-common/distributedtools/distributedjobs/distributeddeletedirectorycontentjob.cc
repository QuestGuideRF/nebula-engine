//------------------------------------------------------------------------------
//  distributeddeletedirectorycontentjob.cc
//  (C) 2009 г. Радон Лабс ГмбХ
//  (C) 2013–2016 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "foundation/stdneb.h"
#include "distributedtools/distributedjobs/distributeddeletedirectorycontentjob.h"
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
__ImplementClass(DistributedDeleteDirectoryContentJob,'DDCJ',DistributedJob);

//------------------------------------------------------------------------------
/**
    Конструктор 
*/
DistributedDeleteDirectoryContentJob::DistributedDeleteDirectoryContentJob()
{

}

//------------------------------------------------------------------------------
/**
    Деструктор  
*/
DistributedDeleteDirectoryContentJob::~DistributedDeleteDirectoryContentJob()
{
}

//------------------------------------------------------------------------------
/**
    Запускает приложение этого задания
*/
void
DistributedDeleteDirectoryContentJob::RunJob()
{
    n_assert(JobError != this->jobState);
    n_assert(UpToDate == this->jobState);
    
    this->jobState = Working;

    // получить все файлы из каталога
    Array<String> files = IoServer::Instance()->ListFiles(this->delDirPath.LocalPath(), "*");
    // удалить каждый файл
    IndexT fileIndex;
    for (fileIndex = 0; fileIndex < files.Size(); fileIndex++)
    {
        String file;
        file.Format("%s/%s", this->delDirPath.LocalPath().AsCharPtr(), files[fileIndex].AsCharPtr());
        IoServer::Instance()->DeleteFile(file);
    }

    
}

//------------------------------------------------------------------------------
/**
    записывает основные атрибуты в поток xml. извлеките из этого информацию для дальнейшего содержания
*/
void
DistributedDeleteDirectoryContentJob::WriteXmlContent(const Ptr<IO::XmlWriter> & writer)
{
    // написать основные атрибуты
    DistributedJob::WriteXmlContent(writer);
    // записать атрибуты удаления каталога
    writer->SetString("deldirpath", this->delDirPath.LocalPath());

}
//------------------------------------------------------------------------------
/**
    читает основные атрибуты из потока XML. извлеките из этого информацию для дальнейшего содержания
*/
void
DistributedDeleteDirectoryContentJob::ReadXmlContent(const Ptr<IO::XmlReader> & reader)
{
    // прочитать основные атрибуты
    DistributedJob::ReadXmlContent(reader);
    // прочитать атрибуты удаления каталога
    if (reader->HasAttr("deldirpath"))
    {
        this->delDirPath = reader->GetString("deldirpath");
    }
}

//------------------------------------------------------------------------------
/**
    Выполните несколько проверок файлов, чтобы определить, можно ли выполнить задание под
    текущая среда.
*/
bool
DistributedDeleteDirectoryContentJob::IsAbleToRun()
{
    bool success = DistributedJob::IsAbleToRun();
    // проверить путь к каталогу
    if(!this->delDirPath.IsValid())
    {
        success = false;
    }
    if(!IoServer::Instance()->DirectoryExists(this->delDirPath))
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
DistributedDeleteDirectoryContentJob::UpdateJob()
{
    n_assert(JobError != this->jobState);
    n_assert(Working == this->jobState);

    this->jobState = Finished;
}

//------------------------------------------------------------------------------
/**
    clones of this job (with a different guid)
*/
Ptr<DistributedJob>
DistributedDeleteDirectoryContentJob::Clone()
{
    Ptr<DistributedDeleteDirectoryContentJob> res = DistributedDeleteDirectoryContentJob::Create();
    res->CopyFrom(this);

    res->SetDelDirectoryPath(this->delDirPath);

    return res.upcast<DistributedJob>();
}
//------------------------------------------------------------------------------
/**
    скопировать к себе атрибуты другого задания, кроме руководства   
*/
void
DistributedDeleteDirectoryContentJob::CopyFrom(const Ptr<DistributedDeleteDirectoryContentJob> & job)
{
    DistributedJob::CopyFrom(job.upcast<DistributedJob>());
    this->SetDelDirectoryPath(job->delDirPath);
}

} // пространство имен DistributedTools