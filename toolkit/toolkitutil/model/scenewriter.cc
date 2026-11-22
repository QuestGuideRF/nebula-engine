//------------------------------------------------------------------------------
//  @file Scenewriter.cc
//  @copyright (C) 2022 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "foundation/stdneb.h"
#include "scenewriter.h"
#include "io/memorystream.h"
#include "io/ioserver.h"
#include "model/modelutil/modeldatabase.h"
#include "model/modelutil/modelconstants.h"
#include "model/modelutil/modelbuilder.h"
#include "util/crc.h"

namespace ToolkitUtil
{

using namespace IO;
using namespace Util;

//------------------------------------------------------------------------------
/**
*/
void
SceneWriter::GenerateModels(
    const Util::String& basePath
    , const Scene* scene
    , const Platform::Code platform
    , const Util::Array<SceneNode*>& graphicsNodes
    , const Util::Array<SceneNode*>& physicsNodes
    , const Util::Array<SceneNode*>& characterNodes
    , const Util::String& physicsMeshResource
    , const ToolkitUtil::ExportFlags& flags
)
{
    n_assert(scene != nullptr);

    // убедитесь, что каталог категорий существует
    IO::IoServer::Instance()->CreateDirectory(basePath);

    // извлечь путь к файлу из пути экспорта
    String path = basePath + scene->GetName() + ".xml";
    String file = scene->GetName();
    String category = path.ExtractLastDirName();

    // объединить файл и категорию в одно имя
    String fileCat = category + "/" + scene->GetName();

    // получить постоянный набор моделей
    Ptr<ModelConstants> constants = ModelDatabase::Instance()->LookupConstants(fileCat);

    // получить атрибуты модели
    Ptr<ModelAttributes> attributes = ModelDatabase::Instance()->LookupAttributes(fileCat);

    // получить физику
    Ptr<ModelPhysics> physics = ModelDatabase::Instance()->LookupPhysics(fileCat);

    if (!physicsMeshResource.IsEmpty())
    {
        physics->SetPhysicsMesh(physicsMeshResource);
    }

    // четкие константы
    constants->Clear();

    // Пройдитесь по ограничивающим рамкам и получите ограничивающую рамку нашей сцены.
    Math::bbox globalBox;
    IndexT meshIndex;
    globalBox.begin_extend();
    for (meshIndex = 0; meshIndex < graphicsNodes.Size(); meshIndex++)
    {
        const SceneNode* mesh = graphicsNodes[meshIndex];
        globalBox.extend(mesh->base.boundingBox);
    }
    globalBox.end_extend();

    // установить глобальный ящик для констант
    constants->SetGlobalBoundingBox(globalBox);

    CreateModel(
        file
        , scene
        , category
        , constants
        , attributes
        , physics
        , graphicsNodes
        , physicsNodes
        , characterNodes
    );

    String constantsFile;
    constantsFile.Format("src:assets/%s/%s.constants", category.AsCharPtr(), file.AsCharPtr());

    // update
    UpdateConstants(constantsFile, constants);

    // форматировать файл атрибутов
    String attributesFile;
    attributesFile.Format("src:assets/%s/%s.attributes", category.AsCharPtr(), file.AsCharPtr());

    // update
    UpdateAttributes(attributesFile, attributes);

    // форматировать файл атрибутов
    String physicsFile;
    physicsFile.Format("src:assets/%s/%s.физика", category.AsCharPtr(), file.AsCharPtr());

    // update
    UpdatePhysics(physicsFile, physics);

    // Как только мы закончим обновление промежуточных XML-файлов, нам следует экспортировать их в наш двоичный файл N3.
    Ptr<ModelBuilder> modelBuilder = ModelBuilder::Create();

    // устанавливать константы и атрибуты
    modelBuilder->SetConstants(constants);
    modelBuilder->SetAttributes(attributes);
    modelBuilder->SetPhysics(physics);

    // формат названия модели
    String destination;
    destination.Format("mdl:%s/%s.n3", category.AsCharPtr(), file.AsCharPtr());

    // сохранить файл
    modelBuilder->SaveN3(destination, platform);

    // сохранить физику
    destination.Format("phys:%s/%s.actor", category.AsCharPtr(), file.AsCharPtr());
    modelBuilder->SaveN3Physics(destination, platform);
}

//------------------------------------------------------------------------------
/**
*/
void 
SetupDefaultState(
    const Util::String& nodePath
    , const Util::String& nodeMaterial
    , const Ptr<ModelAttributes>& attributes    
    , bool skinned = false
)
{
    // получить состояние из атрибутов
    State state;

    // получить состояние, если оно существует
    if (attributes->HasState(nodePath))
    {
        state = attributes->GetState(nodePath);
    }

    // Если в узле есть материал, который уже является поверхностью, используйте его вместо создания материала по умолчанию.
    Util::String surfacePath = nodeMaterial;
    IO::URI uri = surfacePath;
    if (!surfacePath.EndsWithString(".sur"))
    {
        surfacePath = Util::String::Sprintf("вкл.:%s.вкл.", nodeMaterial.AsCharPtr());
    }
    if (IO::IoServer::Instance()->FileExists(surfacePath) || surfacePath.BeginsWithString("syssur:"))
    {
        state.material = surfacePath;
    }
    else if (!state.material.IsValid())
    {
        state.material = skinned ? "syssur:placeholder_skinned.sur" : "syssur:placeholder.sur";
    }
    else if (!state.material.EndsWithString(".sur")) // Временно для исправления всех плохих заданий по материалам
    {
        state.material = Util::String::Sprintf("%s.sur", state.material.AsCharPtr());
    }
    
    // установить состояние атрибутов
    attributes->SetState(nodePath, state);
}

//------------------------------------------------------------------------------
/**
*/
void 
SceneWriter::CreateModel(
    const Util::String& file
    , const Scene* scene
    , const Util::String& category
    , const Ptr<ModelConstants>& constants
    , const Ptr<ModelAttributes>& attributes
    , const Ptr<ModelPhysics>& physics
    , const Util::Array<SceneNode*>& graphicsNodes
    , const Util::Array<SceneNode*>& physicsNodes
    , const Util::Array<SceneNode*>& characterNodes
)
{
    // форматировать анимационный ресурс
    String animRes;
    animRes.Format("ани:%s/%s.nax", category.AsCharPtr(), file.AsCharPtr());

    // скелетный ресурс формата
    String skeletonRes;
    skeletonRes.Format("ske:%s/%s.nsk", category.AsCharPtr(), file.AsCharPtr());

    // создать имя сетки
    String physicsMeshRes;
    physicsMeshRes.Format("phymsh:%s/%s_ph.nvx", category.AsCharPtr(), scene->GetName().AsCharPtr());

    // создать имя сетки
    String meshResource;
    meshResource.Format("msh:%s/%s.nvx", category.AsCharPtr(), scene->GetName().AsCharPtr());

    IndexT i;
    for (i = 0; i < graphicsNodes.Size(); i++)
    {
        // получить узел сетки
        const SceneNode* mesh = graphicsNodes[i];

        // создать полный путь к узлу
        String nodePath;
        nodePath.Format("корень/%s", mesh->base.name.AsCharPtr());

        // создать и добавить узел формы
        if (mesh->base.isSkin)
        {
            // Каждый скин имеет набор фрагментов, которые следует рассматривать как серию вызовов отрисовки.
            // в отличие от набора отдельных узлов модели, для чего и предназначен SkinSetNode.
            ModelConstants::SkinSetNode skinSetNode;
            skinSetNode.name = mesh->base.name;
            skinSetNode.path = nodePath;

            for (IndexT j = 0; j < mesh->skin.skinFragments.Size(); j++)
            {
                ModelConstants::SkinNode skinNode;
                skinNode.meshResource = meshResource;
                skinNode.meshIndex = mesh->mesh.meshIndex;
                String skinPath;
                skinNode.path.Format("%s/фрагмент_%d", nodePath.AsCharPtr(), j);
                skinNode.useLOD = mesh->mesh.lodIndex != InvalidIndex;
                if (skinNode.useLOD)
                {
                    skinNode.LODMax = mesh->mesh.maxLodDistance;
                    skinNode.LODMin = mesh->mesh.minLodDistance;
                }
                skinNode.transform.position = mesh->base.translation;
                skinNode.transform.rotation = mesh->base.rotation;
                skinNode.transform.scale = mesh->base.scale;
                skinNode.boundingBox = mesh->base.boundingBox;
                skinNode.name = mesh->base.name;
                skinNode.primitiveGroupIndex = mesh->skin.skinFragments[j];
                skinNode.fragmentJoints = mesh->skin.jointLookup[j].KeysAsArray();

                skinSetNode.skinFragments.Append(skinNode);

                SetupDefaultState(skinNode.path, Util::String::Sprintf("%s/%s/%s", category.AsCharPtr(), file.AsCharPtr(), mesh->mesh.material.AsCharPtr()), attributes, true);
            }

            constants->AddSkinSetNode(skinSetNode);
        }
        else
        {
            ModelConstants::ShapeNode shapeNode;
            shapeNode.meshResource = meshResource;
            shapeNode.meshIndex = mesh->mesh.meshIndex;
            shapeNode.path = nodePath;
            shapeNode.useLOD = mesh->mesh.lodIndex != InvalidIndex;
            if (shapeNode.useLOD)
            {
                shapeNode.LODMax = mesh->mesh.maxLodDistance;
                shapeNode.LODMin = mesh->mesh.minLodDistance;
            }
            shapeNode.transform.position = mesh->base.translation;
            shapeNode.transform.rotation = mesh->base.rotation;
            shapeNode.transform.scale = mesh->base.scale;
            shapeNode.boundingBox = mesh->base.boundingBox;
            shapeNode.name = mesh->base.name;
            shapeNode.primitiveGroupIndex = mesh->mesh.groupId;

            // добавить к константам
            constants->AddShapeNode(shapeNode);

            Util::String nodeMaterial = mesh->mesh.material;

            // Если материал является материалом .sur, мы передаем его как есть.
            if ((nodeMaterial.Length() <= 4) || (!nodeMaterial.IsEmpty() && !nodeMaterial.EndsWithString(".sur")))
            {
                nodeMaterial = Util::String::Sprintf("%s/%s/%s", category.AsCharPtr(), file.AsCharPtr(), nodeMaterial.AsCharPtr());
            }

            SetupDefaultState(shapeNode.path, nodeMaterial, attributes);
        }
    }

    for (i = 0; i < characterNodes.Size(); i++)
    {
        // создать узел персонажа
        ModelConstants::CharacterNode characterNode;
        characterNode.skeletonResource = skeletonRes;
        characterNode.skeletonIndex = characterNodes[i]->skeleton.skeletonIndex;
        characterNode.animationResource = animRes;
        characterNode.animationIndex = characterNodes[i]->anim.animIndex;
        characterNode.name = characterNodes[i]->base.name;

        // добавить символьный узел в константы
        constants->AddCharacterNode(characterNode.name, characterNode);
    }

    for (i = 0; i < physicsNodes.Size(); i++)
    {
        // получить сетку
        const SceneNode* mesh = physicsNodes[i];

        // создать полный путь к узлу
        String nodePath;
        nodePath.Format("корень/%s", mesh->base.name.AsCharPtr());

        // создать узел физики
        ModelConstants::PhysicsNode node;
        node.mesh = physicsMeshRes;
        node.path = nodePath;
        node.transform.position = mesh->base.translation;
        node.transform.rotation = mesh->base.rotation;
        node.transform.scale = mesh->base.scale;
        node.primitiveGroupIndex = mesh->mesh.groupId;
        node.name = mesh->base.name;

        // добавить к константам
        constants->AddPhysicsNode(mesh->base.name, node);
    }
}

//------------------------------------------------------------------------------
/**
*/
void 
SceneWriter::UpdateConstants(const Util::String & file, const Ptr<ToolkitUtil::ModelConstants>& constants)
{
    // создать поток
    Ptr<Stream> stream = IoServer::Instance()->CreateStream(file);
    stream->SetAccessMode(Stream::ReadAccess);
    stream->Open();

    if (stream->IsOpen())
    {
        // создать поток памяти
        Ptr<Stream> memStream = MemoryStream::Create();

        // теперь сохрани константы
        constants->Save(memStream);

        // прочитать из файла
        uchar* origData = (uchar*)stream->Map();
        SizeT origSize = stream->GetSize();

        // откройте мемстрим еще раз
        memStream->SetAccessMode(Stream::ReadAccess);
        memStream->Open();

        // получить данные из памяти
        uchar* newData = (uchar*)memStream->Map();
        SizeT newSize = memStream->GetSize();

        // создать CRC-коды
        Crc crc1;
        Crc crc2;

        // вычислить первый CRC
        crc1.Begin();
        crc1.Compute(origData, origSize);
        crc1.End();

        crc2.Begin();
        crc2.Compute(newData, newSize);
        crc2.End();

        // сравнить CRC-коды
        if (crc1.GetResult() != crc2.GetResult())
        {
            // записать новый файл, если crc-коды не совпадают
            stream->Close();
            stream->SetAccessMode(Stream::WriteAccess);
            stream->Open();
            stream->Write(newData, newSize);
            stream->Close();
        }

        // закрыть поток сообщений
        memStream->Close();
    }
    else
    {
        // сохранить напрямую
        constants->Save(stream);
    }
}

//------------------------------------------------------------------------------
/**
*/
void 
SceneWriter::UpdateAttributes(const Util::String & file, const Ptr<ToolkitUtil::ModelAttributes>&attributes)
{
    // создать поток
    Ptr<Stream> stream = IoServer::Instance()->CreateStream(file);
    stream->SetAccessMode(Stream::ReadAccess);
    stream->Open();

    if (stream->IsOpen())
    {
        // создать поток памяти
        Ptr<Stream> memStream = MemoryStream::Create();

        // теперь сохрани константы
        attributes->Save(memStream);

        // прочитать из файла
        uchar* origData = (uchar*)stream->Map();
        SizeT origSize = stream->GetSize();

        // откройте мемстрим еще раз
        memStream->SetAccessMode(Stream::ReadAccess);
        memStream->Open();

        // получить данные из памяти
        uchar* newData = (uchar*)memStream->Map();
        SizeT newSize = memStream->GetSize();

        // создать CRC-коды
        Crc crc1;
        Crc crc2;

        // вычислить первый CRC
        crc1.Begin();
        crc1.Compute(origData, origSize);
        crc1.End();

        crc2.Begin();
        crc2.Compute(newData, newSize);
        crc2.End();

        // сравнить CRC-коды
        if (crc1.GetResult() != crc2.GetResult())
        {
            // записать новый файл, если crc-коды не совпадают
            stream->Close();
            stream->SetAccessMode(Stream::WriteAccess);
            stream->Open();
            stream->Write(newData, newSize);
            stream->Close();
        }

        // закрыть поток сообщений
        memStream->Close();
    }
    else
    {
        attributes->Save(stream);
    }
}

//------------------------------------------------------------------------------
/**
*/
void 
SceneWriter::UpdatePhysics(const Util::String & file, const Ptr<ToolkitUtil::ModelPhysics>&physics)
{
    // создать поток
    Ptr<Stream> stream = IoServer::Instance()->CreateStream(file);
    stream->SetAccessMode(Stream::ReadAccess);
    stream->Open();

    if (stream->IsOpen())
    {
        // создать поток памяти
        Ptr<Stream> memStream = MemoryStream::Create();

        // теперь сохрани константы
        physics->Save(memStream);

        // прочитать из файла
        uchar* origData = (uchar*)stream->Map();
        SizeT origSize = stream->GetSize();

        // откройте мемстрим еще раз
        memStream->SetAccessMode(Stream::ReadAccess);
        memStream->Open();

        // получить данные из памяти
        uchar* newData = (uchar*)memStream->Map();
        SizeT newSize = memStream->GetSize();

        // создать CRC-коды
        Crc crc1;
        Crc crc2;

        // вычислить первый CRC
        crc1.Begin();
        crc1.Compute(origData, origSize);
        crc1.End();

        crc2.Begin();
        crc2.Compute(newData, newSize);
        crc2.End();

        // сравнить CRC-коды
        if (crc1.GetResult() != crc2.GetResult())
        {
            // записать новый файл, если crc-коды не совпадают
            stream->Close();
            stream->SetAccessMode(Stream::WriteAccess);
            stream->Open();
            stream->Write(newData, newSize);
            stream->Close();
        }

        // закрыть поток сообщений
        memStream->Close();
    }
    else
    {
        physics->Save(stream);
    }
}

} // пространство имен ToolkitUtil
