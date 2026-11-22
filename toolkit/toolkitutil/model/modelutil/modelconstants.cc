//------------------------------------------------------------------------------
//  modelconstants.cc
//  (C) 2012 Густав Стербрант
//  (C) 2012–2020 гг. Индивидуальные участники. См. файл АВТОРЫ.
//------------------------------------------------------------------------------
#include "foundation/stdneb.h"
#include "modelconstants.h"
#include "io/xmlwriter.h"
#include "io/xmlreader.h"
#include "io/stream.h"
#include "io/ioserver.h"

using namespace IO;
using namespace Util;
namespace ToolkitUtil
{
__ImplementClass(ToolkitUtil::ModelConstants, 'MDCN', Core::RefCounted);

//------------------------------------------------------------------------------
/**
*/
ModelConstants::ModelConstants()
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
ModelConstants::~ModelConstants()
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
void 
ModelConstants::AddCharacterNode( const Util::String& name, const ModelConstants::CharacterNode& node )
{
    n_assert(!this->characterNodes.Contains(name));
    this->characterNodes.Add(name, node);
}

//------------------------------------------------------------------------------
/**
*/
const ModelConstants::CharacterNode& 
ModelConstants::GetCharacterNode(const Util::String& name) const
{
    n_assert(this->characterNodes.Contains(name));
    return this->characterNodes[name];
}

//------------------------------------------------------------------------------
/**
*/
bool
ModelConstants::HasCharacterNode(const Util::String& name)
{
    return this->characterNodes.Contains(name);
}

//------------------------------------------------------------------------------
/**
*/
void
ModelConstants::DeleteCharacterNode(const Util::String& name)
{
    n_assert(this->characterNodes.Contains(name));
    this->characterNodes.Erase(name);
}

//------------------------------------------------------------------------------
/**
*/
void
ModelConstants::AddShapeNode(const ModelConstants::ShapeNode& node)
{
    this->shapeNodes.Append(node);
}

//------------------------------------------------------------------------------
/**
*/
const Util::Array<ModelConstants::ShapeNode>&
ModelConstants::GetShapeNodes() const
{
    return this->shapeNodes;
}

//------------------------------------------------------------------------------
/**
*/
void 
ModelConstants::AddSkinSetNode(const ModelConstants::SkinSetNode& skinSet)
{
    this->skinSetNodes.Append(skinSet);
}

//------------------------------------------------------------------------------
/**
*/
const Util::Array<ModelConstants::SkinSetNode>& 
ModelConstants::GetSkinSetNodes() const
{
    return this->skinSetNodes;
}

//------------------------------------------------------------------------------
/**
*/
void 
ModelConstants::AddParticleNode( const Util::String& name, const ModelConstants::ParticleNode& node )
{
    n_assert(!this->particleNodes.Contains(name));
    this->particleNodes.Add(name, node);
}

//------------------------------------------------------------------------------
/**
*/
const ModelConstants::ParticleNode& 
ModelConstants::GetParticleNode( const Util::String& name )
{
    n_assert(this->particleNodes.Contains(name));
    return this->particleNodes[name];
}

//------------------------------------------------------------------------------
/**
*/
bool
ModelConstants::HasParticleNode(const Util::String& name)
{
    return this->particleNodes.Contains(name);
}

//------------------------------------------------------------------------------
/**
*/
void
ModelConstants::DeleteParticleNode(const Util::String& name)
{
    n_assert(this->particleNodes.Contains(name));
    this->particleNodes.Erase(name);
}

//------------------------------------------------------------------------------
/**
*/
void 
ModelConstants::ReplaceParticleNode( const Util::String& name, const ModelConstants::ParticleNode& node )
{
    n_assert(this->particleNodes.Contains(name));
    this->particleNodes[name] = node;
}

//------------------------------------------------------------------------------
/**
*/
void 
ModelConstants::AddPhysicsNode( const Util::String& name, const ModelConstants::PhysicsNode& node )
{
    n_assert(!this->physicsNodes.Contains(name));
    this->physicsNodes.Add(name, node);
}

//------------------------------------------------------------------------------
/**
*/
const ModelConstants::PhysicsNode& 
ModelConstants::GetPhysicsNode( const Util::String& name )
{
    n_assert(this->physicsNodes.Contains(name));
    return this->physicsNodes[name];
}

//------------------------------------------------------------------------------
/**
*/
bool
ModelConstants::HasPhysicsNode(const Util::String& name)
{
    return this->physicsNodes.Contains(name);
}

//------------------------------------------------------------------------------
/**
*/
void
ModelConstants::DeletePhysicsNode(const Util::String& name)
{
    n_assert(this->physicsNodes.Contains(name));
    this->physicsNodes.Erase(name);
}

//------------------------------------------------------------------------------
/**
*/
void
WriteTransform(const Ptr<XmlWriter>& writer, const ModelConstants::TransformNode* node, bool hasLOD = false)
{
    // напиши имя
    writer->SetString("name", node->name);

    // написать путь
    writer->SetString("path", node->path);

    if (hasLOD)
    {
        // напишите, нужно ли использовать lod
        writer->SetBool("useLOD", node->useLOD);

        if (node->useLOD)
        {
            // напишите лодочные расстояния
            writer->SetFloat("LODMax", node->LODMax);
            writer->SetFloat("LODMin", node->LODMin);
        }
    }

    // установить положение
    writer->SetVec4("position", node->transform.position);

    // установить вращение
    writer->SetVec4("rotation", node->transform.rotation.vec);

    // установить масштаб
    writer->SetVec4("scale", node->transform.scale);

    // установить ограничивающую рамку
    writer->SetVec4("bboxcenter", node->boundingBox.center());
    writer->SetVec4("bboxextents", node->boundingBox.extents());
}

//------------------------------------------------------------------------------
/**
*/
void
ReadTransform(const Ptr<XmlReader>& reader, ModelConstants::TransformNode* node, bool hasLOD = false)
{
    node->name = reader->GetString("name");
    node->path = reader->GetString("path");
    node->boundingBox = Math::bbox(xyz(reader->GetVec4("bboxcenter")), xyz(reader->GetVec4("bboxextents")));

    if (hasLOD)
    {
        node->useLOD = reader->GetBool("useLOD");
        if (node->useLOD)
        {
            node->LODMin = reader->GetFloat("LODMin");
            node->LODMax = reader->GetFloat("LODMax");
        }
    }

    // получить позицию
    node->transform.position = reader->GetVec4("position");

    // получить ротацию
    node->transform.rotation = reader->GetVec4("rotation").vec;

    // получить масштаб
    node->transform.scale = reader->GetVec4("scale").vec;
}

//------------------------------------------------------------------------------
/**
*/
void 
ModelConstants::Save(const Ptr<Stream>& stream)
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

        // начни с тега туманности
        writer->BeginNode("Nebula");

        // установить версию
        writer->SetInt("version", ModelConstants::Version);

        // стартовый узел модели
        writer->BeginNode("Model");

        // установить название модели
        writer->SetString("name", this->name);

        // установить ограничивающую рамку
        writer->SetVec4("bboxcenter", this->globalBoundingBox.center());
        writer->SetVec4("bboxextents", this->globalBoundingBox.extents());

        if (this->characterNodes.Size() > 0)
        {
            // напишите контент, начните с узлов символов
            writer->BeginNode("CharacterNodes");

            // пройтись по фигурам и написать узлы
            IndexT i;
            for (i = 0; i < this->characterNodes.Size(); i++)
            {

                // получить узел формы
                const ModelConstants::CharacterNode& node = this->characterNodes.ValueAtIndex(i);

                // начальный символьный узел
                writer->BeginNode("CharacterNode");

                // установить имя символьного узла
                writer->SetString("name", node.name);

                // установить ресурс анимации
                writer->SetString("animation", node.animationResource);
                writer->SetInt("animationIndex", node.animationIndex);

                // установить ресурс скелета
                writer->SetString("skeleton", node.skeletonResource);
                writer->SetInt("skeletonIndex", node.skeletonIndex);

                // конечный символьный узел
                writer->EndNode();
            }

            // конечные узлы символов
            writer->EndNode();
        }
        
        if (this->shapeNodes.Size() > 0)
        {
            // писать узлы формы
            writer->BeginNode("ShapeNodes");

            // пройтись по фигурам и написать узлы
            for (const auto& shapeNode : this->shapeNodes)
            {
                // написать узел формы
                writer->BeginNode("ShapeNode");

                WriteTransform(writer, &shapeNode, true);

                // установить сетку
                writer->SetString("mesh", shapeNode.meshResource);
                writer->SetInt("mid", shapeNode.meshIndex);
                writer->SetInt("primitive", shapeNode.primitiveGroupIndex);

                // Конечный узел формы
                writer->EndNode();
            }

            // Тег конечного узла формы
            writer->EndNode();
        }

        if (this->physicsNodes.Size() > 0)
        {
            // начать узлы физики
            writer->BeginNode("PhysicsNodes");

            // пройти и выписать узлы физики
            IndexT i;
            for (i = 0; i < this->physicsNodes.Size(); i++)
            {
                // получить физический узел
                const ModelConstants::PhysicsNode& node = this->physicsNodes.ValueAtIndex(i);

                // написать физический узел
                writer->BeginNode("PhysicsNode");

                // записать данные
                writer->SetString("name", node.name);
                writer->SetString("path", node.path);

                // установить положение
                writer->SetVec4("position", node.transform.position);

                // установить вращение
                writer->SetVec4("rotation", node.transform.rotation.vec);

                // установить масштаб
                writer->SetVec4("scale", node.transform.scale);

                // Написать сетку
                writer->SetString("mesh", node.mesh);
                writer->SetInt("mid", 0);
                writer->SetInt("primitive", node.primitiveGroupIndex);

                // конечный физический узел
                writer->EndNode();
            }

            // конечные физические узлы
            writer->EndNode();
        }

        if (this->particleNodes.Size() > 0)
        {
            // писать узлы формы
            writer->BeginNode("ParticleNodes");

            // пройтись по фигурам и написать узлы
            IndexT i;
            for (i = 0; i < this->particleNodes.Size(); i++)
            {
                // получить узел формы
                const ModelConstants::ParticleNode& node = this->particleNodes.ValueAtIndex(i);

                // написать узел формы
                writer->BeginNode("ParticleNode");

                WriteTransform(writer, &node);

                // записать индекс примитивной группы
                writer->SetInt("primitive", node.primitiveGroupIndex);

                // Конечный узел формы
                writer->EndNode();
            }

            // Тег конечного узла формы
            writer->EndNode();
        }

        if (this->skinSetNodes.Size() > 0)
        {
            writer->BeginNode("SkinSetNodes");
            for (const auto& skinSet : this->skinSetNodes)
            {
                writer->BeginNode("SkinSetNode");

                WriteTransform(writer, &skinSet);

                if (skinSet.skinFragments.Size() > 0)
                {

                    // писать скины 
                    for (const auto& skin : skinSet.skinFragments)
                    {
                        // написать скин
                        writer->BeginNode("SkinNode");

                        WriteTransform(writer, &skin, true);

                        // написать сетку
                        writer->SetString("mesh", skin.meshResource);
                        writer->SetInt("mid", skin.meshIndex);
                        writer->SetInt("primitive", skin.primitiveGroupIndex);

                        // написать тег для суставов
                        writer->BeginNode("Joints");

                        // мы пишем здесь каждый сустав как контент
                        IndexT l;
                        for (l = 0; l < skin.fragmentJoints.Size(); l++)
                        {
                            writer->WriteContent(String::FromInt(skin.fragmentJoints[l]));
                            if (l < skin.fragmentJoints.Size() - 1)
                            {
                                writer->WriteContent(", ");
                            }
                        }

                        // Торцевые соединения
                        writer->EndNode();
                    }

                    // торцевые оболочки
                    writer->EndNode();
                }

                // Конечный набор скинов
                writer->EndNode();
            }
        }

        // закрыть узел модели
        writer->EndNode();

        // закрыть узел nebula3
        writer->EndNode();

        // закрывает запись и файл
        writer->Close();
        stream->Close();
    }
}

//------------------------------------------------------------------------------
/**
*/
void 
ModelConstants::Load(const Ptr<Stream>& stream)
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

        // if the versions mismatch, don't continue with the loading process
        if (version != ModelConstants::Version)
        {
            n_warning("Неверная версия констант модели: %s\n", stream->GetURI().LocalPath().AsCharPtr());
            stream->Close();
            return;
        }

        // затем убедитесь, что у нас есть тег параметров
        n_assert2(reader->SetToFirstChild("Model"), "ПОВРЕЖДЕН ФАЙЛ .constants!: Первым тегом должна быть модель!");

        // получить имя
        this->name = reader->GetString("name");

        // получить ограничивающую рамку
        this->globalBoundingBox = Math::bbox(reader->GetVec4("bboxcenter"), reader->GetVec4("bboxextents").vec);

        // сначала проверьте, есть ли у нас символьный узел
        if (reader->SetToFirstChild("CharacterNodes"))
        {
            // пройти через все узлы персонажей
            if (reader->SetToFirstChild("CharacterNode")) do 
            {
                // создать узел персонажа
                ModelConstants::CharacterNode characterNode;

                // получить имя
                characterNode.name = reader->GetString("name");

                // получить анимацию
                characterNode.animationResource = reader->GetString("animation");
                characterNode.animationIndex = reader->GetInt("animationIndex");

                // получить скелет
                characterNode.skeletonResource = reader->GetString("skeleton");
                characterNode.skeletonIndex = reader->GetInt("skeletonIndex");

                // добавить символ в константы
                this->AddCharacterNode(characterNode.name, characterNode);
            } 
            while (reader->SetToNextChild("CharacterNode"));

            // вернуться к родительскому элементу
            reader->SetToParent();
        }

        if (reader->SetToFirstChild("SkinSetNodes"))
        {
            if (reader->SetToFirstChild("SkinSetNode")) do
            {
                ModelConstants::SkinSetNode skinSetNode;
                ReadTransform(reader, &skinSetNode);

                if (reader->SetToFirstChild("SkinNode")) do
                {
                    ModelConstants::SkinNode skin;

                    ReadTransform(reader, &skin);
                    skin.meshResource = reader->GetString("mesh");
                    skin.meshIndex = reader->GetInt("mid");

                    // получить примитивный индекс
                    skin.primitiveGroupIndex = reader->GetInt("primitive");

                    if (reader->SetToFirstChild("Joints"))
                    {
                        const Util::String& content = reader->GetContent();
                        Util::Array<String> joints = content.Tokenize(", ");
                        for (const auto& joint : joints)
                        {
                            skin.fragmentJoints.Append(joint.AsInt());
                        }
                    }
                    reader->SetToParent();

                    skinSetNode.skinFragments.Append(skin);
                } 
                while (reader->SetToNextChild("SkinNode"));

                this->AddSkinSetNode(skinSetNode);

            } 
            while (reader->SetToNextChild("SkinSetNode"));
        }

        // проходить через формы
        if (reader->SetToFirstChild("ShapeNodes"))
        {
            // перебирать все фигуры
            if (reader->SetToFirstChild("ShapeNode")) do
            {
                // получить узел формы
                ModelConstants::ShapeNode node;

                ReadTransform(reader, &node, true);

                // получить примитивный индекс
                node.primitiveGroupIndex = reader->GetInt("primitive");

                // получить сетку
                node.meshResource = reader->GetString("mesh");
                node.meshIndex = reader->GetOptInt("mid", 0);

                // добавить форму константам
                this->AddShapeNode(node);

            }
            while (reader->SetToNextChild("ShapeNode"));

            // вернуться к родительскому элементу
            reader->SetToParent();
        }

        // пройти физику
        if (reader->SetToFirstChild("PhysicsNodes"))
        {
            // перебрать все физические узлы
            if (reader->SetToFirstChild("PhysicsNode")) do
            {
                // создать узел физики
                ModelConstants::PhysicsNode node;

                // установить данные
                node.name = reader->GetString("name");
                node.path = reader->GetString("path");
                node.mesh = reader->GetString("mesh");
                node.primitiveGroupIndex = reader->GetInt("primitive");

                // получить позицию
                node.transform.position = reader->GetVec4("position");

                // получить ротацию
                node.transform.rotation = reader->GetVec4("rotation").vec;

                // получить масштаб
                node.transform.scale = reader->GetVec4("scale").vec;

                // добавить узел
                this->AddPhysicsNode(node.name, node);
            }
            while (reader->SetToNextChild("PhysicsNode"));

            // перейти к родителю
            reader->SetToParent();
        }

        // проходить через формы
        if (reader->SetToFirstChild("ParticleNodes"))
        {
            // перебирать все фигуры
            if (reader->SetToFirstChild("ParticleNode")) do
            {
                // создать узел частиц
                ModelConstants::ParticleNode node;

                ReadTransform(reader, &node);

                // получить примитивный индекс
                node.primitiveGroupIndex = reader->GetInt("primitive");

                // добавить форму константам
                this->AddParticleNode(node.name, node);

            }
            while (reader->SetToNextChild("ParticleNode"));

            // вернуться к родительскому элементу
            reader->SetToParent();
        }

        // закрывает читалку и файл
        reader->Close();
        stream->Close();
    }
}

//------------------------------------------------------------------------------
/**
*/
void 
ModelConstants::Clear()
{
    this->shapeNodes.Clear();
    this->characterNodes.Clear();
    this->skinSetNodes.Clear();
    this->physicsNodes.Clear();
    this->particleNodes.Clear();
}

} // пространство имен ToolkitUtil