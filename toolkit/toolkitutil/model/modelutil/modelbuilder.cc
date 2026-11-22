//------------------------------------------------------------------------------
//  modelbuilder.cc
//  (C) 2012 Густав Стербрант
//------------------------------------------------------------------------------
#include "foundation/stdneb.h"
#include "modelbuilder.h"
#include "io/stream.h"
#include "io/ioserver.h"
#include "io/filestream.h"
#include "modelconstants.h"
#include "math/transform44.h"
#include "math/transform.h"
#include "nflatbuffer/flatbufferinterface.h"
#include "nflatbuffer/nebula_flat.h"
#include "flat/physics/actor.h"

using namespace Util;
using namespace IO;
using namespace Particles;
namespace ToolkitUtil
{
__ImplementClass(ToolkitUtil::ModelBuilder, 'MDBU', Core::RefCounted);

//------------------------------------------------------------------------------
/**
*/
ModelBuilder::ModelBuilder() : 
    constants(nullptr),
    attributes(nullptr)
    ,physics(nullptr)
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
ModelBuilder::~ModelBuilder()
{
    this->constants = nullptr;
    this->attributes = nullptr;
    this->physics = nullptr;
}

//------------------------------------------------------------------------------
/**
*/
bool
ModelBuilder::SaveN3(const IO::URI& uri, Platform::Code platform)
{
    // убедитесь, что целевой каталог существует
    IoServer::Instance()->CreateDirectory(uri.LocalPath().ExtractDirName());

    // создать поток
    Ptr<Stream> stream = IoServer::Instance()->CreateStream(uri);
    stream->SetAccessMode(Stream::WriteAccess);
    if (stream->Open())
    {
        // создать двоичный модуль записи
        Ptr<BinaryModelWriter> writer = BinaryModelWriter::Create();
        writer->SetPlatform(platform);
        writer->SetStream(stream);

        // открытый писатель
        writer->Open();

        // начать модель
        writer->BeginModel("Model", 'MODL', this->constants->GetName());

            writer->BeginModelNode("TransformNode", 'TRFN', "root");

                writer->BeginTag("Ограничивающая рамка сцены", 'LBOX');

                const auto& sceneBB = this->constants->GetGlobalBoundingBox();
                writer->WriteVec4(sceneBB.center());
                writer->WriteVec4(sceneBB.extents());

                writer->EndTag();

                // писать символы
                this->WriteCharacters(writer);

                // писать скины
                this->WriteSkins(writer);

                // писать фигуры
                this->WriteShapes(writer);

                // писать частицы
                this->WriteParticles(writer);

            writer->EndModelNode();

        // конечное имя
        writer->EndModel();

        // близкий писатель
        writer->Close();

        stream->Close();
        return true;
    }

    return false;
}
//------------------------------------------------------------------------------
/**
*/
bool
ModelBuilder::SaveN3Physics(const IO::URI& uri, Platform::Code platform)
{

    // убедитесь, что целевой каталог существует
    IoServer::Instance()->CreateDirectory(uri.LocalPath().ExtractDirName());

    Util::Blob target = this->WritePhysics();
    if (!target.IsValid())
    {
        return false;
    }

    // создать поток
    Ptr<IO::FileStream> stream = IO::IoServer::Instance()->CreateStream(uri).downcast<IO::FileStream>();
    stream->SetAccessMode(Stream::WriteAccess);
    if (stream->Open())
    {
        stream->Write(target.GetPtr(), target.Size());
        stream->Close();
        return true;
    }

    return false;
}

//------------------------------------------------------------------------------
/**
*/
Util::Blob
ModelBuilder::WritePhysics()
{
    PhysicsResource::ActorT actor;
    switch(this->physics->GetExportMode())
    {
        case UseBoundingBox:
        case UseBoundingSphere:
        case UseBoundingCapsule:
            {
                auto body = std::make_unique< PhysicsResource::BodyT>();
                const Array<ModelConstants::ShapeNode> & nodes = this->constants->GetShapeNodes();
                for(Array<ModelConstants::ShapeNode>::Iterator iter = nodes.Begin();iter != nodes.End();iter++)
                {
                    auto newShape = std::make_unique<PhysicsResource::ShapeT>();
                    newShape->collider = std::make_unique<PhysicsResource::ColliderT>();
                    Math::mat4 nodetrans = iter->transform.GetTransform44().getmatrix();

                    Math::bbox colBox = iter->boundingBox;
                    Math::mat4 newtrans;
                    
                    colBox.transform(nodetrans);

                    newShape->material = this->physics->GetMaterial();
                    newShape->transform = Math::transform(colBox.center().vec);

                    switch (this->physics->GetExportMode())
                    {
                    case UseBoundingBox:
                    {
                        PhysicsResource::BoxColliderT col;
                        col.extents = colBox.extents();
                        newShape->collider->data.Set(col);
                        newShape->collider->name = iter->name;
                        newShape->collider->type = Physics::ColliderType_Box;
                    }
                    break;
                    case UseBoundingSphere:
                    {
                        PhysicsResource::SphereColliderT col;
                        Math::vector v = colBox.size();
                        col.radius = 0.5f * Math::min(v.x, Math::min(v.y, v.z));
                        newShape->collider->data.Set(col);
                        newShape->collider->name = iter->name;
                        newShape->collider->type = Physics::ColliderType_Sphere;
                    }
                    break;
                    case UseBoundingCapsule:
                    {
                        PhysicsResource::CapsuleColliderT col;
                        Math::vector v = colBox.size();
                        col.halfheight = v.y;
                        col.radius = Math::min(v.z, v.x);
                        newShape->collider->data.Set(col);
                        newShape->collider->name = iter->name;
                        newShape->collider->type = Physics::ColliderType_Capsule;
                    }
                    break;
                    default:
                        break;
                    }                    
                    body->shapes.push_back(std::move(newShape));
                }
                const Array<ModelConstants::ParticleNode> & particleNodes = this->constants->GetParticleNodes();
                for(Array<ModelConstants::ParticleNode>::Iterator iter = particleNodes.Begin();iter != particleNodes.End();iter++)
                {                   
                    auto newShape = std::make_unique<PhysicsResource::ShapeT>();
                    newShape->collider = std::make_unique<PhysicsResource::ColliderT>();
                    newShape->material = this->physics->GetMaterial();
                    Math::transform44 t = iter->transform.GetTransform44();
                    // частицы имеют довольно фиктивные значения, игнорируйте масштаб, если он равен нулю
                    if(lengthsq(t.getscale()) < 0.001f)
                    {
                        t.setscale(Math::vector(1,1,1));
                    }
                    Math::mat4 nodetrans = t.getmatrix();                   

                    Math::bbox colBox = iter->boundingBox;
                    Math::mat4 newtrans;

                    colBox.transform(nodetrans);
                    newShape->transform = Math::transform(iter->transform.position.vec, iter->transform.rotation, iter->transform.scale.vec);

                    PhysicsResource::BoxColliderT col;

                    col.extents = colBox.extents();
                    newShape->collider->data.Set(col);
                    newShape->collider->name = iter->name;
                    newShape->collider->type = Physics::ColliderType_Box;
                    body->shapes.push_back(std::move(newShape));
                }

                const Array<ModelConstants::SkinSetNode>& skinSets = this->constants->GetSkinSetNodes();
                for (Array<ModelConstants::SkinSetNode>::Iterator iter = skinSets.Begin(); iter != skinSets.End(); iter++)
                {
                    Math::mat4 setTransform = iter->transform.GetTransform44().getmatrix();
                    for (ModelConstants::SkinNode const& skinIter : iter->skinFragments)
                    {
                        auto newShape = std::make_unique<PhysicsResource::ShapeT>();
                        newShape->collider = std::make_unique<PhysicsResource::ColliderT>();
                        newShape->material = this->physics->GetMaterial();
                        Math::transform44 t = iter->transform.GetTransform44();

                        Math::mat4 nodetrans = setTransform * t.getmatrix();
                        Math::bbox colBox = skinIter.boundingBox;

                        colBox.transform(nodetrans);
                        newShape->transform = Math::transform(iter->transform.position.vec, iter->transform.rotation, iter->transform.scale.vec);

                        PhysicsResource::BoxColliderT col;

                        col.extents = colBox.extents();
                        newShape->collider->data.Set(col);
                        newShape->collider->name = skinIter.name;
                        newShape->collider->type = Physics::ColliderType_Box;
                        body->shapes.push_back(std::move(newShape));
                    }
                }
                PhysicsResource::BodySetupT* bodySetup = new PhysicsResource::BodySetupT;
                bodySetup->body = std::move(body);
                actor.data.type = PhysicsResource::PhysicsResourceUnion_BodySetup;
                actor.data.value = (void*)bodySetup;
            }
            break;
        case UseGraphicsMesh:
            {
                auto body = std::make_unique< PhysicsResource::BodyT>();
                // получить список фигур
                const Array<ModelConstants::ShapeNode>& shapes = this->constants->GetShapeNodes();
                
                for(int i=0;i<shapes.Size();i++)
                {
                    auto newShape = std::make_unique<PhysicsResource::ShapeT>();
                    newShape->material = this->physics->GetMaterial();
                    newShape->collider = std::make_unique<PhysicsResource::ColliderT>();
                    
                    PhysicsResource::MeshColliderT newColl;
                    newColl.file = shapes[i].meshResource;
                    newColl.prim_group = shapes[i].primitiveGroupIndex;
                    newColl.type = this->physics->GetMeshMode();
                    newShape->collider->type = Physics::ColliderType_Mesh;
                    newShape->collider->name = shapes[i].name;


                    newShape->transform = Math::transform(shapes[i].transform.position.vec, shapes[i].transform.rotation, shapes[i].transform.scale.vec);
                    newShape->collider->data.Set(newColl);
                    body->shapes.push_back(std::move(newShape));
                }
                const Array<ModelConstants::SkinSetNode>& skinSets = this->constants->GetSkinSetNodes();
                for (Array<ModelConstants::SkinSetNode>::Iterator iter = skinSets.Begin(); iter != skinSets.End(); iter++)
                {
                    Math::mat4 setTransform = iter->transform.GetTransform44().getmatrix();
                    for (ModelConstants::SkinNode const& skinIter : iter->skinFragments)
                    {
                        auto newShape = std::make_unique<PhysicsResource::ShapeT>();
                        newShape->material = this->physics->GetMaterial();
                        newShape->collider = std::make_unique<PhysicsResource::ColliderT>();

                        PhysicsResource::MeshColliderT newColl;
                        newColl.file = skinIter.meshResource;
                        newColl.prim_group = skinIter.primitiveGroupIndex;
                        newColl.type = this->physics->GetMeshMode();
                        newShape->collider->type = Physics::ColliderType_Mesh;
                        newShape->collider->name = skinIter.name;                  
                        
                        newShape->transform = Math::transform(skinIter.transform.position.vec, skinIter.transform.rotation, skinIter.transform.scale.vec);
                        newShape->collider->data.Set(newColl);
                        body->shapes.push_back(std::move(newShape));
                    }
                }
                PhysicsResource::BodySetupT* bodySetup = new PhysicsResource::BodySetupT;
                bodySetup->body = std::move(body);
                actor.data.type = PhysicsResource::PhysicsResourceUnion_BodySetup;
                actor.data.value = (void*)bodySetup;
            }
            break;
        case UsePhysics:
            {
                auto body = std::make_unique< PhysicsResource::BodyT>();
                if(this->constants->GetPhysicsNodes().Size()>0)
                {
                    // получить список фигур
                    const Array<ModelConstants::PhysicsNode>& shapes = this->constants->GetPhysicsNodes();
                                                
                    for(int i=0;i<shapes.Size();i++)
                    {
                        auto newShape = std::make_unique<PhysicsResource::ShapeT>();
                        newShape->material = this->physics->GetMaterial();
                        newShape->collider = std::make_unique<PhysicsResource::ColliderT>();
                        PhysicsResource::MeshColliderT newColl;
                        newColl.file = shapes[i].mesh;
                        newColl.prim_group = shapes[i].primitiveGroupIndex;
                        newColl.type = this->physics->GetMeshMode();
                        newShape->collider->type = Physics::ColliderType_Mesh;
                        newShape->collider->name = shapes[i].name;
                        newShape->transform = Math::transform(shapes[i].transform.position.vec, shapes[i].transform.rotation, shapes[i].transform.scale.vec);
                        newShape->collider->data.Set(newColl);
                        body->shapes.push_back(std::move(newShape));
                    }                                                                           
                }
                else
                {
                    auto newShape = std::make_unique<PhysicsResource::ShapeT>();
                    newShape->material = this->physics->GetMaterial();
                    newShape->collider = std::make_unique<PhysicsResource::ColliderT>();
                    PhysicsResource::MeshColliderT newColl;
                                        
                    newColl.type = this->physics->GetMeshMode();
                    newColl.file = this->physics->GetPhysicsMesh();
                    newColl.prim_group = 0;
                    newShape->collider->data.Set(newColl);
                    body->shapes.push_back(std::move(newShape));
                }         
                PhysicsResource::BodySetupT* bodySetup = new PhysicsResource::BodySetupT;
                bodySetup->body = std::move(body);
                actor.data.type = PhysicsResource::PhysicsResourceUnion_BodySetup;
                actor.data.value = (void*)bodySetup;
            }
            break;
        
        default:
            n_error("не реализовано");
    }
    return Flat::FlatbufferInterface::SerializeFlatbuffer<PhysicsResource::Actor>(actor);
}

//------------------------------------------------------------------------------
/**
*/
void 
WriteTransform(const Ptr<ModelWriter>& writer, const Transform& transform)
{
    if (transform.position != Math::point())
    {
        writer->BeginTag("Преобразование позиции", 'POSI');
        writer->WriteVec4(transform.position);
        writer->EndTag();
    }

    if (transform.rotation != Math::quat())
    {
        writer->BeginTag("Преобразовать вращение", 'ROTN');
        writer->WriteVec4(transform.rotation.vec);
        writer->EndTag();
    }

    if (transform.scale != Math::vector())
    {
        writer->BeginTag("Преобразование масштаба", 'SCAL');
        writer->WriteVec4(transform.scale);
        writer->EndTag();
    }

    if (transform.rotatePivot != Math::point())
    {
        writer->BeginTag("Преобразовать поворот вращения", 'RPIV');
        writer->WriteVec4(transform.rotatePivot);
        writer->EndTag();
    }

    if (transform.scalePivot != Math::point())
    {
        writer->BeginTag("Преобразование масштаба оси", 'SPIV');
        writer->WriteVec4(transform.scalePivot);
        writer->EndTag();
    }
}

//------------------------------------------------------------------------------
/**
*/
void
WriteState(
    const Ptr<ModelWriter>& writer
    , const Util::String& meshResource
    , const IndexT meshIndex
    , const Math::bbox& boundingBox
    , IndexT groupIndex
    , const State& state)
{
    // написать сетку
    writer->BeginTag("Ресурс сетки", 'MESH');
    writer->WriteString(meshResource);
    writer->EndTag();

    writer->BeginTag("Индекс сетки", 'MSHI');
    writer->WriteInt(meshIndex);
    writer->EndTag();

    // записать индекс примитивной группы
    writer->BeginTag("Индекс примитивной группы", 'PGRI');
    writer->WriteInt(groupIndex);
    writer->EndTag();

    // написать материал
    writer->BeginTag("Material", 'MATE');
    writer->WriteString(state.material);
    writer->EndTag();

    // написать ограничивающую рамку
    writer->BeginTag("Ограничивающая рамка", 'LBOX');
    writer->WriteVec4(boundingBox.center());
    writer->WriteVec4(boundingBox.extents());
    writer->EndTag();
}

//------------------------------------------------------------------------------
/**
*/
void
ModelBuilder::WriteShapes(const Ptr<ModelWriter>& writer)
{
    // получить список фигур
    const Array<ModelConstants::ShapeNode>& shapes = this->constants->GetShapeNodes();

    // перебирать фигуры
    IndexT i;
    for (i = 0; i < shapes.Size(); i++)
    {
        // получить форму
        const ModelConstants::ShapeNode& shape = shapes[i];

        // получить имя формы
        const String& name = shape.name;

        // получить состояние
        const State& state = this->attributes->GetState(shape.path);

        // затем создайте фактическую модель с узлом формы
        writer->BeginModelNode("ShapeNode", 'SPND', name);

            WriteTransform(writer, shape.transform);
            WriteState(writer, shape.meshResource, shape.meshIndex, shape.boundingBox, shape.primitiveGroupIndex, state);

            if (shape.useLOD)
            {
                writer->BeginTag("LODMinDistance", 'SMID');
                writer->WriteFloat(shape.LODMin);
                writer->EndTag();

                writer->BeginTag("LODMaxDistance", 'SMAD');
                writer->WriteFloat(shape.LODMax);
                writer->EndTag();
            }

        writer->EndModelNode();
    }
}

//------------------------------------------------------------------------------
/**
*/
void
ModelBuilder::WriteCharacters(const Ptr<ModelWriter>& writer)
{
    // получить список персонажей
    const Array<ModelConstants::CharacterNode>& characters = this->constants->GetCharacterNodes();

    // получить узел символа из констант, но получить только первый символ
    if (characters.Size() > 0)
    {
        const auto& jointMasks = this->GetAttributes()->GetJointMasks();
        if (jointMasks.Size() > 0)
        {
            // напишите количество масок
            writer->BeginTag("Количество масок", 'NJMS');
            writer->WriteInt(jointMasks.Size());
            writer->EndTag();

            // написать совместную маску
            for (const auto& jointMask : jointMasks)
            {
                writer->BeginTag("Суставная маска", 'JOMS');
                writer->WriteString(jointMask.name);
                writer->WriteInt(jointMask.weights.Size());
                for (const auto weight : jointMask.weights)
                {
                    writer->WriteFloat(weight);
                }
                writer->EndTag();
            }
        }
    }
}

//------------------------------------------------------------------------------
/**
*/
void
ModelBuilder::WriteSkins(const Ptr<ModelWriter>& writer)
{
    // получить список скинов
    const Array<ModelConstants::SkinSetNode>& skinSets = this->constants->GetSkinSetNodes();

    // перебирать скины
    for (const auto& skinSet : skinSets)
    {
        // узнать название скина
        const String& name = skinSet.name;

        writer->BeginModelNode("CharacterSkinNode", 'CHSN', name);
            WriteTransform(writer, skinSet.transform);

            writer->BeginTag("Количество фрагментов кожи", 'NSKF');
            writer->WriteInt(skinSet.skinFragments.Size());
            writer->EndTag();

            for (const auto& skinFragment : skinSet.skinFragments)
            {
                // напишите использованные фрагменты скина
                writer->BeginTag("Использованные фрагменты кожи", 'SFRG');
                writer->WriteInt(skinFragment.primitiveGroupIndex);

                // напишите использованные суставы для фрагмента
                writer->WriteInt(skinFragment.fragmentJoints.Size());

                IndexT j;
                for (j = 0; j < skinFragment.fragmentJoints.Size(); j++)
                {
                    writer->WriteInt(skinFragment.fragmentJoints[j]);
                }
                writer->EndTag();

                // получить состояние имени
                const State& state = this->attributes->GetState(skinFragment.path);

                // писать основную информацию, такую ​​как материал, переменные шейдера, текстуры и сетка. (PGRI равен 0, потому что у нас есть скины)
                WriteState(writer, skinFragment.meshResource, skinFragment.meshIndex, skinFragment.boundingBox, skinFragment.primitiveGroupIndex, state);
            }

        // конечный узел кожи
        writer->EndModelNode();
    }
}

//------------------------------------------------------------------------------
/**
*/
void
ModelBuilder::WriteParticles(const Ptr<ModelWriter>& writer)
{
    // получить список частиц
    const Array<ModelConstants::ParticleNode>& particlesNodes = this->constants->GetParticleNodes();

    // перебирать узлы
    IndexT i;
    for (i = 0; i < particlesNodes.Size(); i++)
    {
        // получить узел частицы
        const ModelConstants::ParticleNode& particleNode = particlesNodes[i];

        // получить имя частицы
        const String& name = particleNode.name;

        // получить состояние частицы
        const State& state = this->attributes->GetState(particleNode.path);

        // получить атрибуты
        const EmitterAttrs& emitterAttrs = this->attributes->GetEmitterAttrs(particleNode.path);

        // получить сетку эмиттера
        const String& emitterMesh = this->attributes->GetEmitterMesh(particleNode.path);

        writer->BeginModelNode("ParticleSystemNode", 'PSND', name);
            WriteTransform(writer, particleNode.transform);
            WriteState(writer, emitterMesh, particleNode.meshIndex, Math::bbox(), particleNode.primitiveGroupIndex, state);

            // написать кривую частоты излучения
            writer->BeginTag("Частота излучения", 'EFRQ');
            emitterAttrs.GetEnvelope(Particles::EmitterAttrs::EmissionFrequency);
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::EmissionFrequency).GetValues()[0]);
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::EmissionFrequency).GetValues()[1]);
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::EmissionFrequency).GetValues()[2]);
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::EmissionFrequency).GetValues()[3]);
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::EmissionFrequency).GetKeyPos0());
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::EmissionFrequency).GetKeyPos1());
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::EmissionFrequency).GetFrequency());
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::EmissionFrequency).GetAmplitude());
            writer->WriteInt(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::EmissionFrequency).GetModFunc());
            writer->EndTag();

            // написать кривую времени жизни
            writer->BeginTag("Продолжительность жизни", 'PLFT');
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::LifeTime).GetValues()[0]);
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::LifeTime).GetValues()[1]);
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::LifeTime).GetValues()[2]);
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::LifeTime).GetValues()[3]);
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::LifeTime).GetKeyPos0());
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::LifeTime).GetKeyPos1());
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::LifeTime).GetFrequency());
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::LifeTime).GetAmplitude());
            writer->WriteInt(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::LifeTime).GetModFunc());
            writer->EndTag();

            // написать минимальную кривую спреда
            writer->BeginTag("Спред мин.", 'PSMN');
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::SpreadMin).GetValues()[0]);
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::SpreadMin).GetValues()[1]);
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::SpreadMin).GetValues()[2]);
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::SpreadMin).GetValues()[3]);
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::SpreadMin).GetKeyPos0());
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::SpreadMin).GetKeyPos1());
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::SpreadMin).GetFrequency());
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::SpreadMin).GetAmplitude());
            writer->WriteInt(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::SpreadMin).GetModFunc());
            writer->EndTag();

            // написать кривую максимального распространения
            writer->BeginTag("Распространение Макс.", 'PSMX');
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::SpreadMax).GetValues()[0]);
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::SpreadMax).GetValues()[1]);
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::SpreadMax).GetValues()[2]);
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::SpreadMax).GetValues()[3]);
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::SpreadMax).GetKeyPos0());
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::SpreadMax).GetKeyPos1());
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::SpreadMax).GetFrequency());
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::SpreadMax).GetAmplitude());
            writer->WriteInt(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::SpreadMax).GetModFunc());
            writer->EndTag();

            // написать начальную кривую скорости
            writer->BeginTag("Начальная скорость", 'PSVL');
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::StartVelocity).GetValues()[0]);
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::StartVelocity).GetValues()[1]);
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::StartVelocity).GetValues()[2]);
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::StartVelocity).GetValues()[3]);
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::StartVelocity).GetKeyPos0());
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::StartVelocity).GetKeyPos1());
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::StartVelocity).GetFrequency());
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::StartVelocity).GetAmplitude());
            writer->WriteInt(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::StartVelocity).GetModFunc());
            writer->EndTag();

            // написать кривую скорости вращения
            writer->BeginTag("Скорость вращения", 'PRVL');
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::RotationVelocity).GetValues()[0]);
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::RotationVelocity).GetValues()[1]);
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::RotationVelocity).GetValues()[2]);
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::RotationVelocity).GetValues()[3]);
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::RotationVelocity).GetKeyPos0());
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::RotationVelocity).GetKeyPos1());
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::RotationVelocity).GetFrequency());
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::RotationVelocity).GetAmplitude());
            writer->WriteInt(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::RotationVelocity).GetModFunc());
            writer->EndTag();

            // написать кривую размера
            writer->BeginTag("Size", 'PSZE');
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::Size).GetValues()[0]);
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::Size).GetValues()[1]);
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::Size).GetValues()[2]);
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::Size).GetValues()[3]);
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::Size).GetKeyPos0());
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::Size).GetKeyPos1());
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::Size).GetFrequency());
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::Size).GetAmplitude());
            writer->WriteInt(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::Size).GetModFunc());
            writer->EndTag();

            // написать массовую кривую
            writer->BeginTag("Mass", 'PMSS');
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::Mass).GetValues()[0]);
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::Mass).GetValues()[1]);
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::Mass).GetValues()[2]);
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::Mass).GetValues()[3]);
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::Mass).GetKeyPos0());
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::Mass).GetKeyPos1());
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::Mass).GetFrequency());
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::Mass).GetAmplitude());
            writer->WriteInt(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::Mass).GetModFunc());
            writer->EndTag();

            // написать кривую манипулятора времени
            writer->BeginTag("Манипулятор времени", 'PTMN');
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::TimeManipulator).GetValues()[0]);
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::TimeManipulator).GetValues()[1]);
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::TimeManipulator).GetValues()[2]);
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::TimeManipulator).GetValues()[3]);
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::TimeManipulator).GetKeyPos0());
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::TimeManipulator).GetKeyPos1());
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::TimeManipulator).GetFrequency());
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::TimeManipulator).GetAmplitude());
            writer->WriteInt(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::TimeManipulator).GetModFunc());
            writer->EndTag();

            // написать кривую коэффициента скорости
            writer->BeginTag("Фактор скорости", 'PVLF');
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::VelocityFactor).GetValues()[0]);
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::VelocityFactor).GetValues()[1]);
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::VelocityFactor).GetValues()[2]);
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::VelocityFactor).GetValues()[3]);
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::VelocityFactor).GetKeyPos0());
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::VelocityFactor).GetKeyPos1());
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::VelocityFactor).GetFrequency());
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::VelocityFactor).GetAmplitude());
            writer->WriteInt(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::VelocityFactor).GetModFunc());
            writer->EndTag();

            // написать кривую сопротивления воздуха
            writer->BeginTag("Сопротивление воздуха", 'PAIR');
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::AirResistance).GetValues()[0]);
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::AirResistance).GetValues()[1]);
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::AirResistance).GetValues()[2]);
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::AirResistance).GetValues()[3]);
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::AirResistance).GetKeyPos0());
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::AirResistance).GetKeyPos1());
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::AirResistance).GetFrequency());
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::AirResistance).GetAmplitude());
            writer->WriteInt(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::AirResistance).GetModFunc());
            writer->EndTag();

            // написать красную кривую
            writer->BeginTag("Red", 'PRED');
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::Red).GetValues()[0]);
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::Red).GetValues()[1]);
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::Red).GetValues()[2]);
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::Red).GetValues()[3]);
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::Red).GetKeyPos0());
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::Red).GetKeyPos1());
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::Red).GetFrequency());
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::Red).GetAmplitude());
            writer->WriteInt(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::Red).GetModFunc());
            writer->EndTag();

            // написать зеленую кривую
            writer->BeginTag("Green", 'PGRN');
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::Green).GetValues()[0]);
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::Green).GetValues()[1]);
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::Green).GetValues()[2]);
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::Green).GetValues()[3]);
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::Green).GetKeyPos0());
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::Green).GetKeyPos1());
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::Green).GetFrequency());
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::Green).GetAmplitude());
            writer->WriteInt(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::Green).GetModFunc());
            writer->EndTag();

            // написать синюю кривую
            writer->BeginTag("Blue", 'PBLU');
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::Blue).GetValues()[0]);
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::Blue).GetValues()[1]);
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::Blue).GetValues()[2]);
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::Blue).GetValues()[3]);
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::Blue).GetKeyPos0());
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::Blue).GetKeyPos1());
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::Blue).GetFrequency());
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::Blue).GetAmplitude());
            writer->WriteInt(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::Blue).GetModFunc());
            writer->EndTag();

            // написать альфа-кривую
            writer->BeginTag("Alpha", 'PALP');
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::Alpha).GetValues()[0]);
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::Alpha).GetValues()[1]);
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::Alpha).GetValues()[2]);
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::Alpha).GetValues()[3]);
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::Alpha).GetKeyPos0());
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::Alpha).GetKeyPos1());
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::Alpha).GetFrequency());
            writer->WriteFloat(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::Alpha).GetAmplitude());
            writer->WriteInt(emitterAttrs.GetEnvelope(Particles::EmitterAttrs::Alpha).GetModFunc());
            writer->EndTag();

            // написать EmissionDuration
            writer->BeginTag("EmissionDuration", 'PEDU');
            writer->WriteFloat(emitterAttrs.GetFloat(Particles::EmitterAttrs::EmissionDuration));
            writer->EndTag();

            // написать цикл
            writer->BeginTag("Looping", 'PLPE');
            writer->WriteInt(emitterAttrs.GetBool(Particles::EmitterAttrs::Looping));
            writer->EndTag();

            // написать расстояние активности
            writer->BeginTag("Расстояние активности", 'PACD');
            writer->WriteFloat(emitterAttrs.GetFloat(Particles::EmitterAttrs::ActivityDistance));
            writer->EndTag();

            // написать «Сначала отображать самые старые»
            writer->BeginTag("Отрисовывать сначала самые старые", 'PROF');
            writer->WriteInt(emitterAttrs.GetBool(Particles::EmitterAttrs::RenderOldestFirst));
            writer->EndTag();

            // написать рекламный щит
            writer->BeginTag("Billboard", 'PBBO');
            writer->WriteInt(emitterAttrs.GetBool(Particles::EmitterAttrs::Billboard));
            writer->EndTag();

            // напишите «Начало вращения мин»
            writer->BeginTag("Начало вращения мин.", 'PRMN');
            writer->WriteFloat(emitterAttrs.GetFloat(Particles::EmitterAttrs::StartRotationMin));
            writer->EndTag();

            // напишите «Начало вращения Макс»
            writer->BeginTag("Начало вращения Макс.", 'PRMX');
            writer->WriteFloat(emitterAttrs.GetFloat(Particles::EmitterAttrs::StartRotationMax));
            writer->EndTag();

            // написать Гравитацию
            writer->BeginTag("Gravity", 'PGRV');
            writer->WriteFloat(emitterAttrs.GetFloat(Particles::EmitterAttrs::Gravity));
            writer->EndTag();

            // написать растяжение частиц
            writer->BeginTag("Растяжение частиц", 'PSTC');
            writer->WriteFloat(emitterAttrs.GetFloat(Particles::EmitterAttrs::ParticleStretch));
            writer->EndTag();

            // написать текстуру плитки
            writer->BeginTag("Текстурная плитка", 'PTTX');
            writer->WriteInt((int)(emitterAttrs.GetFloat(Particles::EmitterAttrs::TextureTile)));
            writer->EndTag();

            // напишите «Растянуть для начала»
            writer->BeginTag("Растянуть, чтобы начать", 'PSTS');
            writer->WriteInt(emitterAttrs.GetBool(Particles::EmitterAttrs::StretchToStart));
            writer->EndTag();

            // написать скорость в случайном порядке
            writer->BeginTag("Рандомизация скорости", 'PVRM');
            writer->WriteFloat(emitterAttrs.GetFloat(Particles::EmitterAttrs::VelocityRandomize));
            writer->EndTag();

            // написать вращение в случайном порядке
            writer->BeginTag("Вращение в случайном порядке", 'PRRM');
            writer->WriteFloat(emitterAttrs.GetFloat(Particles::EmitterAttrs::RotationRandomize));
            writer->EndTag();

            // написать размер в случайном порядке
            writer->BeginTag("Размер случайным образом", 'PSRM');
            writer->WriteFloat(emitterAttrs.GetFloat(Particles::EmitterAttrs::SizeRandomize));
            writer->EndTag();

            // написать время предварительного расчета
            writer->BeginTag("Предварительный расчет времени", 'PPCT');
            writer->WriteFloat(emitterAttrs.GetFloat(Particles::EmitterAttrs::PrecalcTime));
            writer->EndTag();

            // написать случайное вращение
            writer->BeginTag("Рандомизировать вращение", 'PRRD');
            writer->WriteInt(emitterAttrs.GetBool(Particles::EmitterAttrs::RandomizeRotation));
            writer->EndTag();

            // написать Растянуть детали
            writer->BeginTag("Растянуть детали", 'PSDL');
            writer->WriteInt(emitterAttrs.GetInt(Particles::EmitterAttrs::StretchDetail));
            writer->EndTag();

            // написать Угол обзора Fade
            writer->BeginTag("Угол обзора исчезает", 'PVAF');
            writer->WriteInt(emitterAttrs.GetBool(Particles::EmitterAttrs::ViewAngleFade));
            writer->EndTag();

            // написать StartDelay
            writer->BeginTag("StartDelay", 'PDEL');
            writer->WriteFloat(emitterAttrs.GetFloat(Particles::EmitterAttrs::StartDelay));
            writer->EndTag();

            // написать Фазы в секунду
            writer->BeginTag("Фазы в секунду", 'PDPS');
            writer->WriteFloat(emitterAttrs.GetFloat(Particles::EmitterAttrs::PhasesPerSecond));
            writer->EndTag();

            // напиши направление ветра
            writer->BeginTag("Направление ветра", 'WIDR');
            writer->WriteVec4(emitterAttrs.GetVec4(Particles::EmitterAttrs::WindDirection));
            writer->EndTag();

            // написать Угол обзора Fade
            writer->BeginTag("AnimPhases", 'PVAP');
            writer->WriteInt(emitterAttrs.GetInt(Particles::EmitterAttrs::AnimPhases));
            writer->EndTag();

        writer->EndModelNode();

    }
}

} // пространство имен ToolkitUtil