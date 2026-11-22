//------------------------------------------------------------------------------
//  modelphysics.cc
//  (C) 2012 Густав Стербрант
//------------------------------------------------------------------------------
#include "foundation/stdneb.h"
#include "modelphysics.h"

#include "io/stream.h"
#include "io/ioserver.h"
#include "io/xmlwriter.h"
#include "io/xmlreader.h"

using namespace Util;
using namespace IO;


namespace ToolkitUtil
{
__ImplementClass(ToolkitUtil::ModelPhysics, 'MDPH', Core::RefCounted);

//------------------------------------------------------------------------------
/**
*/
ModelPhysics::ModelPhysics() : 
    physicsMode(ToolkitUtil::UseBoundingBox),
    meshMode(Physics::MeshTopology_Convex),
    material("default")
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
ModelPhysics::~ModelPhysics()
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
void 
ModelPhysics::SetMeshMode(Physics::MeshTopology flags)
{
    this->meshMode = flags;
}

//------------------------------------------------------------------------------
/**
*/
Physics::MeshTopology
ModelPhysics::GetMeshMode()
{
    return this->meshMode;
}

//------------------------------------------------------------------------------
/**
*/
void 
ModelPhysics::SetExportMode(PhysicsExportMode flags)
{
    this->physicsMode = flags;
}

//------------------------------------------------------------------------------
/**
*/
PhysicsExportMode 
ModelPhysics::GetExportMode()
{
    return this->physicsMode;
}

//------------------------------------------------------------------------------
/**
*/
void 
ModelPhysics::Save(const Ptr<IO::Stream>& stream)
{
    n_assert(stream.isvalid());

    // установите правильный доступ и откройте поток
    stream->SetAccessMode(Stream::WriteAccess);
    if (stream->Open())
    {
        // создать XML-писатель
        Ptr<XmlWriter> writer = XmlWriter::Create();
        writer->SetStream(stream);
        writer->Open();

        // сначала напишите закрывающий тег
        writer->BeginNode("Nebula");

        // установить версию
        writer->SetInt("version", ModelPhysics::Version);

        // начни с написания флагов
        writer->BeginNode("Options");

        // написать варианты        
        writer->SetInt("exportMode", this->physicsMode);
        writer->SetInt("meshMode", this->meshMode); 
        writer->SetString("material", this->material);

        if(this->GetExportMode() == UsePhysics)
        {
            writer->SetString("physicsMesh",this->GetPhysicsMesh());
        }

        // конечный узел опций
        writer->EndNode();  

        // конец узла Nebula 3
        writer->EndNode();

        // закончить писать
        writer->Close();
        stream->Close();
    }
}

//------------------------------------------------------------------------------
/**
*/
void 
ModelPhysics::Load(const Ptr<IO::Stream>& stream)
{
    n_assert(stream.isvalid());

    // установите правильный режим доступа и откройте поток
    stream->SetAccessMode(Stream::ReadAccess);
    if (stream->Open())
    {
        // создать программу чтения XML
        Ptr<XmlReader> reader = XmlReader::Create();
        reader->SetStream(stream);
        reader->Open();

        // получить версию
        int version = reader->GetInt("version");

        // прекратите загрузку, если версия неправильная
        if (version != ModelPhysics::Version)
        {
            stream->Close();
            return;
        }

        // затем убедитесь, что у нас есть тег параметров
        n_assert2(reader->SetToFirstChild("Options"), "ПОВРЕЖДЕН ФАЙЛ .физики!: Первый тег должен быть «Параметры»!");

        // теперь загрузим параметры
        this->physicsMode = (ToolkitUtil::PhysicsExportMode)reader->GetInt("exportMode");
        this->meshMode = (Physics::MeshTopology)reader->GetInt("meshMode");
        this->material = reader->GetOptString("material", "default");
        if (this->material.IsEmpty())
        {
            this->material = "default";
        }

        if(this->GetExportMode() == UsePhysics)
        {
            this->physicsMesh = reader->GetString("physicsMesh");
        }
    
        // закончить писать
        reader->Close();
        stream->Close();
    }
}

//------------------------------------------------------------------------------
/**
*/
void 
ModelPhysics::Clear()
{
    // empty
}

} // пространство имен ToolkitUtil