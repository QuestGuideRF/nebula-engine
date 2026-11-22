//------------------------------------------------------------------------------
//  distributedcreateemptyfilejob.cc
//  (C) 2009 г. Радон Лабс ГмбХ
//  (C) 2013–2016 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "foundation/stdneb.h"
#include "distributedtools/distributedjobs/distributedcreateemptyfilejob.h"
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
__ImplementClass(DistributedCreateEmptyFileJob,'DCFJ',DistributedJob);

//------------------------------------------------------------------------------
/**
    Конструктор 
*/
DistributedCreateEmptyFileJob::DistributedCreateEmptyFileJob()
{
}

//------------------------------------------------------------------------------
/**
    Деструктор  
*/
DistributedCreateEmptyFileJob::~DistributedCreateEmptyFileJob()
{
}

//------------------------------------------------------------------------------
/**
    Запускает приложение этого задания
*/
void
DistributedCreateEmptyFileJob::RunJob()
{
    n_assert(JobError != this->jobState);
    n_assert(UpToDate == this->jobState);
    
    this->jobState = Working;

    Ptr<IO::Stream> stream = IoServer::Instance()->CreateStream(this->filePath);
    stream->SetAccessMode(IO::Stream::ReadWriteAccess);
    if(stream->Open())
    {
        stream->Close();
    }
}

//------------------------------------------------------------------------------
/**
    записывает основные атрибуты в поток xml. извлеките из этого информацию для дальнейшего содержания
*/
void
DistributedCreateEmptyFileJob::WriteXmlContent(const Ptr<IO::XmlWriter> & writer)
{
    // написать основные атрибуты
    DistributedJob::WriteXmlContent(writer);
    // записать атрибуты удаления каталога
    writer->SetString("filepath", this->filePath.LocalPath());

}
//------------------------------------------------------------------------------
/**
    читает основные атрибуты из потока XML. извлеките из этого информацию для дальнейшего содержания
*/
void
DistributedCreateEmptyFileJob::ReadXmlContent(const Ptr<IO::XmlReader> & reader)
{
    // прочитать основные атрибуты
    DistributedJob::ReadXmlContent(reader);
    // прочитать атрибуты удаления каталога
    if (reader->HasAttr("filepath"))
    {
        this->filePath = reader->GetString("filepath");
    }
}

//------------------------------------------------------------------------------
/**
    Выполните несколько проверок файлов, чтобы определить, можно ли выполнить задание под
    текущая среда.
*/
bool
DistributedCreateEmptyFileJob::IsAbleToRun()
{
    bool success = DistributedJob::IsAbleToRun();
    return success;
}

//------------------------------------------------------------------------------
/**
    Check if job application has finished and updates output stream
*/
void
DistributedCreateEmptyFileJob::UpdateJob()
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
DistributedCreateEmptyFileJob::Clone()
{
    Ptr<DistributedCreateEmptyFileJob> res = DistributedCreateEmptyFileJob::Create();
    res->CopyFrom(this);

    res->SetFilePath(this->filePath);

    return res.upcast<DistributedJob>();
}
//------------------------------------------------------------------------------
/**
    скопировать к себе атрибуты другого задания, кроме руководства   
*/
void
DistributedCreateEmptyFileJob::CopyFrom(const Ptr<DistributedCreateEmptyFileJob> & job)
{
    DistributedJob::CopyFrom(job.upcast<DistributedJob>());
    this->SetFilePath(job->filePath);
}

} // пространство имен DistributedTools