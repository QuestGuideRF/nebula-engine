//------------------------------------------------------------------------------
//  remotejobservice.cc
//  (C) 2009 г. РадонЛабс ГмбХ
//  (C) 2013–2016 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "foundation/stdneb.h"
#include "remotejobservice.h"
#include "distributedjobs/distributedjob.h"
#include "io/xmlreader.h"
#include "io/xmlwriter.h"
#include "io/memorystream.h"
#include "io/console.h"
#include "net/messageclient.h"

using namespace Net;
using namespace IO;
using namespace Util;

namespace DistributedTools
{
    __ImplementClass(DistributedTools::RemoteJobService,'DRJS',Core::RefCounted)
//------------------------------------------------------------------------------
/**
    Конструктор 
*/
RemoteJobService::RemoteJobService() :
    isOpen(false),
    isClean(true),
    verboseFlag(false),
    state(Closed),
    processorCount(1)
{
}

//------------------------------------------------------------------------------
/**
    Установите соединение с удаленной службой   
*/
bool
RemoteJobService::Open()
{
    n_assert(!this->isOpen);
   
    if (!this->clientID.IsValid())
    {
        this->clientID.Generate();
    }

    this->tcpclient = MessageClient::Create();
    this->tcpclient->SetBlocking(true);
    this->tcpclient->SetServerAddress(this->ipAddress);
    
    if (TcpClient::Error != this->tcpclient->Connect())
    {
        if (this->tcpclient->IsConnected())
        {
            this->state = Connecting;
            this->SendDetailedStateRequest();
            this->isOpen = true;
            return true;
        }
    }
    this->tcpclient = nullptr;
    return false;
}

//------------------------------------------------------------------------------
/**
    Тесное подключение к сервису 
*/
void
RemoteJobService::Close()
{
    n_assert(this->isOpen);
    // нам нужно проверить подключенный tcpclient,
    // because Send() can disconnect from
    // клиент в некоторых случаях...
    if (this->tcpclient->IsConnected())
    {
        this->tcpclient->Disconnect();
    }
    this->tcpclient = nullptr;
    this->state = Closed;

    if (this->timer.Running())
    {
        this->timer.Stop();
    }
    this->timer.Reset();

    if (this->lastReceivedAnswerTimer.Running())
    {
        this->lastReceivedAnswerTimer.Stop();
    }
    this->lastReceivedAnswerTimer.Reset();

    this->isOpen = false;
}

//------------------------------------------------------------------------------
/**
    Отправить задание в сервис
*/
bool
RemoteJobService::SendJob(const Ptr<DistributedJob> & job)
{

    bool success = false;
    n_assert(this->tcpclient.isvalid());
    if (!this->tcpclient->IsConnected())
    {
        this->Close();
        this->state = ServiceError;
        this->errorMessage = "Потеряно соединение с сервисом.";
        return false;
    }
    Ptr<XmlWriter> writer = XmlWriter::Create();
    writer->SetStream(this->tcpclient->GetSendStream());
    if (writer->Open())
    {
        writer->BeginNode("Command");
            this->WriteClientHeader(writer);
            writer->SetString("name","JobCommand");
            if (job->IsA('DTAJ'))
            {
                writer->BeginNode("AppJob");
            }
            else if (job->IsA('DDCJ'))
            {
                writer->BeginNode("DeleteDirectoryContentJob");
            }
            else
            {
                writer->BeginNode("Job");
            }
            job->GenerateJobCommand(writer);
            writer->EndNode();
        writer->EndNode();
        writer->Close();
        success = this->tcpclient->Send();
        if (success)
        {
            this->sentJobGuids.Append(job->GetGuid());
            this->uncompleteJobs.Append(job);
        }
    }
    writer = nullptr;
    return success;
}

//------------------------------------------------------------------------------
/**
    Запустить все отправленные задания
*/
void
RemoteJobService::RunJobs()
{
    n_assert(this->tcpclient.isvalid());
    if (!this->tcpclient->IsConnected())
    {
        this->Close();
        this->state = ServiceError;
        this->errorMessage = "Потеряно соединение с сервисом.";
        return;
    }
    Ptr<XmlWriter> writer = XmlWriter::Create();
    writer->SetStream(this->tcpclient->GetSendStream());
    if (writer->Open())
    {
        writer->BeginNode("Command");
        this->WriteClientHeader(writer);
        writer->SetString("name","RunJobs");
            writer->BeginNode("InitializeJobs");
            IndexT job;
            for (job = 0; job < this->initializeJobs.Size(); job++)
            {
                if (this->initializeJobs[job]->IsA('DTAJ'))
                {
                    writer->BeginNode("AppJob");
                }
                else if (this->initializeJobs[job]->IsA('DDCJ'))
                {
                    writer->BeginNode("DeleteDirectoryContentJob");
                }
                else
                {
                    writer->BeginNode("Job");
                }
                this->initializeJobs[job]->GenerateJobCommand(writer);
                writer->EndNode();
            }
            writer->EndNode();
            writer->BeginNode("FinalizeJobs");
            for (job = 0; job < this->finalizeJobs.Size(); job++)
            {
                if (this->finalizeJobs[job]->IsA('DTAJ'))
                {
                    writer->BeginNode("AppJob");
                }
                else if (this->finalizeJobs[job]->IsA('DDCJ'))
                {
                    writer->BeginNode("DeleteDirectoryContentJob");
                }
                else
                {
                    writer->BeginNode("Job");
                }
                this->finalizeJobs[job]->GenerateJobCommand(writer);
                writer->EndNode();
            }
            writer->EndNode();
        writer->EndNode();
        writer->Close();
        this->tcpclient->Send();
    }
    writer = nullptr;
    this->state = Busy;

    // поскольку могут приходить устаревшие сообщения о состоянии, удалите все направляющие
    // from the pending list... this way only actual messages are handled
    if (this->pendingRequestGuids.Size() > 0)
    {
        n_printf("Отмена ожидающих запросов %i.\n",this->pendingRequestGuids.Size());
    }
    this->pendingRequestGuids.Clear();

    return;    
}
//------------------------------------------------------------------------------
/**
    Обновляет состояние задания и обрабатывает получение сообщений.
*/
void
RemoteJobService::Update()
{
    n_assert(this->isOpen);
    n_assert(this->tcpclient.isvalid());
    if (!this->tcpclient->IsConnected())
    {
        this->Close();
        this->state = ServiceError;
        this->errorMessage = "Потеряно соединение с сервисом.";
        return;
    }
    // отправить запрос состояния все 1 секунда
    if (timer.GetTime()>1 || !timer.Running())
    {
        if (!this->SendStateRequest())
        {
            // соединение кажется недействительным
            this->Close();
            this->state = ServiceError;
            this->errorMessage = "Потеряно соединение с сервисом.";
            return;
        }
    }

    // перейти в неблокирующий режим только для приема
    this->tcpclient->SetBlocking(false);
    if (this->tcpclient->Recv())
    {
        this->tcpclient->SetBlocking(true);
        
        Ptr<XmlReader> reader = XmlReader::Create();
        reader->SetStream(this->tcpclient->GetRecvStream());
        if (reader->Open())
        {
            // проверьте, являются ли входящие данные служебной командой
            if (reader->HasNode("/Command"))
            {
                reader->SetToNode("/Command");
                this->HandleCommandNode(reader, this->tcpclient->GetServerAddress());
                // перезапустить таймер ответа
                if (this->lastReceivedAnswerTimer.Running())
                {
                    this->lastReceivedAnswerTimer.Stop();
                    this->lastReceivedAnswerTimer.Reset();
                }
                this->lastReceivedAnswerTimer.Start();
            }
            reader->Close();
        }
        reader = nullptr;
    }
    this->tcpclient->SetBlocking(true);

    // переключиться в состояние ошибки, если последний ответ старше 15 минут...
    if (this->lastReceivedAnswerTimer.GetTime() > 900)
    {
        this->errorMessage = "Ожидаемые ответы службы истекли.";
        this->state = ServiceError;
    }
}
//------------------------------------------------------------------------------
/**
    Обрабатывает командный узел в данном средстве чтения XML, указывающий на этот узел.
*/
void
RemoteJobService::HandleCommandNode(const Ptr<XmlReader> & reader, const Net::IpAddress & address)
{
    n_assert("Command" == reader->GetCurrentNodeName());
    if (reader->HasAttr("name"))
    {
        // обрабатывать различные команды здесь
        if ("DetailedState" == reader->GetString("name"))
        {
            this->HandleStateNode(reader, address);
            if (reader->HasNode("Processorcount"))
            {
                if (reader->SetToFirstChild("Processorcount"))
                {
                    if(reader->HasAttr("value"))
                    {
                        this->processorCount = reader->GetInt("value");
                    }
                    reader->SetToParent();
                }
            }
        }
        else if ("CurrentState" == reader->GetString("name"))
        {
            this->HandleStateNode(reader, address);
        }
        else if ("JobAppendSuccess" == reader->GetString("name"))
        {
            if (reader->HasAttr("jobGuid"))
            {
                IndexT idx = this->sentJobGuids.FindIndex(Guid::FromString(reader->GetString("jobGuid")));
                n_assert(idx!=InvalidIndex);
                this->addedJobGuids.Append(this->sentJobGuids[idx]);
                this->sentJobGuids.EraseIndex(idx);
            }
        }
        else if ("JobAppendFailed" == reader->GetString("name"))
        {
            if (reader->HasAttr("jobGuid"))
            {
                bool retry = false;
                if (reader->HasAttr("reason"))
                {
                    if (reader->GetString("reason") == "busy")
                    {
                        retry = true;
                        this->state = Busy;
                    }
                    else if(reader->GetString("reason") == "reserved")
                    {
                        retry = true;
                        this->state = Reserved;
                    }

                }
                if (retry)
                {
                    IndexT idx = this->sentJobGuids.FindIndex(Guid::FromString(reader->GetString("jobGuid")));
                    n_assert(idx!=InvalidIndex);
                    this->failedJobGuids.Append(this->sentJobGuids[idx]);
                    this->sentJobGuids.EraseIndex(idx);
                }
                else
                {
                    IndexT idx = this->sentJobGuids.FindIndex(Guid::FromString(reader->GetString("jobGuid")));
                    n_assert(idx!=InvalidIndex);
                    this->sentJobGuids.EraseIndex(idx);
                    this->state = ServiceError;
                    this->errorMessage = "Не удалось добавить задание в удаленную службу.";
                }
            }
        }
        else if ("CleanedUp" == reader->GetString("name"))
        {
            this->isClean = true;
        }
        
        // проверить результат задания

        if (reader->HasNode("JobOutput"))
        {
            reader->SetToFirstChild("JobOutput");
            do 
            {
                Console::Instance()->Print(
                    "\n\n[Удаленный вывод с %s]\n\n%s\n\n",
                    address.GetHostName().AsCharPtr(),
                    reader->GetContent().AsCharPtr());
            } while (reader->SetToNextChild("JobOutput"));
        }

        // проверьте узлы завершенного задания или ошибки в команде xml
        
        if (reader->HasNode("JobFinished"))
        {
            reader->SetToFirstChild("JobFinished");
            do 
            {
                n_printf("Получен командный узел 'JobFinished'\n");
                if (reader->HasAttr("jobGuid"))
                {
                    Guid guid = Guid::FromString(reader->GetString("jobGuid"));
                    this->finishedJobGuids.Append(guid);
                    IndexT idx = this->addedJobGuids.FindIndex(guid);
                    if (idx!=InvalidIndex)
                    {
                        this->addedJobGuids.EraseIndex(idx);
                    }
                }
                if (reader->HasAttr("wasLastJob"))
                {
                    if (reader->GetBool("wasLastJob"))
                    {
                        this->uncompleteJobs.Clear();
                    }
                }
            } while (reader->SetToNextChild("JobFinished"));
        }
        
        if (reader->HasNode("JobError"))
        {
            reader->SetToFirstChild("JobError");
            do
            {
                if (reader->HasAttr("jobGuid"))
                {
                    this->state = ServiceError;
                    this->errorMessage.Format("Задание с руководством %s сообщило об ошибке.\n",reader->GetString("jobGuid").AsCharPtr());
                }
            } while (reader->SetToNextChild("JobError"));
        }
    }
}

//------------------------------------------------------------------------------
/**
    Анализирует действительный узел состояния для текущего состояния
*/
void
RemoteJobService::HandleStateNode(const Ptr<IO::XmlReader> & reader, const Net::IpAddress & address)
{
    if(reader->HasAttr("value") && reader->HasAttr("guid") && this->state != ServiceError)
    {
        // обрабатывать это сообщение только в том случае, если полученное руководство равно
        // одному гиду в списке ожидающих гидов. В таком случае удалите
        // руководство из списка.
        IndexT guidIdx = this->pendingRequestGuids.FindIndex(
                                Guid::FromString(reader->GetString("guid"))
                                );
        if(guidIdx!=InvalidIndex)
        {
            this->pendingRequestGuids.EraseIndex(guidIdx);
            if(this->verboseFlag)
            {
                n_printf("Received State: '%s' from %s ( %s )\n", reader->GetString("value").AsCharPtr(), address.GetHostName().AsCharPtr(), address.GetHostAddr().AsCharPtr());
            }
            if("ready" == reader->GetString("value"))
            {
                this->state = Ready;
            }
            else if("reserved" == reader->GetString("value"))
            {
                this->state = Reserved;
            }
            else if("exclusiveReady" == reader->GetString("value"))
            {
                this->state = ExclusiveReady;
            }
            else if("busy" == reader->GetString("value"))
            {
                this->state = Busy;
            }
            else if("inactive" == reader->GetString("value"))
            {
                this->state = Closed;
            }
            else
            {
                this->state = ServiceError;
                this->errorMessage = "Получено недопустимое состояние";
            }
        }
        else
        {
            if(verboseFlag)
            {
                n_printf("Received and ignored outdated State: '%s' from %s ( %s )\n", reader->GetString("value").AsCharPtr(), address.GetHostName().AsCharPtr(), address.GetHostAddr().AsCharPtr());
            }
        }
    }
    else
    {
        this->state = ServiceError;
        this->errorMessage = "Получено недопустимое состояние";
    }
}

//------------------------------------------------------------------------------
/**
    Отправьте запрос состояния, чтобы получить состояние удаленной службы.    
*/
bool
RemoteJobService::SendStateRequest()
{
    // Создайте руководство, идентифицирующее этот запрос.
    // Если получено сообщение о состоянии, оно будет обработано только
    // if it has a guid in it, that was sent by us.
    // Ожидающие руководства могут быть удалены, если устаревшие сообщения не должны обрабатываться...   
    Guid guid;
    guid.Generate();

    if(this->timer.Running())
    {
        this->timer.Stop();
        this->timer.Reset();
    }
    this->timer.Start();
    bool retval = false;
    n_assert(this->tcpclient.isvalid());
    if(!this->tcpclient->IsConnected())
    {
        this->Close();
        this->state = ServiceError;
        this->errorMessage = "Потеряно соединение с сервисом.";
        return retval;
    }
    Ptr<XmlWriter> writer = XmlWriter::Create();
    writer->SetStream(this->tcpclient->GetSendStream());
    if(writer->Open())
    {
        writer->BeginNode("Command");
            this->WriteClientHeader(writer);
            writer->SetString("name","StateRequest");
            writer->SetString("guid",guid.AsString());
        writer->EndNode();
        writer->Close();
        retval = this->tcpclient->Send();
        if(retval)
        {
            this->pendingRequestGuids.Append(guid);
        }
    }
    writer = nullptr;
    return retval;
}

//------------------------------------------------------------------------------
/**
    Отправьте запрос, в котором запрашивается подробный статус услуги.
    Это вызывается, когда служба впервые подключается к удаленному процессу.
*/
bool
RemoteJobService::SendDetailedStateRequest()
{
    // Создайте руководство, идентифицирующее этот запрос.
    // Если получено сообщение о состоянии, оно будет обработано только
    // if it has a guid in it, that was sent by us.
    // Ожидающие руководства могут быть удалены, если устаревшие сообщения не должны обрабатываться...   
    Guid guid;
    guid.Generate();

    bool retval = false;
    n_assert(this->tcpclient.isvalid());
    if(!this->tcpclient->IsConnected())
    {
        this->Close();
        this->state = ServiceError;
        this->errorMessage = "Потеряно соединение с сервисом.";
        return retval;
    }
    Ptr<XmlWriter> writer = XmlWriter::Create();
    writer->SetStream(this->tcpclient->GetSendStream());
    if(writer->Open())
    {
        writer->BeginNode("Command");
            this->WriteClientHeader(writer);
            writer->SetString("name","DetailedStateRequest");
            writer->SetString("guid",guid.AsString());
        writer->EndNode();
        writer->Close();
        retval = this->tcpclient->Send();
        if(retval)
        {
            this->pendingRequestGuids.Append(guid);
        }
    }
    writer = nullptr;
    return retval;
}

//------------------------------------------------------------------------------
/**
*/
void 
RemoteJobService::WriteClientHeader(const Ptr<IO::XmlWriter> & writer)
{
    writer->SetString("clientID", this->clientID.AsString());
}

//------------------------------------------------------------------------------
/**
    Пытается отправить сообщение сервису, что он забанен
*/
void
RemoteJobService::SendBannedInfo()
{
    if(this->IsOpen())
    {    
        // Создайте руководство, идентифицирующее этот запрос.
        // Если получено сообщение о состоянии, оно будет обработано только
        // if it has a guid in it, that was sent by us.
        // Ожидающие руководства могут быть удалены, если устаревшие сообщения не должны обрабатываться...   
        Guid guid;
        guid.Generate();
        
        bool retval = false;

        Ptr<XmlWriter> writer = XmlWriter::Create();
        writer->SetStream(this->tcpclient->GetSendStream());
        if (writer->Open())
        {
            writer->BeginNode("Command");
            this->WriteClientHeader(writer);
            writer->SetString("name","Banned");
            writer->SetString("guid",guid.AsString());
            writer->EndNode();
            writer->Close();
            retval = this->tcpclient->Send();
            if(retval)
            {
                this->pendingRequestGuids.Append(guid);
            }
        }
        writer = nullptr;
    }
}

//------------------------------------------------------------------------------
/**
    Возвращает true, если сообщение от службы было получено ранее,
    который объявляет службу очищенной.
*/
bool
RemoteJobService::HasCleanedUp()
{
    return this->isClean;
}

//------------------------------------------------------------------------------
/**
    Сбросьте все внутренние параметры этого сервиса. После вызова сброса
    сервис находится в закрытом состоянии.
*/
void
RemoteJobService::Reset()
{
    // if service is open just close it, else be sure tcpclient and state
    // сбрасываются
    if (this->IsOpen())
    {
        this->Close();
    }
    else
    {
        this->tcpclient = nullptr;
        this->state = Closed;
    }
    this->addedJobGuids.Clear();
    this->failedJobGuids.Clear();
    this->finalizeJobs.Clear();
    this->finishedJobGuids.Clear();
    this->initializeJobs.Clear();
    this->pendingRequestGuids.Clear();
    this->sentJobGuids.Clear();
    this->uncompleteJobs.Clear();
    this->processorCount = 1;
    this->errorMessage = "";
    
    if (this->timer.Running())
    {
        this->timer.Stop();
    }
    this->timer.Reset();

    if (this->lastReceivedAnswerTimer.Running())
    {
        this->lastReceivedAnswerTimer.Stop();
    }
    this->lastReceivedAnswerTimer.Reset();
}

} // пространство имен DistributedTools