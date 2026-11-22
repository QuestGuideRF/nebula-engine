//------------------------------------------------------------------------------
//  distributedjobscheduler.cc
//  (C) 2009 г. Радон Лабс ГмбХ
//  (C) 2013–2016 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "foundation/stdneb.h"
#include "distributedtools/distributedjobscheduler.h"
#include "distributedtools/distributedjobs/distributedjob.h"
#include "io/console.h"
#include "io/textwriter.h"

using namespace IO;
using namespace Util;
using namespace Net;
//------------------------------------------------------------------------------
namespace DistributedTools
{
//------------------------------------------------------------------------------
/**
    Конструктор 
*/
DistributedJobScheduler::DistributedJobScheduler() :
    isRunningJobs(false),
    isOpen(false),
    maxParallelLocalJob(2),
    useRemoteServices(false),
    printLocalJobOutput(true)
{
}

//------------------------------------------------------------------------------
/**
    Откройте планировщик  
*/
bool
DistributedJobScheduler::Open()
{
    n_assert(!this->isOpen);   
    this->isOpen = true;
    return true;
}

//------------------------------------------------------------------------------
/**
    Закрыть планировщик 
*/
void
DistributedJobScheduler::Close()
{
    n_assert(this->isOpen);

    this->bannedRemoteServices.Clear();
    this->finalizeJobs.Clear();
    this->initializeJobs.Clear();
    this->localRunningJobs.Clear();
    this->readyJobs.Clear();
    this->remoteFinalizeJobs.Clear();
    this->remoteInitializeJobs.Clear();
    this->remoteRunningJobs.Clear();
    this->remoteServices.Clear();
    this->isRunningJobs = false;

    this->isOpen = false;
}

//------------------------------------------------------------------------------
/**
    Прикрепить вакансию к списку готовых вакансий
*/
void
DistributedJobScheduler::AttachJob(const Ptr<DistributedJob> & job)
{
    n_assert(this->isOpen);
    n_assert(!this->isRunningJobs);
    this->readyJobs.Append(job);
}

//------------------------------------------------------------------------------
/**
    Подключите службу удаленных заданий, которая используется этим планировщиком для обработки
    рабочие места.
*/
void
DistributedJobScheduler::AttachRemoteJobService(const Ptr<RemoteJobService> & service)
{
    n_assert(this->isOpen);
    n_assert(!this->isRunningJobs);     
    this->remoteServices.Append(service);
}

//------------------------------------------------------------------------------
/**
    Запустите прикрепленные задания. Возвращается немедленно.
*/
void
DistributedJobScheduler::RunJobs()
{
    n_assert(this->isOpen);
    n_assert(!this->isRunningJobs);
    
    // настроить инициализацию и завершение списков заданий
    this->remoteInitializeJobs.Clear();
    this->remoteFinalizeJobs.Clear();
    IndexT job;
    for(job = 0; job < this->initializeJobs.Size(); job++)
    {
        this->remoteInitializeJobs.Append(this->initializeJobs[job]->Clone());
    }
    for(job = 0; job < this->finalizeJobs.Size(); job++)
    {
        this->remoteFinalizeJobs.Append(this->finalizeJobs[job]->Clone());
    }

    // немедленно запускайте локальные задания и начинайте отправлять удаленные задания
    // подключенным рабам
    this->StartReadyJobs();
}

//------------------------------------------------------------------------------
/**
    Updates the scheduler.
    
    First all initialize-jobs will start sequentially on the local machine.

    Then, after all initialize-jobs have finished, it process all standard jobs
    by starting them local or sending them to assigned remote services.

    After all standard jobs have finished, all finalize-jobs will be processed
    sequentially on the local machine.

    If a job has finished (remote or local) new ready jobs will started.

    Upates the isRunningJobs flag.
*/
void
DistributedJobScheduler::Update()
{
    n_assert(this->isOpen);
    n_assert(this->isRunningJobs);

    // становится истинным, если локальное или удаленное задание завершено
    bool jobFinished = false;
    
    // сначала запустите задания инициализации, если они есть
    if (this->initializeJobs.Size()>0)
    {
        // обработать работу
        this->ProcessLocalJob(this->initializeJobs[0]);
        // вывод на печать
        if (this->printLocalJobOutput && this->initializeJobs[0]->HasOutputContent())
        {
            Console::Instance()->Print(
                "\n\n[Локальный вывод]\n\n%s\n\n",
                this->initializeJobs[0]->DequeueOutputContent().AsCharPtr());
        }
        // проверить состояние
        if(DistributedJob::Finished == this->initializeJobs[0]->GetCurrentState())
        {
            n_printf("[Локальное задание инициализации №%s] завершено\n",this->initializeJobs[0]->GetIdentifier().AsCharPtr());
            this->initializeJobs.EraseIndex(0);
        }
        else if(DistributedJob::JobError == this->initializeJobs[0]->GetCurrentState())
        {
            n_printf("[Локальное задание инициализации #%s] ОШИБКА\n",this->initializeJobs[0]->GetIdentifier().AsCharPtr());            
            this->initializeJobs.EraseIndex(0);
            Console::Instance()->Error("Не удалось выполнить локальное задание инициализации.");
        }
        // if all initial jobs are finished, set the jobFinished Flag,
        // поэтому новый StartReadyJobs называется...
        if(this->initializeJobs.Size()==0)
        {
            jobFinished = true;
        }
        else
        {
            // else return and process left initial jobs
            return;
        }
    }

    // прочитать выходные данные локальных заданий и поместить их в выходной поток
    // удалить завершенные вакансии из списка
    IndexT i=0;
    while (i<this->localRunningJobs.Size())
    {
        // обработка задания
        this->ProcessLocalJob(this->localRunningJobs[i]);
        // печать
        if (this->printLocalJobOutput && this->localRunningJobs[i]->HasOutputContent())
        {
            Console::Instance()->Print(
                "\n\n[Локальный вывод]\n\n%s\n\n",
                this->localRunningJobs[i]->DequeueOutputContent().AsCharPtr());
        }
        // проверить состояние
        if (DistributedJob::Finished == this->localRunningJobs[i]->GetCurrentState())
        {
            n_printf("[Локальное задание №%s] завершено\n",this->localRunningJobs[i]->GetIdentifier().AsCharPtr());
            this->localRunningJobs.EraseIndex(i);
            jobFinished = true;
        }
        else if (DistributedJob::JobError == this->localRunningJobs[i]->GetCurrentState())
        {
            n_printf("[Локальное задание №%s] НЕ выполнено\n",this->localRunningJobs[i]->GetIdentifier().AsCharPtr());            
            this->localRunningJobs.EraseIndex(i);
            jobFinished = true;
            Console::Instance()->Print("Не удалось выполнить локальное задание.");
        }
        else
        {
            i++;
        }
    }
    
    if (this->useRemoteServices)
    {
        // обновить службы удаленной работы
        IndexT s;
        for (s = 0; s < this->remoteServices.Size(); s++)
        {
            if (this->bannedRemoteServices.FindIndex(this->remoteServices[s]) == InvalidIndex)
            {
                if (RemoteJobService::ServiceError!=this->remoteServices[s]->GetServiceState())
                {
                    if (this->remoteServices[s]->IsOpen())
                    {
                        this->remoteServices[s]->Update();
                    }
                    
                    // справляться с готовыми работами
                    Util::Array<Util::Guid> finishedJobs = this->remoteServices[s]->GetFinishedJobGuids();
                    IndexT guid;
                    for (guid = 0; guid < finishedJobs.Size(); guid++)
                    {
                        IndexT i = 0;
                        while (i < this->remoteRunningJobs.Size())
                        {
                            if (this->remoteRunningJobs[i]->GetGuid() == finishedJobs[guid])
                            {
                                n_printf("[Удаленное задание №%s] завершено\n",this->remoteRunningJobs[i]->GetIdentifier().AsCharPtr());
                                this->remoteRunningJobs.EraseIndex(i);
                                jobFinished = true;
                            }
                            else
                            {
                                i++;
                            }
                        }
                    }
                    
                    // обрабатывать мягкие неудачные задания
                    Util::Array<Util::Guid> failedJobs = this->remoteServices[s]->GetFailedJobGuids();
                    for (guid = 0; guid < failedJobs.Size(); guid++)
                    {
                        IndexT i = 0;
                        while (i < this->remoteRunningJobs.Size())
                        {
                            if (this->remoteRunningJobs[i]->GetGuid() == failedJobs[guid])
                            {
                                // проверьте, находится ли задание в инициализации или финализации списка заданий
                                bool wasInitializeFinalizeJob = false;
                                IndexT j = 0;
                                while (i < this->remoteServices[s]->GetInitializeJobs().Size())
                                {
                                    if (this->remoteServices[s]->GetInitializeJobs()[j]->GetGuid() == this->remoteRunningJobs[i]->GetGuid())
                                    {
                                        wasInitializeFinalizeJob = true;
                                    }
                                    else
                                    {
                                        j++;
                                    }
                                }
                                j=0;
                                while (i < this->remoteServices[s]->GetFinalizeJobs().Size())
                                {
                                    if (this->remoteServices[s]->GetFinalizeJobs()[j]->GetGuid() == this->remoteRunningJobs[i]->GetGuid())
                                    {
                                        wasInitializeFinalizeJob = true;
                                    }
                                    else
                                    {
                                        j++;
                                    }
                                }
                                if (wasInitializeFinalizeJob)
                                {
                                    n_printf("[Задание удаленной инициализации/финализации №%s] Не удалось запустить. сбросить службу...\n",this->remoteRunningJobs[i]->GetIdentifier().AsCharPtr());
                                    Console::Instance()->Error("Задание удаленной инициализации или завершения находилось в состоянии ошибки.");
                                }
                                else
                                {
                                    n_printf("[Удаленное задание №%s] Не удалось запустить. Повторная попытка позже...\n",this->remoteRunningJobs[i]->GetIdentifier().AsCharPtr());
                                    this->ResetJobsFromService(this->remoteServices[s]);
                                    this->remoteServices[s]->RemoveFromFailedJobList(failedJobs[guid]);
                                }
                            }
                            else
                            {
                                i++;
                            }
                        }
                    }
                }
                else
                {
                    Console::Instance()->Warning(("Служба сообщила об ошибке:\n"+this->remoteServices[s]->GetErrorMessage()).AsCharPtr());
                    // Обработка служб в состоянии ошибки
                    Console::Instance()->Print("\nЗапрещена служба на %s.\n",this->remoteServices[s]->GetIpAddress().GetHostName().AsCharPtr());
                    this->DisableRunningService(this->remoteServices[s]);
                    // сообщите планировщику, что задание завершено, и будут запущены новые готовые задания
                    jobFinished = true;
                }
            }
            else
            {
                this->HandleBannedService(this->remoteServices[s]);
            }
        }
    }
    // начать новую работу, если какая-либо из них была завершена
    if(jobFinished)
    {
        this->StartReadyJobs();
    }
    
    // запустить задания завершения, если не осталось локальных и удаленных заданий
    if (
        this->finalizeJobs.Size()>0 &&
        this->localRunningJobs.Size() == 0 &&
        this->remoteRunningJobs.Size() == 0
        )
    {
        this->ProcessLocalJob(this->finalizeJobs[0]);
        // вывод на печать
        if (this->printLocalJobOutput && this->finalizeJobs[0]->HasOutputContent())
        {
            Console::Instance()->Print(
                "\n\n[Локальный вывод]\n\n%s\n\n",
                this->finalizeJobs[0]->DequeueOutputContent().AsCharPtr());
        }
        // проверить состояние
        if (DistributedJob::Finished == this->finalizeJobs[0]->GetCurrentState())
        {
            n_printf("[Локальное задание завершения #%s] завершено\n",this->finalizeJobs[0]->GetIdentifier().AsCharPtr());
            this->finalizeJobs.EraseIndex(0);
        }
        else if (DistributedJob::JobError == this->finalizeJobs[0]->GetCurrentState())
        {
            n_printf("[Локальное задание финализации №%s] НЕ СШЛО\n",this->finalizeJobs[0]->GetIdentifier().AsCharPtr());            
            this->finalizeJobs.EraseIndex(0);
            Console::Instance()->Error("Не удалось выполнить локальное задание завершения.");
        }
        // return and process finalize jobs until all are finished
        return;
    }
    
    // проверьте состояние isRunning
    if(
        this->HasRunningRemoteServices() ||
        this->localRunningJobs.Size() > 0 ||
        this->remoteRunningJobs.Size() > 0 ||
        this->finalizeJobs.Size() > 0 ||
        this->initializeJobs.Size() > 0)
    {
        this->isRunningJobs = true;
    }
    else
    {
        this->isRunningJobs = false;
    } 
}

//------------------------------------------------------------------------------
/**
    Работа еще продолжается?   
*/
bool
DistributedJobScheduler::HasActiveJobs()
{
    n_assert(this->isOpen);
    return this->isRunningJobs;
}

//------------------------------------------------------------------------------
/**
    Запускает новые задания из списка готовых заданий.
    Очищает список заданий инициализации/финализации.
*/
void
DistributedJobScheduler::StartReadyJobs()
{
    n_assert(this->isOpen);
    DistributedJobScheduler::StartReadyJobsLocal();
    if (this->useRemoteServices)
    {
        DistributedJobScheduler::StartReadyJobsRemote();
    }

    // На этом этапе должны быть запущены задания, если готовые задания ждут,
    // else this would cause an endless loop. Abort with an error if so.
    if (
            (
            this->localRunningJobs.Size() +
            this->remoteRunningJobs.Size() + 
            this->initializeJobs.Size()
            ) == 0 &&
        this->readyJobs.Size() > 0
        )
    {
        Console::Instance()->Error("Не найдена действующая служба для обработки заданий.\n");
    }

    this->isRunningJobs = true;   
}
//------------------------------------------------------------------------------
/**
    Добавляет готовые вакансии в локальный список вакансий   
*/
void
DistributedJobScheduler::StartReadyJobsLocal()
{  
    IndexT i = 0;
    while (i<this->readyJobs.Size() && this->initializeJobs.Size()==0)
    {
        // добавить местную работу
        bool addJob = false;
        bool skipUpdate = this->isRunningJobs;
        if(this->localRunningJobs.Size() < this->maxParallelLocalJob)
        {
            addJob = true;
            if (this->localRunningJobs.Size()>0)
            {
                // if a local job is already in the job list, skip the update process
                // каждой новой работы...
                skipUpdate = true;
                // добавляйте задания в один и тот же список только в том случае, если им нужна одна и та же редакция
                // и работать по тому же пути проекта
                if(
                    this->localRunningJobs[0]->GetRequiredRevision()!=this->readyJobs[i]->GetRequiredRevision() &&
                    this->localRunningJobs[0]->GetProjectDirectory()!=this->readyJobs[i]->GetProjectDirectory()
                    )
                {
                    addJob = false;
                }
            }
            if(DistributedJob::Ready != this->readyJobs[i]->GetCurrentState())
            {
                addJob = false;
            }
        }
        if (addJob)
        {            
            n_printf("[Локальное задание №%s] НАЧИНАЕТСЯ\n",this->readyJobs[i]->GetIdentifier().AsCharPtr());
            this->localRunningJobs.Append(this->readyJobs[i]);
            // проверить путь проекта к заданию
            this->readyJobs[i]->ValidateProjectPath();
            if(skipUpdate)
            {
                this->readyJobs[i]->SkipProjectDirectoryUpdate();
            }
            this->readyJobs.EraseIndex(i);
        }
        else
        {
            i++;
        }
    }
}

//------------------------------------------------------------------------------
/**
    Добавляет готовые задания в списки удаленных заданий 
*/
void
DistributedJobScheduler::StartReadyJobsRemote()
{
    SizeT addedJobCount;
    // проверить все услуги
    IndexT s;
    for (s = 0; s < this->remoteServices.Size(); s++)
    {
        // пропустить услугу, если она была запрещена
        if (this->bannedRemoteServices.FindIndex(this->remoteServices[s]) == InvalidIndex)
        {
            addedJobCount = 0;
            IndexT i = 0;
            // while there are ready jobs and free processors on the service
            while ( i < this->readyJobs.Size() && addedJobCount < this->remoteServices[s]->GetProcessorCount())
            {
                // становится истинным, если текущая служба может принимать задания
                bool addJob = false;
                // услуга подключена?
                if (RemoteJobService::Closed == this->remoteServices[s]->GetServiceState())
                {
                    this->remoteServices[s]->Open();
                    // подождите, пока служба не будет готова принимать задания или не произойдет сбой соединения
                    while (RemoteJobService::Connecting == this->remoteServices[s]->GetServiceState())
                    {
                        this->remoteServices[s]->Update();
                    }
                }
                // Доступна ли услуга у этого мастера
                if(
                    RemoteJobService::Ready == this->remoteServices[s]->GetServiceState() ||
                    RemoteJobService::ExclusiveReady == this->remoteServices[s]->GetServiceState())
                {
                    addJob = true;
                }
                else
                {
                    // Кажется, эта служба не может обработать ни одно задание, пропустите его...
                    break;
                }
                
                if(addJob)
                {
                    // отправить задание в сервис. Если не получилось, пропустите сервис...
                    if(this->remoteServices[s]->SendJob(this->readyJobs[i]))
                    {
                        n_printf("[Удаленное задание №%s] НАЧИНАЕТСЯ\n",this->readyJobs[i]->GetIdentifier().AsCharPtr());
                        this->remoteRunningJobs.Append(this->readyJobs[i]);
                        this->readyJobs.EraseIndex(i);
                        addedJobCount++;
                    }
                    else
                    {
                        i++;
                    }
                }
                else
                {
                    // пропустить обслуживание, если оно не может принимать задания
                    i++;
                }   
            }
            
            // запустить сервис, если какое-либо задание было прикреплено
            if (
                    (
                    RemoteJobService::Ready == this->remoteServices[s]->GetServiceState() ||
                    RemoteJobService::ExclusiveReady == this->remoteServices[s]->GetServiceState()
                    ) &&
                addedJobCount > 0
                )
            {
                // создавать клоны списков заданий инициализации/финализации
                Util::Array<Ptr<DistributedJob>> initJobs;
                Util::Array<Ptr<DistributedJob>> finaJobs;
                IndexT idx;
                for(idx = 0; idx < this->remoteInitializeJobs.Size(); idx++)
                {
                    initJobs.Append(this->remoteInitializeJobs[idx]->Clone());
                }
                for(idx = 0; idx < this->remoteFinalizeJobs.Size(); idx++)
                {
                    finaJobs.Append(this->remoteFinalizeJobs[idx]->Clone());
                }
                // добавить задания инициализации/завершения в список удаленно выполняемых заданий
                this->remoteRunningJobs.AppendArray(initJobs);
                this->remoteRunningJobs.AppendArray(finaJobs);
                // добавить задания инициализации/завершения в удаленную службу
                this->remoteServices[s]->SetInitializeJobs(initJobs);
                this->remoteServices[s]->SetFinalizeJobs(finaJobs);
                // сообщить службе, чтобы она выполнила все свои задания
                this->remoteServices[s]->RunJobs();
            }
        }
    }
}

//------------------------------------------------------------------------------
/**
    Обработка локального задания. В зависимости от состояния задания это будет делать
    разные вещи.
*/
void
DistributedJobScheduler::ProcessLocalJob(const Ptr<DistributedJob> & job)
{
    if(DistributedJob::Ready == job->GetCurrentState())
    {
        job->ValidateProjectPath();
    }
    else if(DistributedJob::ValidProjectPath == job->GetCurrentState())
    {
        // Проверьте, выполняют ли другие локальные задания обновление svn.
        // или в данный момент работаете над каталогом проекта.
        // Если нет, то запустите обновление задания по SVN.
        bool isAnyUpdating = false;
        IndexT i;
        for (i = 0; i<this->localRunningJobs.Size(); i++)
        {
            if(
                DistributedJob::Updating == this->localRunningJobs[i]->GetCurrentState() ||
                DistributedJob::Working == this->localRunningJobs[i]->GetCurrentState()
                )
            {
                isAnyUpdating = true;
                break;
            }
        }
        if(!isAnyUpdating)
        {
            job->UpdateProjectDirectory();
        }
    }
    else if(DistributedJob::Updating == job->GetCurrentState())
    {
        job->Update();
    }
    else if(DistributedJob::UpToDate == job->GetCurrentState())
    {
        // Проверьте, выполняют ли другие локальные задания обновление svn.
        // Если нет, то начните работу
        bool isAnyUpdating = false;
        IndexT i;
        for (i = 0; i<this->localRunningJobs.Size(); i++)
        {
            if(DistributedJob::Updating == this->localRunningJobs[i]->GetCurrentState())
            {
                isAnyUpdating = true;
                break;
            }
        }
        if(!isAnyUpdating)
        {
            job->RunJob();
        }
    }
    else if(DistributedJob::Working == job->GetCurrentState())
    {
        job->Update();
    }
}

//------------------------------------------------------------------------------
/**
    Возвращает true, если какой-либо удаленный сервис все еще занят.
*/
bool
DistributedJobScheduler::HasRunningRemoteServices()
{
    bool found = false;
    IndexT si;
    for(si = 0; si < this->remoteServices.Size(); si++)
    {
        if (this->bannedRemoteServices.FindIndex(this->remoteServices[si]) == InvalidIndex)
        {
            found |= (this->remoteServices[si]->GetServiceState() == RemoteJobService::Busy);
        }
    }
    return found;
}

//------------------------------------------------------------------------------
/**
    Отправьте службу в черный список и обработайте ее, работающую в данный момент.
    рабочие места. 
*/
void
DistributedJobScheduler::DisableRunningService(const Ptr<RemoteJobService> & service)
{
    // проверьте, заблокирован ли сервис уже
    n_assert(this->bannedRemoteServices.FindIndex(service)==InvalidIndex);
    
    // добавить услугу в черный список
    this->bannedRemoteServices.Append(service);
    
    // сбросить задания из сервиса
    this->ResetJobsFromService(service);
    
    // перезагрузите сервис и пометьте его как грязный
    service->SetDirty();
    service->Reset();
}

//------------------------------------------------------------------------------
/**
    Sends a message to the given service and checks for it's current state.
    If the service reported, that it has cleaned itself up, then remove it from
    the banned service list.
*/
void
DistributedJobScheduler::HandleBannedService(const Ptr<RemoteJobService> & service)
{
    // проверьте, сообщила ли служба, что все очищено.
    if (service->HasCleanedUp())
    {
        this->bannedRemoteServices.EraseIndex(this->bannedRemoteServices.FindIndex(service));
    }
    else
    {
        if (service->GetServiceState() == RemoteJobService::Closed)
        {
            service->Open();
            // дождитесь, пока служба будет готова принимать команды или соединение не будет установлено
            while (RemoteJobService::Connecting == service->GetServiceState())
            {
                service->Update();
            }
        }
        if(service->IsOpen())
        {
            service->Update();
            if (
                RemoteJobService::Ready == service->GetServiceState() ||
                RemoteJobService::ExclusiveReady == service->GetServiceState())
            {
                // отправить на сервис «вас забанили».
                service->SendBannedInfo();
            }
        }            
    }
}

//------------------------------------------------------------------------------
/**
    Перехватите все задания сервиса и повторно добавьте их в список готовых заданий.
*/
void
DistributedJobScheduler::ResetJobsFromService(const Ptr<RemoteJobService> & service)
{
    // удалить все задания инициализации, завершения и стандартные задания из списка удаленных заданий
    Array<Ptr<DistributedJob>> removeJobs;
    removeJobs.AppendArray(service->GetUncompleteJobs());
    removeJobs.AppendArray(service->GetInitializeJobs());
    removeJobs.AppendArray(service->GetFinalizeJobs());
    IndexT remIdx;
    IndexT foundIndex;
    for (remIdx = 0; remIdx < removeJobs.Size(); remIdx++)
    {
        foundIndex = this->remoteRunningJobs.FindIndex(removeJobs[remIdx]);
        if(foundIndex != InvalidIndex)
        {
            this->remoteRunningJobs.EraseIndex(foundIndex);
        }
    }

    // получить все руководства по вакансиям, отправленные в эту службу, и переназначить их
    // готовый список вакансий.
    Console::Instance()->Print("Повторно добавить %i заданий...\n",service->GetUncompleteJobs().Size());
    this->readyJobs.AppendArray(service->GetUncompleteJobs());
}

} // пространство имен DistributedTools