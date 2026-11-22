//------------------------------------------------------------------------------
//  shareddircreator.cc
//  (C) 2009 г. РадонЛабс ГмбХ
//  (C) 2013–2016 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "foundation/stdneb.h"
#include "distributedtools/shareddircreator.h"
#include "distributedtools/shareddirfilesystem.h"
#include "distributedtools/shareddirftp.h"

using namespace Util;
using namespace IO;

namespace DistributedTools
{
    __ImplementClass(DistributedTools::SharedDirCreator, 'SDCR', Core::RefCounted);
    __ImplementSingleton(DistributedTools::SharedDirCreator);
//------------------------------------------------------------------------------
/**
    Конструктор 
*/
SharedDirCreator::SharedDirCreator()
{
    __ConstructSingleton;
}

//------------------------------------------------------------------------------
/**
    Деструктор  
*/
SharedDirCreator::~SharedDirCreator()
{
}

//------------------------------------------------------------------------------
/**
    создает объект SharedControl, который является либо SharedDirFileSystem
    или SharedDirFTP, в зависимости от заданного пути.
*/
Ptr<SharedDirControl>
SharedDirCreator::CreateSharedControlObject(const Util::String & path)
{
    Guid guid;
    guid.Generate();
    return this->CreateSharedControlObject(path, guid);
}

//------------------------------------------------------------------------------
/**
    creates a SharedControl object (from guid), which is either a
    SharedDirFileSystem or a SharedDirFTP, depending on the given path.
*/
Ptr<SharedDirControl>
SharedDirCreator::CreateSharedControlObject(const Util::String & path, const Util::Guid & guid)
{
    Ptr<SharedDirControl> result;
    URI sharedURI;
    sharedURI.Set(path);
    if(sharedURI.Scheme() == "ftp")
    {
        n_assert(this->projectInfo.HasAttr("FTPUserName")
            && this->projectInfo.HasAttr("FTPPassword")
            && this->projectInfo.HasAttr("FTPExeLocation"));

        Ptr<SharedDirFTP> obj = SharedDirFTP::Create();
        obj->SetPath(path);
        obj->SetGuid(guid);
        obj->SetUserName(this->projectInfo.GetAttr("FTPUserName"));
        obj->SetPassword(this->projectInfo.GetAttr("FTPPassword"));
        obj->SetExeLocation(this->projectInfo.GetAttr("FTPExeLocation"));
        result = obj;
        obj = nullptr;
    }
    else
    {
        Ptr<SharedDirFileSystem> obj = SharedDirFileSystem::Create();
        obj->SetPath(path);
        obj->SetGuid(guid);
        result = obj;
        obj = nullptr;
    }
    return result;
}

} // пространство имен DistributedTools
