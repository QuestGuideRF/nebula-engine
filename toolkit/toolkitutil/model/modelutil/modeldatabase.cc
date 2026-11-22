//------------------------------------------------------------------------------
//  importdatabase.cc
//  (C) 2012-2016 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "foundation/stdneb.h"
#include "model/modelutil/modeldatabase.h"
#include "io/uri.h"
#include "util/string.h"
#include "io/ioserver.h"

using namespace IO;
using namespace Util;
namespace ToolkitUtil
{

__ImplementSingleton(ModelDatabase);
__ImplementClass(ToolkitUtil::ModelDatabase, 'IMDB', Core::RefCounted);

//------------------------------------------------------------------------------
/**
*/
ModelDatabase::ModelDatabase() :
    isOpen(false)
{
    __ConstructSingleton;
}

//------------------------------------------------------------------------------
/**
*/
ModelDatabase::~ModelDatabase()
{
    __DestructSingleton;
}

//------------------------------------------------------------------------------
/**
*/
void 
ModelDatabase::Open()
{
    n_assert(!this->IsOpen());
    this->isOpen = true;
}

//------------------------------------------------------------------------------
/**
*/
void 
ModelDatabase::Close()
{
    n_assert(this->IsOpen());
    IndexT i;
    for (i = 0; i < this->modelAttributes.Size(); i++)
    {
        this->modelAttributes.ValueAtIndex(i)->Clear();
    }
    this->modelAttributes.Clear();
    for (i = 0; i < this->modelConstants.Size(); i++)
    {
        this->modelConstants.ValueAtIndex(i)->Clear();
    }
    this->modelConstants.Clear();
    for (i = 0; i < this->modelPhysics.Size(); i++)
    {
        this->modelPhysics.ValueAtIndex(i)->Clear();
    }
    this->modelPhysics.Clear();
    this->isOpen = false;
}


//------------------------------------------------------------------------------
/**
*/

void 
ModelDatabase::LoadAttributes(const Util::String & folder)
{
    
    Array<String> files = IoServer::Instance()->ListFiles(folder, "*.атрибуты");
    for (IndexT fileIndex = 0; fileIndex < files.Size(); fileIndex++)
    {

        String modelName = files[fileIndex];

        // создать URI
        String path;
        path.Format("%s%s", folder.AsCharPtr(), modelName.AsCharPtr());


        modelName.StripFileExtension();
        Util::String category = path.ExtractLastDirName();

        modelName = category + "/" + modelName;


        Ptr<ModelAttributes> attrs = ModelAttributes::Create();

        // установить имя атрибута
        attrs->SetName(modelName);
        // создать поток
        Ptr<Stream> file = IoServer::Instance()->CreateStream(path);
        attrs->Load(file);

        this->modelAttributes.Add(modelName, attrs);
    }
    Array<String> cfiles = IoServer::Instance()->ListFiles(folder, "*.константы");
    for (IndexT fileIndex = 0; fileIndex < cfiles.Size(); fileIndex++)
    {

        String modelName = cfiles[fileIndex];

        // создать URI
        String path;
        path.Format("%s/%s", folder.AsCharPtr(), modelName.AsCharPtr());


        modelName.StripFileExtension();
        Util::String category = path.ExtractLastDirName();
        modelName = category + "/" + modelName;


        Ptr<ModelConstants> attrs = ModelConstants::Create();

        // установить имя атрибута
        attrs->SetName(modelName);
        // создать поток
        Ptr<Stream> file = IoServer::Instance()->CreateStream(path);
        attrs->Load(file);

        this->modelConstants.Add(modelName, attrs);
    }
}

//------------------------------------------------------------------------------
/**
*/
Ptr<ModelAttributes>
ModelDatabase::LookupAttributes(const Util::String& name, bool reload)
{
    if (!this->modelAttributes.Contains(name))
    {
        // создать новые атрибуты модели
        Ptr<ModelAttributes> attrs = ModelAttributes::Create();

        // установить имя атрибута
        attrs->SetName(name);

        // проверьте, существует ли файл, если да, загрузите его, в противном случае используйте базовый объект
        if (this->AttributesExist(name))
        {
            // создать URI
            String path;
            path.Format("src:assets/%s.attributes", name.AsCharPtr());

            // создать поток
            Ptr<Stream> file = IoServer::Instance()->CreateStream(path);
            attrs->Load(file);
        }

        // наконец, добавь его в список
        this->modelAttributes.Add(name, attrs);

        // return attributes
        return attrs;
    }
    else if (reload)
    {
        Ptr<ModelAttributes> attrs = this->modelAttributes[name];

        // четкие атрибуты
        attrs->Clear();

        // проверьте, существует ли файл, если да, загрузите его, в противном случае используйте базовый объект
        if (this->AttributesExist(name))
        {
            // создать URI
            String path;
            path.Format("src:assets/%s.attributes", name.AsCharPtr());

            // создать поток
            Ptr<Stream> file = IoServer::Instance()->CreateStream(path);
            attrs->Load(file);
        }

        return attrs;
    }
    else
    {
        return this->modelAttributes[name];
    }
}

//------------------------------------------------------------------------------
/**
*/
bool
ModelDatabase::AttributesExist(const Util::String& name)
{
    // форматировать файл
    String file;
    file.Format("src:assets/%s.attributes", name.AsCharPtr());

    // в основном проверьте, имеет ли файловая система запрошенный атрибут
    if (IoServer::Instance()->FileExists(file))
    {
        return true;
    }

    return false;
}

//------------------------------------------------------------------------------
/**
*/
const Util::String&
ModelDatabase::GetAttributesName(const Ptr<ModelAttributes>& attrs)
{
    IndexT index = this->modelAttributes.ValuesAsArray().FindIndex(attrs);
    n_assert(index != InvalidIndex);
    return this->modelAttributes.KeysAsArray()[index];
}

//------------------------------------------------------------------------------
/**
*/
Ptr<ModelPhysics>
ModelDatabase::LookupPhysics(const Util::String& name, bool reload)
{
    if (!this->modelPhysics.Contains(name))
    {
        // создать новые атрибуты модели
        Ptr<ModelPhysics> phys = ModelPhysics::Create();

        // установить имя атрибута
        phys->SetName(name);

        // проверьте, существует ли файл, если да, загрузите его, в противном случае используйте базовый объект
        if (this->PhysicsExist(name))
        {
            // создать URI
            String path;
            path.Format("src:assets/%s.физика", name.AsCharPtr());

            // создать поток
            Ptr<Stream> file = IoServer::Instance()->CreateStream(path);
            phys->Load(file);
        }

        // наконец, добавь его в список
        this->modelPhysics.Add(name, phys);

        // return attributes
        return phys;
    }
    else if (reload)
    {
        Ptr<ModelPhysics> phys = this->modelPhysics[name];

        // четкие атрибуты
        phys->Clear();

        // проверьте, существует ли файл, если да, загрузите его, в противном случае используйте базовый объект
        if (this->PhysicsExist(name))
        {
            // создать URI
            String path;
            path.Format("src:assets/%s.физика", name.AsCharPtr());

            // создать поток
            Ptr<Stream> file = IoServer::Instance()->CreateStream(path);
            phys->Load(file);
        }

        return phys;
    }
    else
    {
        return this->modelPhysics[name];
    }
}

//------------------------------------------------------------------------------
/**
*/
bool
ModelDatabase::PhysicsExist(const Util::String& name)
{
    // форматировать файл
    String file;
    file.Format("src:assets/%s.физика", name.AsCharPtr());

    // в основном проверьте, имеет ли файловая система запрошенный атрибут
    if (IoServer::Instance()->FileExists(file))
    {
        return true;
    }

    return false;
}

//------------------------------------------------------------------------------
/**
*/
const Util::String&
ModelDatabase::GetPhysicsName(const Ptr<ModelPhysics>& attrs)
{
    IndexT index = this->modelPhysics.ValuesAsArray().FindIndex(attrs);
    n_assert(index != InvalidIndex);
    return this->modelPhysics.KeysAsArray()[index];
}

//------------------------------------------------------------------------------
/**
*/
Ptr<ModelConstants>
ModelDatabase::LookupConstants(const Util::String& name, bool reload)
{
    if (!this->modelConstants.Contains(name))
    {
        // создать новые атрибуты модели
        Ptr<ModelConstants> constants = ModelConstants::Create();

        // установить имя атрибута
        constants->SetName(name);

        // проверьте, существует ли файл, если да, загрузите его, в противном случае используйте базовый объект
        if (this->AttributesExist(name))
        {
            // создать URI
            String path;
            path.Format("src:assets/%s.constants", name.AsCharPtr());

            // создать поток
            Ptr<Stream> file = IoServer::Instance()->CreateStream(path);
            constants->Load(file);
        }

        // наконец, добавь его в список
        this->modelConstants.Add(name, constants);

        // return attributes
        return constants;
    }
    else if (reload)
    {
        Ptr<ModelConstants> constants = this->modelConstants[name];

        // четкие атрибуты
        constants->Clear();

        // проверьте, существует ли файл, если да, загрузите его, в противном случае используйте базовый объект
        if (this->AttributesExist(name))
        {
            // создать URI
            String path;
            path.Format("src:assets/%s.constants", name.AsCharPtr());

            // создать поток
            Ptr<Stream> file = IoServer::Instance()->CreateStream(path);
            constants->Load(file);
        }

        return constants;
    }
    else
    {
        return this->modelConstants[name];
    }
}

//------------------------------------------------------------------------------
/**
*/
bool
ModelDatabase::ConstantsExist(const Util::String& name)
{
    // форматировать файл
    String file;
    file.Format("src:assets/%s.constants", name.AsCharPtr());

    // в основном проверьте, имеет ли файловая система запрошенный атрибут
    if (IoServer::Instance()->FileExists(file))
    {
        return true;
    }

    return false;
}

//------------------------------------------------------------------------------
/**
*/
const Util::String&
ModelDatabase::GetConstantsName(const Ptr<ModelConstants>& constants)
{
    IndexT index = this->modelConstants.ValuesAsArray().FindIndex(constants);
    n_assert(index != InvalidIndex);
    return this->modelConstants.KeysAsArray()[index];
}

} // пространство имен ToolkitUtil