//------------------------------------------------------------------------------
//  modelattributes.cc
//  (C) 2012–2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "foundation/stdneb.h"
#include "modelattributes.h"
#include "io/stream.h"
#include "io/ioserver.h"
#include "clip.h"
#include "io/xmlwriter.h"
#include "io/xmlreader.h"
#include "util/crc.h"

using namespace IO;
using namespace Util;
using namespace Particles;
namespace ToolkitUtil
{
__ImplementClass(ToolkitUtil::ModelAttributes, 'MOAT', Core::RefCounted);

//------------------------------------------------------------------------------
/**
*/
ModelAttributes::ModelAttributes() :    
    exportFlags(ToolkitUtil::ExportFlags(ToolkitUtil::FlipUVs)),
    scaleFactor(1.0f)
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
ModelAttributes::~ModelAttributes()
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
void
ModelAttributes::AddTake(const Ptr<Take>& take)
{
    this->takes.Append(take);
}

//------------------------------------------------------------------------------
/**
*/
const Ptr<Take>&
ModelAttributes::GetTake(uint index)
{
    return this->takes[index];
}

//------------------------------------------------------------------------------
/**
*/
const Ptr<Take>&
ModelAttributes::GetTake(const Util::String& name)
{
    IndexT i;
    for (i = 0; i < this->takes.Size(); i++)
    {
        if (name == this->takes[i]->GetName())
        {
            return takes[i];
        }
    }
    n_error("Дубль «%s» не найден!\n", name.AsCharPtr());

    // дурак компилятор
    return takes[0];
}

//------------------------------------------------------------------------------
/**
*/
const Array<Ptr<Take> >& 
ModelAttributes::GetTakes() const
{
    return this->takes;
}

//------------------------------------------------------------------------------
/**
*/
const bool
ModelAttributes::HasTake(const Ptr<Take>& take)
{
    IndexT i;
    for (i = 0; i < this->takes.Size(); i++)
    {
        if (take->GetName() == this->takes[i]->GetName())
        {
            return true;
        }
    }
    return false;
}

//------------------------------------------------------------------------------
/**
*/
const bool
ModelAttributes::HasTake(const Util::String& name)
{
    IndexT i;
    for (i = 0; i < this->takes.Size(); i++)
    {
        if (name == this->takes[i]->GetName())
        {
            return true;
        }
    }
    return false;
}

//------------------------------------------------------------------------------
/**
*/
void
ModelAttributes::SetState(const Util::String& node, const ToolkitUtil::State& state)
{
    if (!this->nodeStateMap.Contains(node))
    {
        this->nodeStateMap.Add(node, state);
    }
    else
    {
        this->nodeStateMap[node] = state;
    }
}

//------------------------------------------------------------------------------
/**
*/
const ToolkitUtil::State&
ModelAttributes::GetState(const Util::String& node)
{
    n_assert(this->nodeStateMap.Contains(node));
    return this->nodeStateMap[node];
}

//------------------------------------------------------------------------------
/**
*/
bool
ModelAttributes::HasState(const Util::String& node)
{
    return this->nodeStateMap.Contains(node);
}

//------------------------------------------------------------------------------
/**
*/
void
ModelAttributes::DeleteState(const Util::String& node)
{
    n_assert(this->nodeStateMap.Contains(node));
    this->nodeStateMap.Erase(node);
}

//------------------------------------------------------------------------------
/**
*/
void
ModelAttributes::SetEmitterAttrs(const Util::String& node, const Particles::EmitterAttrs& attrs)
{
    if (!this->particleAttrMap.Contains(node))
    {
        this->particleAttrMap.Add(node, attrs);
    }
    else
    {
        this->particleAttrMap[node] = attrs;
    }
}

//------------------------------------------------------------------------------
/**
*/
const Particles::EmitterAttrs&
ModelAttributes::GetEmitterAttrs(const Util::String& node)
{
    n_assert(this->particleAttrMap.Contains(node));
    return this->particleAttrMap[node];
}

//------------------------------------------------------------------------------
/**
*/
bool
ModelAttributes::HasEmitterAttrs(const Util::String& node)
{
    return this->particleAttrMap.Contains(node);
}

//------------------------------------------------------------------------------
/**
*/
void
ModelAttributes::DeleteEmitterAttrs(const Util::String& node)
{
    n_assert(this->particleAttrMap.Contains(node));
    this->particleAttrMap.Erase(node);
}

//------------------------------------------------------------------------------
/**
*/
void
ModelAttributes::SetEmitterMesh(const Util::String& node, const Util::String& mesh)
{
    if (!this->particleMeshMap.Contains(node))
    {
        this->particleMeshMap.Add(node, mesh);
    }
    else
    {
        this->particleMeshMap[node] = mesh;
    }
}

//------------------------------------------------------------------------------
/**
*/
const Util::String&
ModelAttributes::GetEmitterMesh(const Util::String& node)
{
    n_assert(this->particleMeshMap.Contains(node));
    return this->particleMeshMap[node];
}

//------------------------------------------------------------------------------
/**
*/
void
ModelAttributes::DeleteEmitterMesh(const Util::String& node)
{
    n_assert(this->particleMeshMap.Contains(node));
    this->particleMeshMap.Erase(node);
}

//------------------------------------------------------------------------------
/**
*/
void 
ModelAttributes::Save(const Ptr<IO::Stream>& stream)
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
        writer->SetInt("version", ModelAttributes::Version);

        // начни с написания флагов
        writer->BeginNode("Options");

        // написать варианты
        writer->SetInt("exportFlags", this->exportFlags);
        writer->SetFloat("scale", this->scaleFactor);

        // конечный узел опций
        writer->EndNode();

        if (this->nodeStateMap.Size() > 0)
        {
            // узел состояний записи
            writer->BeginNode("States");

            // пройтись по всем узлам и записать их состояния
            IndexT i;
            for (i = 0; i < this->nodeStateMap.Size(); i++)
            {
                // получить имя узла
                const String& node = this->nodeStateMap.KeyAtIndex(i);

                // получить состояние 
                const ToolkitUtil::State& state = this->nodeStateMap.ValueAtIndex(i);

                // написать узел модели
                writer->BeginNode("ModelNode");

                // установить имя
                writer->SetString("name", node);

                // набор материала
                writer->SetString("material", state.material);
                
                // конечный узел модели
                writer->EndNode();

            }

            // узел конечных состояний
            writer->EndNode();
        }

        if (this->takes.Size() > 0)
        {
            // начало принимает узел
            writer->BeginNode("Takes");

            // добавить каждый дубль
            IndexT i;
            for (i = 0; i < this->takes.Size(); i++)
            {
                // получить выбор из списка
                const Ptr<Take>& take = this->takes[i];

                // написать взять узел
                writer->BeginNode("Take");

                // установить название дубля
                writer->SetString("name", take->GetName());

                // пройти и добавить каждый клип
                IndexT j;
                for (j = 0; j < take->GetClips().Size(); j++)
                {
                    // получить клип
                    const Ptr<Clip>& clip = take->GetClips()[j];

                    // запись узла клипа
                    writer->BeginNode("Clip");

                    // установить название клипа
                    writer->SetString("name", clip->GetName());

                    // написать начальное значение
                    writer->SetInt("start", clip->GetStart());

                    // записать стоповое значение
                    writer->SetInt("stop", clip->GetEnd());

                    // написать пост с тегом бесконечности
                    writer->SetInt("post", clip->GetPostInfinity());

                    // написать тег до бесконечности
                    writer->SetInt("pre", clip->GetPreInfinity());

                    const Util::Array<Ptr<ClipEvent>>& events = clip->GetEvents();
                    IndexT k;
                    for (k = 0; k < events.Size(); k++)
                    {
                        // получить событие
                        const Ptr<ClipEvent>& event = events[k];

                        // запись узла события
                        writer->BeginNode("Event");

                        // установить имя и категорию клипа
                        writer->SetString("name", event->GetName());
                        writer->SetString("category", event->GetCategory());

                        // написать тип маркера и маркер
                        writer->SetInt("type", (int)event->GetMarkerType());
                        writer->SetInt("mark", event->GetMarker());

                        // конечное событие
                        writer->EndNode();
                    }

                    // конечный узел клипа
                    writer->EndNode();
                }

                // конец узла
                writer->EndNode();
            }

            // конец занимает узел
            writer->EndNode();
        }

        if (this->jointMasks.Size() > 0)
        {
            writer->BeginNode("Masks");

            IndexT i;
            for (i = 0; i < this->jointMasks.Size(); i++)
            {
                // получить маску из списка
                const JointMask& mask = this->jointMasks[i];
                const Util::String& name = mask.name;

                writer->BeginNode("Mask");
                writer->SetString("name", name);
                writer->SetInt("weights", mask.weights.Size());
                IndexT j;
                for (j = 0; j < mask.weights.Size(); j++)
                {
                    writer->BeginNode("Joint");
                    writer->SetInt("index", j);
                    writer->SetFloat("weight", mask.weights[j]);
                    writer->EndNode();
                }
                writer->EndNode();
            }
        }

        if (this->particleAttrMap.Size() > 0)
        {
            // писать частицы
            writer->BeginNode("Particles");

            IndexT j;
            for (j = 0; j < this->particleAttrMap.Size(); j++)
            {
                // получить узел
                const String& name = this->particleAttrMap.KeyAtIndex(j);

                // получить атрибуты
                const EmitterAttrs& attrs = this->particleAttrMap.ValueAtIndex(j);

                // получить сетку
                const String& mesh = this->particleMeshMap.ValueAtIndex(j);

                // написать частицу
                writer->BeginNode("Particle");

                // установить имя
                writer->SetString("name", name);

                // установить сетку
                writer->SetString("mesh", mesh);

                // получить кривые
                const EnvelopeCurve& emissionFrequency = attrs.GetEnvelope(EmitterAttrs::EmissionFrequency);
                const EnvelopeCurve& lifeTime = attrs.GetEnvelope(EmitterAttrs::LifeTime);
                const EnvelopeCurve& spreadMin = attrs.GetEnvelope(EmitterAttrs::SpreadMin);
                const EnvelopeCurve& spreadMax = attrs.GetEnvelope(EmitterAttrs::SpreadMax);
                const EnvelopeCurve& startVel = attrs.GetEnvelope(EmitterAttrs::StartVelocity);
                const EnvelopeCurve& rotationVel = attrs.GetEnvelope(EmitterAttrs::RotationVelocity);
                const EnvelopeCurve& size = attrs.GetEnvelope(EmitterAttrs::Size);
                const EnvelopeCurve& mass = attrs.GetEnvelope(EmitterAttrs::Mass);
                const EnvelopeCurve& timeMan = attrs.GetEnvelope(EmitterAttrs::TimeManipulator);
                const EnvelopeCurve& velFac = attrs.GetEnvelope(EmitterAttrs::VelocityFactor);
                const EnvelopeCurve& air = attrs.GetEnvelope(EmitterAttrs::AirResistance);
                const EnvelopeCurve& red = attrs.GetEnvelope(EmitterAttrs::Red);
                const EnvelopeCurve& green = attrs.GetEnvelope(EmitterAttrs::Green);
                const EnvelopeCurve& blue = attrs.GetEnvelope(EmitterAttrs::Blue);
                const EnvelopeCurve& alpha = attrs.GetEnvelope(EmitterAttrs::Alpha);

                // начать узел
                writer->BeginNode("EmissionFrequency");

                // теперь установите все атрибуты...
                writer->SetVec4("Values", Math::vec4(emissionFrequency.GetValues()[0],
                    emissionFrequency.GetValues()[1],
                    emissionFrequency.GetValues()[2],
                    emissionFrequency.GetValues()[3]));
                writer->SetFloat("Pos0", emissionFrequency.GetKeyPos0());
                writer->SetFloat("Pos1", emissionFrequency.GetKeyPos1());
                writer->SetFloat("Frequency", emissionFrequency.GetFrequency());
                writer->SetFloat("Amplitude", emissionFrequency.GetAmplitude());
                writer->SetInt("Function", emissionFrequency.GetModFunc());
                writer->SetVec2("Limits", Math::vec2(emissionFrequency.GetLimits()[0], emissionFrequency.GetLimits()[1]));

                // конечный узел
                writer->EndNode();

                // начать узел
                writer->BeginNode("LifeTime");

                writer->SetVec4("Values", Math::vec4(lifeTime.GetValues()[0],
                    lifeTime.GetValues()[1],
                    lifeTime.GetValues()[2],
                    lifeTime.GetValues()[3]));
                writer->SetFloat("Pos0", lifeTime.GetKeyPos0());
                writer->SetFloat("Pos1", lifeTime.GetKeyPos1());
                writer->SetFloat("Frequency", lifeTime.GetFrequency());
                writer->SetFloat("Amplitude", lifeTime.GetAmplitude());
                writer->SetInt("Function", lifeTime.GetModFunc());
                writer->SetVec2("Limits", Math::vec2(lifeTime.GetLimits()[0], lifeTime.GetLimits()[1]));

                // узел конечных значений
                writer->EndNode();

                // начать узел
                writer->BeginNode("SpreadMin");

                writer->SetVec4("Values", Math::vec4(spreadMin.GetValues()[0],
                    spreadMin.GetValues()[1],
                    spreadMin.GetValues()[2],
                    spreadMin.GetValues()[3]));
                writer->SetFloat("Pos0", spreadMin.GetKeyPos0());
                writer->SetFloat("Pos1", spreadMin.GetKeyPos1());
                writer->SetFloat("Frequency", spreadMin.GetFrequency());
                writer->SetFloat("Amplitude", spreadMin.GetAmplitude());
                writer->SetInt("Function", spreadMin.GetModFunc());
                writer->SetVec2("Limits", Math::vec2(spreadMin.GetLimits()[0], spreadMin.GetLimits()[1]));

                // узел конечных значений
                writer->EndNode();

                // начать узел
                writer->BeginNode("SpreadMax");

                writer->SetVec4("Values", Math::vec4(spreadMax.GetValues()[0],
                    spreadMax.GetValues()[1],
                    spreadMax.GetValues()[2],
                    spreadMax.GetValues()[3]));
                writer->SetFloat("Pos0", spreadMax.GetKeyPos0());
                writer->SetFloat("Pos1", spreadMax.GetKeyPos1());
                writer->SetFloat("Frequency", spreadMax.GetFrequency());
                writer->SetFloat("Amplitude", spreadMax.GetAmplitude());
                writer->SetInt("Function", spreadMax.GetModFunc());
                writer->SetVec2("Limits", Math::vec2(spreadMax.GetLimits()[0], spreadMax.GetLimits()[1]));

                // узел конечных значений
                writer->EndNode();

                // начать узел
                writer->BeginNode("StartVelocity");

                writer->SetVec4("Values", Math::vec4(startVel.GetValues()[0],
                    startVel.GetValues()[1],
                    startVel.GetValues()[2],
                    startVel.GetValues()[3]));
                writer->SetFloat("Pos0", startVel.GetKeyPos0());
                writer->SetFloat("Pos1", startVel.GetKeyPos1());
                writer->SetFloat("Frequency", startVel.GetFrequency());
                writer->SetFloat("Amplitude", startVel.GetAmplitude());
                writer->SetInt("Function", startVel.GetModFunc());
                writer->SetVec2("Limits", Math::vec2(startVel.GetLimits()[0], startVel.GetLimits()[1]));

                // узел конечных значений
                writer->EndNode();

                // начать узел
                writer->BeginNode("RotationVelocity");

                writer->SetVec4("Values", Math::vec4(rotationVel.GetValues()[0],
                    rotationVel.GetValues()[1],
                    rotationVel.GetValues()[2],
                    rotationVel.GetValues()[3]));
                writer->SetFloat("Pos0", rotationVel.GetKeyPos0());
                writer->SetFloat("Pos1", rotationVel.GetKeyPos1());
                writer->SetFloat("Frequency", rotationVel.GetFrequency());
                writer->SetFloat("Amplitude", rotationVel.GetAmplitude());
                writer->SetInt("Function", rotationVel.GetModFunc());
                writer->SetVec2("Limits", Math::vec2(rotationVel.GetLimits()[0], rotationVel.GetLimits()[1]));

                // узел конечных значений
                writer->EndNode();

                // начать узел
                writer->BeginNode("Size");

                writer->SetVec4("Values", Math::vec4(size.GetValues()[0],
                    size.GetValues()[1],
                    size.GetValues()[2],
                    size.GetValues()[3]));
                writer->SetFloat("Pos0", size.GetKeyPos0());
                writer->SetFloat("Pos1", size.GetKeyPos1());
                writer->SetFloat("Frequency", size.GetFrequency());
                writer->SetFloat("Amplitude", size.GetAmplitude());
                writer->SetInt("Function", size.GetModFunc());
                writer->SetVec2("Limits", Math::vec2(size.GetLimits()[0], size.GetLimits()[1]));

                // узел конечных значений
                writer->EndNode();

                // начать узел
                writer->BeginNode("Mass");

                writer->SetVec4("Values", Math::vec4(mass.GetValues()[0],
                    mass.GetValues()[1],
                    mass.GetValues()[2],
                    mass.GetValues()[3]));
                writer->SetFloat("Pos0", mass.GetKeyPos0());
                writer->SetFloat("Pos1", mass.GetKeyPos1());
                writer->SetFloat("Frequency", mass.GetFrequency());
                writer->SetFloat("Amplitude", mass.GetAmplitude());
                writer->SetInt("Function", mass.GetModFunc());
                writer->SetVec2("Limits", Math::vec2(mass.GetLimits()[0], mass.GetLimits()[1]));
                // узел конечных значений
                writer->EndNode();

                // начать узел
                writer->BeginNode("TimeManipulator");

                writer->SetVec4("Values", Math::vec4(timeMan.GetValues()[0],
                    timeMan.GetValues()[1],
                    timeMan.GetValues()[2],
                    timeMan.GetValues()[3]));
                writer->SetFloat("Pos0", timeMan.GetKeyPos0());
                writer->SetFloat("Pos1", timeMan.GetKeyPos1());
                writer->SetFloat("Frequency", timeMan.GetFrequency());
                writer->SetFloat("Amplitude", timeMan.GetAmplitude());
                writer->SetInt("Function", timeMan.GetModFunc());
                writer->SetVec2("Limits", Math::vec2(timeMan.GetLimits()[0], timeMan.GetLimits()[1]));

                // узел конечных значений
                writer->EndNode();

                // начать узел
                writer->BeginNode("VelocityFactor");

                writer->SetVec4("Values", Math::vec4(velFac.GetValues()[0],
                    velFac.GetValues()[1],
                    velFac.GetValues()[2],
                    velFac.GetValues()[3]));
                writer->SetFloat("Pos0", velFac.GetKeyPos0());
                writer->SetFloat("Pos1", velFac.GetKeyPos1());
                writer->SetFloat("Frequency", velFac.GetFrequency());
                writer->SetFloat("Amplitude", velFac.GetAmplitude());
                writer->SetInt("Function", velFac.GetModFunc());
                writer->SetVec2("Limits", Math::vec2(velFac.GetLimits()[0], velFac.GetLimits()[1]));

                // узел конечных значений
                writer->EndNode();

                // начать узел
                writer->BeginNode("AirResistance");

                writer->SetVec4("Values", Math::vec4(air.GetValues()[0],
                    air.GetValues()[1],
                    air.GetValues()[2],
                    air.GetValues()[3]));
                writer->SetFloat("Pos0", air.GetKeyPos0());
                writer->SetFloat("Pos1", air.GetKeyPos1());
                writer->SetFloat("Frequency", air.GetFrequency());
                writer->SetFloat("Amplitude", air.GetAmplitude());
                writer->SetInt("Function", air.GetModFunc());
                writer->SetVec2("Limits", Math::vec2(air.GetLimits()[0], air.GetLimits()[1]));

                // узел конечных значений
                writer->EndNode();

                // начать узел
                writer->BeginNode("Red");

                writer->SetVec4("Values", Math::vec4(red.GetValues()[0],
                    red.GetValues()[1],
                    red.GetValues()[2],
                    red.GetValues()[3]));
                writer->SetFloat("Pos0", red.GetKeyPos0());
                writer->SetFloat("Pos1", red.GetKeyPos1());
                writer->SetFloat("Frequency", red.GetFrequency());
                writer->SetFloat("Amplitude", red.GetAmplitude());
                writer->SetInt("Function", red.GetModFunc());
                writer->SetVec2("Limits", Math::vec2(red.GetLimits()[0], red.GetLimits()[1]));

                // узел конечных значений
                writer->EndNode();

                // начать узел
                writer->BeginNode("Green");

                writer->SetVec4("Values", Math::vec4(green.GetValues()[0],
                    green.GetValues()[1],
                    green.GetValues()[2],
                    green.GetValues()[3]));
                writer->SetFloat("Pos0", green.GetKeyPos0());
                writer->SetFloat("Pos1", green.GetKeyPos1());
                writer->SetFloat("Frequency", green.GetFrequency());
                writer->SetFloat("Amplitude", green.GetAmplitude());
                writer->SetInt("Function", green.GetModFunc());
                writer->SetVec2("Limits", Math::vec2(green.GetLimits()[0], green.GetLimits()[1]));

                // узел конечных значений
                writer->EndNode();

                // начать узел
                writer->BeginNode("Blue");

                writer->SetVec4("Values", Math::vec4(blue.GetValues()[0],
                    blue.GetValues()[1],
                    blue.GetValues()[2],
                    blue.GetValues()[3]));
                writer->SetFloat("Pos0", blue.GetKeyPos0());
                writer->SetFloat("Pos1", blue.GetKeyPos1());
                writer->SetFloat("Frequency", blue.GetFrequency());
                writer->SetFloat("Amplitude", blue.GetAmplitude());
                writer->SetInt("Function", blue.GetModFunc());
                writer->SetVec2("Limits", Math::vec2(blue.GetLimits()[0], blue.GetLimits()[1]));

                // узел конечных значений
                writer->EndNode();

                // начать узел
                writer->BeginNode("Alpha");

                writer->SetVec4("Values", Math::vec4(alpha.GetValues()[0],
                    alpha.GetValues()[1],
                    alpha.GetValues()[2],
                    alpha.GetValues()[3]));
                writer->SetFloat("Pos0", alpha.GetKeyPos0());
                writer->SetFloat("Pos1", alpha.GetKeyPos1());
                writer->SetFloat("Frequency", alpha.GetFrequency());
                writer->SetFloat("Amplitude", alpha.GetAmplitude());
                writer->SetInt("Function", alpha.GetModFunc());
                writer->SetVec2("Limits", Math::vec2(alpha.GetLimits()[0], alpha.GetLimits()[1]));

                // узел конечных значений
                writer->EndNode();

                // узел начальных значений
                writer->BeginNode("Values");

                // теперь напишите атрибуты с одним значением
                writer->SetFloat("EmissionDuration", attrs.GetFloat(EmitterAttrs::EmissionDuration));
                writer->SetBool("Looping", attrs.GetBool(EmitterAttrs::Looping));
                writer->SetFloat("ActivityDistance", attrs.GetFloat(EmitterAttrs::ActivityDistance));
                writer->SetBool("RenderOldestFirst", attrs.GetBool(EmitterAttrs::RenderOldestFirst));
                writer->SetBool("Billboard", attrs.GetBool(EmitterAttrs::Billboard));
                writer->SetFloat("StartRotationMin", attrs.GetFloat(EmitterAttrs::StartRotationMin));
                writer->SetFloat("StartRotationMax", attrs.GetFloat(EmitterAttrs::StartRotationMax));
                writer->SetFloat("Gravity", attrs.GetFloat(EmitterAttrs::Gravity));
                writer->SetFloat("ParticleStretch", attrs.GetFloat(EmitterAttrs::ParticleStretch));
                writer->SetFloat("TextureTile", attrs.GetFloat(EmitterAttrs::TextureTile));
                writer->SetBool("StretchToStart", attrs.GetBool(EmitterAttrs::StretchToStart));
                writer->SetFloat("VelocityRandomize", attrs.GetFloat(EmitterAttrs::VelocityRandomize));
                writer->SetFloat("RotationRandomize", attrs.GetFloat(EmitterAttrs::RotationRandomize));
                writer->SetFloat("SizeRandomize", attrs.GetFloat(EmitterAttrs::SizeRandomize));
                writer->SetFloat("PrecalcTime", attrs.GetFloat(EmitterAttrs::PrecalcTime));
                writer->SetBool("RandomizeRotation", attrs.GetBool(EmitterAttrs::RandomizeRotation));
                writer->SetInt("StretchDetail", attrs.GetInt(EmitterAttrs::StretchDetail));
                writer->SetBool("ViewAngleFade", attrs.GetBool(EmitterAttrs::ViewAngleFade));
                writer->SetFloat("StartDelay", attrs.GetFloat(EmitterAttrs::StartDelay));
                writer->SetFloat("PhasesPerSecond", attrs.GetFloat(EmitterAttrs::PhasesPerSecond));
                writer->SetInt("AnimPhases", attrs.GetInt(EmitterAttrs::AnimPhases));
                writer->SetVec4("WindDirection", attrs.GetVec4(EmitterAttrs::WindDirection));

                // узел конечных значений
                writer->EndNode();

                // конечная частица
                writer->EndNode();
            }

            // концевые частицы
            writer->EndNode();
        }

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
ModelAttributes::Load(const Ptr<IO::Stream>& stream)
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
        if (version != ModelAttributes::Version)
        {
            n_warning("Недопустимая версия атрибутов модели: %s\n", stream->GetURI().LocalPath().AsCharPtr());
            stream->Close();
            return;
        }

        // затем убедитесь, что у нас есть тег параметров
        n_assert2(reader->SetToFirstChild("Options"), "ПОВРЕЖДЕН ФАЙЛ .attributes!: Первый тег должен быть «Параметры»!");

        // теперь загрузим параметры
        this->exportFlags = (ToolkitUtil::ExportFlags)reader->GetInt("exportFlags");
        this->scaleFactor = reader->GetFloat("scale");

        // теперь вернитесь на один шаг назад и начните собирать узлы
        reader->SetToParent();

        // перейти к узлу состояний, если он существует
        if (reader->SetToFirstChild("States"))
        {
            // установлен на первый узел модели
            if (reader->SetToFirstChild("ModelNode")) do
            {
                // получить имя узла 
                String nodeName = reader->GetString("name");

                // получить состояние
                ToolkitUtil::State state;

                // набор материалов состояния
                state.material = reader->GetString("material");

                // теперь добавьте узел модели в атрибуты
                this->nodeStateMap.Add(nodeName, state);
            }
            while (reader->SetToNextChild("ModelNode"));

            // вернуться к родителю
            reader->SetToParent();
        }

        // теперь иди к метке
        if (reader->SetToFirstChild("Takes"))
        {
            // теперь перебираем каждый дубль
            if (reader->SetToFirstChild("Take")) do 
            {
                // создать новый дубль
                Ptr<Take> take = Take::Create();

                // установить название дубля
                take->SetName(reader->GetString("name"));

                // добавить дубль в список дублей
                this->takes.Append(take);

                // просмотреть каждый клип
                if (reader->SetToFirstChild("Clip")) do 
                {
                    // создать новый клип
                    Ptr<Clip> clip = Clip::Create();

                    // получить данные
                    clip->SetTake(take);
                    clip->SetName(reader->GetString("name"));
                    clip->SetStart(reader->GetInt("start"));
                    clip->SetEnd(reader->GetInt("stop"));
                    clip->SetPostInfinity((Clip::InfinityType)reader->GetInt("post"));
                    clip->SetPreInfinity((Clip::InfinityType)reader->GetInt("pre"));

                    // получать события
                    if (reader->SetToFirstChild("Event")) do 
                    {
                        // создать новое событие клипа
                        Ptr<ClipEvent> event = ClipEvent::Create();

                        // получить данные
                        event->SetName(reader->GetString("name"));
                        event->SetCategory(reader->GetString("category"));

                        // получить тип маркера и загрузить соответственно
                        ClipEvent::MarkerType type = (ClipEvent::MarkerType)reader->GetInt("type");
                        switch (type)
                        {
                        case ClipEvent::Ticks:
                            event->SetMarkerAsMilliseconds(reader->GetInt("mark"));
                            break;
                        case ClipEvent::Frames:
                            event->SetMarkerAsFrames(reader->GetInt("mark"));
                            break;
                        }

                        // добавить событие
                        clip->AddEvent(event);
                    } 
                    while (reader->SetToNextChild("Event"));

                    // добавить клип, чтобы его снять
                    take->AddClip(clip);
                } 
                while (reader->SetToNextChild("Clip"));

            }
            while (reader->SetToNextChild("Take"));

            // вернуться к родителю
            reader->SetToParent();
        }

        if (reader->SetToFirstChild("Masks"))
        {
            if (reader->SetToFirstChild("Mask")) do 
            {
                SizeT numWeights = reader->GetInt("weights");
                JointMask mask;
                mask.weights.Resize(numWeights);

                mask.name = reader->GetString("name");
                if (reader->SetToFirstChild("Joint")) do
                {
                    int index = reader->GetInt("index");
                    mask.weights[index] = reader->GetFloat("weight");
                } 
                while (reader->SetToNextChild("Joint"));

                // добавить в словарь
                this->jointMasks.Append(mask);
            } 
            while (reader->SetToNextChild("Mask"));
            reader->SetToParent();
        }

        // проходить через частицы
        if (reader->SetToFirstChild("Particles"))
        {
            // пройти через узлы частиц
            if (reader->SetToFirstChild("Particle")) do 
            {
                // получить имя узла
                String nodeName = reader->GetString("name");

                // получить имя сетки
                String mesh = reader->GetString("mesh");

                // создать новый узел
                EmitterAttrs attrs;

                // получить кривые
                EnvelopeCurve emissionFrequency = attrs.GetEnvelope(EmitterAttrs::EmissionFrequency);
                EnvelopeCurve lifeTime = attrs.GetEnvelope(EmitterAttrs::LifeTime);
                EnvelopeCurve spreadMin = attrs.GetEnvelope(EmitterAttrs::SpreadMin);
                EnvelopeCurve spreadMax = attrs.GetEnvelope(EmitterAttrs::SpreadMax);
                EnvelopeCurve startVel = attrs.GetEnvelope(EmitterAttrs::StartVelocity);
                EnvelopeCurve rotationVel = attrs.GetEnvelope(EmitterAttrs::RotationVelocity);
                EnvelopeCurve size = attrs.GetEnvelope(EmitterAttrs::Size);
                EnvelopeCurve mass = attrs.GetEnvelope(EmitterAttrs::Mass);
                EnvelopeCurve timeMan = attrs.GetEnvelope(EmitterAttrs::TimeManipulator);
                EnvelopeCurve velFac = attrs.GetEnvelope(EmitterAttrs::VelocityFactor);
                EnvelopeCurve air = attrs.GetEnvelope(EmitterAttrs::AirResistance);
                EnvelopeCurve red = attrs.GetEnvelope(EmitterAttrs::Red);
                EnvelopeCurve green = attrs.GetEnvelope(EmitterAttrs::Green);
                EnvelopeCurve blue = attrs.GetEnvelope(EmitterAttrs::Blue);
                EnvelopeCurve alpha = attrs.GetEnvelope(EmitterAttrs::Alpha);

                Math::vec4 values;
                Math::vec2 limits;
                float keypos0, keypos1, freq, amp;
                int func;

                // перейти к частоте излучения
                reader->SetToFirstChild("EmissionFrequency");

                // теперь получите все атрибуты...
                values = reader->GetVec4("Values");
                keypos0 = reader->GetFloat("Pos0");
                keypos1 = reader->GetFloat("Pos1");
                freq = reader->GetFloat("Frequency");
                amp = reader->GetFloat("Amplitude");
                func = reader->GetInt("Function");
                limits = reader->GetVec2("Limits");

                // кривая настройки
                emissionFrequency.Setup(values.x, values.y, values.z, values.w, keypos0, keypos1, freq, amp, (EnvelopeCurve::ModFunc)func);
                emissionFrequency.SetLimits(limits.x, limits.y);
                attrs.SetEnvelope(EmitterAttrs::EmissionFrequency, emissionFrequency);

                // вернуться к родительскому элементу
                reader->SetToParent();

                // перейти к частоте излучения
                reader->SetToFirstChild("LifeTime");

                // теперь получите все атрибуты...
                values = reader->GetVec4("Values");
                keypos0 = reader->GetFloat("Pos0");
                keypos1 = reader->GetFloat("Pos1");
                freq = reader->GetFloat("Frequency");
                amp = reader->GetFloat("Amplitude");
                func = reader->GetInt("Function");
                limits = reader->GetVec2("Limits");

                // кривая настройки
                lifeTime.Setup(values.x, values.y, values.z, values.w, keypos0, keypos1, freq, amp, (EnvelopeCurve::ModFunc)func);
                lifeTime.SetLimits(limits.x, limits.y);
                attrs.SetEnvelope(EmitterAttrs::LifeTime, lifeTime);

                // вернуться к родительскому элементу
                reader->SetToParent();

                // перейти к частоте излучения
                reader->SetToFirstChild("SpreadMin");

                // теперь получите все атрибуты...
                values = reader->GetVec4("Values");
                keypos0 = reader->GetFloat("Pos0");
                keypos1 = reader->GetFloat("Pos1");
                freq = reader->GetFloat("Frequency");
                amp = reader->GetFloat("Amplitude");
                func = reader->GetInt("Function");
                limits = reader->GetVec2("Limits");

                // кривая настройки
                spreadMin.Setup(values.x, values.y, values.z, values.w, keypos0, keypos1, freq, amp, (EnvelopeCurve::ModFunc)func);
                spreadMin.SetLimits(limits.x, limits.y);
                attrs.SetEnvelope(EmitterAttrs::SpreadMin, spreadMin);

                // вернуться к родительскому элементу
                reader->SetToParent();

                // перейти к частоте излучения
                reader->SetToFirstChild("SpreadMax");

                // теперь получите все атрибуты...
                values = reader->GetVec4("Values");
                keypos0 = reader->GetFloat("Pos0");
                keypos1 = reader->GetFloat("Pos1");
                freq = reader->GetFloat("Frequency");
                amp = reader->GetFloat("Amplitude");
                func = reader->GetInt("Function");
                limits = reader->GetVec2("Limits");

                // кривая настройки
                spreadMax.Setup(values.x, values.y, values.z, values.w, keypos0, keypos1, freq, amp, (EnvelopeCurve::ModFunc)func);
                spreadMax.SetLimits(limits.x, limits.y);
                attrs.SetEnvelope(EmitterAttrs::SpreadMax, spreadMax);

                // вернуться к родительскому элементу
                reader->SetToParent();

                // перейти к частоте излучения
                reader->SetToFirstChild("StartVelocity");

                // теперь получите все атрибуты...
                values = reader->GetVec4("Values");
                keypos0 = reader->GetFloat("Pos0");
                keypos1 = reader->GetFloat("Pos1");
                freq = reader->GetFloat("Frequency");
                amp = reader->GetFloat("Amplitude");
                func = reader->GetInt("Function");
                limits = reader->GetVec2("Limits");

                // кривая настройки
                startVel.Setup(values.x, values.y, values.z, values.w, keypos0, keypos1, freq, amp, (EnvelopeCurve::ModFunc)func);
                startVel.SetLimits(limits.x, limits.y);
                attrs.SetEnvelope(EmitterAttrs::StartVelocity, startVel);

                // вернуться к родительскому элементу
                reader->SetToParent();

                // перейти к частоте излучения
                reader->SetToFirstChild("RotationVelocity");

                // теперь получите все атрибуты...
                values = reader->GetVec4("Values");
                keypos0 = reader->GetFloat("Pos0");
                keypos1 = reader->GetFloat("Pos1");
                freq = reader->GetFloat("Frequency");
                amp = reader->GetFloat("Amplitude");
                func = reader->GetInt("Function");
                limits = reader->GetVec2("Limits");

                // кривая настройки
                rotationVel.Setup(values.x, values.y, values.z, values.w, keypos0, keypos1, freq, amp, (EnvelopeCurve::ModFunc)func);
                rotationVel.SetLimits(limits.x, limits.y);
                attrs.SetEnvelope(EmitterAttrs::RotationVelocity, rotationVel);

                // вернуться к родительскому элементу
                reader->SetToParent();

                // перейти к частоте излучения
                reader->SetToFirstChild("Size");

                // теперь получите все атрибуты...
                values = reader->GetVec4("Values");
                keypos0 = reader->GetFloat("Pos0");
                keypos1 = reader->GetFloat("Pos1");
                freq = reader->GetFloat("Frequency");
                amp = reader->GetFloat("Amplitude");
                func = reader->GetInt("Function");
                limits = reader->GetVec2("Limits");

                // кривая настройки
                size.Setup(values.x, values.y, values.z, values.w, keypos0, keypos1, freq, amp, (EnvelopeCurve::ModFunc)func);
                size.SetLimits(limits.x, limits.y);
                attrs.SetEnvelope(EmitterAttrs::Size, size);

                // вернуться к родительскому элементу
                reader->SetToParent();

                // перейти к частоте излучения
                reader->SetToFirstChild("Mass");

                // теперь получите все атрибуты...
                values = reader->GetVec4("Values");
                keypos0 = reader->GetFloat("Pos0");
                keypos1 = reader->GetFloat("Pos1");
                freq = reader->GetFloat("Frequency");
                amp = reader->GetFloat("Amplitude");
                func = reader->GetInt("Function");
                limits = reader->GetVec2("Limits");

                // кривая настройки
                mass.Setup(values.x, values.y, values.z, values.w, keypos0, keypos1, freq, amp, (EnvelopeCurve::ModFunc)func);
                mass.SetLimits(limits.x, limits.y);
                attrs.SetEnvelope(EmitterAttrs::Mass, mass);

                // вернуться к родительскому элементу
                reader->SetToParent();

                // перейти к частоте излучения
                reader->SetToFirstChild("TimeManipulator");

                // теперь получите все атрибуты...
                values = reader->GetVec4("Values");
                keypos0 = reader->GetFloat("Pos0");
                keypos1 = reader->GetFloat("Pos1");
                freq = reader->GetFloat("Frequency");
                amp = reader->GetFloat("Amplitude");
                func = reader->GetInt("Function");
                limits = reader->GetVec2("Limits");

                // кривая настройки
                timeMan.Setup(values.x, values.y, values.z, values.w, keypos0, keypos1, freq, amp, (EnvelopeCurve::ModFunc)func);
                timeMan.SetLimits(limits.x, limits.y);
                attrs.SetEnvelope(EmitterAttrs::Mass, mass);

                // вернуться к родительскому элементу
                reader->SetToParent();

                // перейти к частоте излучения
                reader->SetToFirstChild("VelocityFactor");

                // теперь получите все атрибуты...
                values = reader->GetVec4("Values");
                keypos0 = reader->GetFloat("Pos0");
                keypos1 = reader->GetFloat("Pos1");
                freq = reader->GetFloat("Frequency");
                amp = reader->GetFloat("Amplitude");
                func = reader->GetInt("Function");
                limits = reader->GetVec2("Limits");

                // кривая настройки
                velFac.Setup(values.x, values.y, values.z, values.w, keypos0, keypos1, freq, amp, (EnvelopeCurve::ModFunc)func);
                velFac.SetLimits(limits.x, limits.y);
                attrs.SetEnvelope(EmitterAttrs::VelocityFactor, velFac);

                // вернуться к родительскому элементу
                reader->SetToParent();

                // перейти к частоте излучения
                reader->SetToFirstChild("AirResistance");

                // теперь получите все атрибуты...
                values = reader->GetVec4("Values");
                keypos0 = reader->GetFloat("Pos0");
                keypos1 = reader->GetFloat("Pos1");
                freq = reader->GetFloat("Frequency");
                amp = reader->GetFloat("Amplitude");
                func = reader->GetInt("Function");
                limits = reader->GetVec2("Limits");

                // кривая настройки
                air.Setup(values.x, values.y, values.z, values.w, keypos0, keypos1, freq, amp, (EnvelopeCurve::ModFunc)func);
                air.SetLimits(limits.x, limits.y);
                attrs.SetEnvelope(EmitterAttrs::AirResistance, air);

                // вернуться к родительскому элементу
                reader->SetToParent();

                // перейти к частоте излучения
                reader->SetToFirstChild("Red");

                // теперь получите все атрибуты...
                values = reader->GetVec4("Values");
                keypos0 = reader->GetFloat("Pos0");
                keypos1 = reader->GetFloat("Pos1");
                freq = reader->GetFloat("Frequency");
                amp = reader->GetFloat("Amplitude");
                func = reader->GetInt("Function");
                limits = reader->GetVec2("Limits");

                // кривая настройки
                red.Setup(values.x, values.y, values.z, values.w, keypos0, keypos1, freq, amp, (EnvelopeCurve::ModFunc)func);
                red.SetLimits(limits.x, limits.y);
                attrs.SetEnvelope(EmitterAttrs::Red, red);

                // вернуться к родительскому элементу
                reader->SetToParent();

                // перейти к частоте излучения
                reader->SetToFirstChild("Green");

                // теперь получите все атрибуты...
                values = reader->GetVec4("Values");
                keypos0 = reader->GetFloat("Pos0");
                keypos1 = reader->GetFloat("Pos1");
                freq = reader->GetFloat("Frequency");
                amp = reader->GetFloat("Amplitude");
                func = reader->GetInt("Function");
                limits = reader->GetVec2("Limits");

                // кривая настройки
                green.Setup(values.x, values.y, values.z, values.w, keypos0, keypos1, freq, amp, (EnvelopeCurve::ModFunc)func);
                green.SetLimits(limits.x, limits.y);
                attrs.SetEnvelope(EmitterAttrs::Green, green);

                // вернуться к родительскому элементу
                reader->SetToParent();

                // перейти к частоте излучения
                reader->SetToFirstChild("Blue");

                // теперь получите все атрибуты...
                values = reader->GetVec4("Values");
                keypos0 = reader->GetFloat("Pos0");
                keypos1 = reader->GetFloat("Pos1");
                freq = reader->GetFloat("Frequency");
                amp = reader->GetFloat("Amplitude");
                func = reader->GetInt("Function");
                limits = reader->GetVec2("Limits");

                // кривая настройки
                blue.Setup(values.x, values.y, values.z, values.w, keypos0, keypos1, freq, amp, (EnvelopeCurve::ModFunc)func);
                blue.SetLimits(limits.x, limits.y);
                attrs.SetEnvelope(EmitterAttrs::Blue, blue);

                // вернуться к родительскому элементу
                reader->SetToParent();

                // перейти к частоте излучения
                reader->SetToFirstChild("Alpha");

                // теперь получите все атрибуты...
                values = reader->GetVec4("Values");
                keypos0 = reader->GetFloat("Pos0");
                keypos1 = reader->GetFloat("Pos1");
                freq = reader->GetFloat("Frequency");
                amp = reader->GetFloat("Amplitude");
                func = reader->GetInt("Function");
                limits = reader->GetVec2("Limits");

                // кривая настройки
                alpha.Setup(values.x, values.y, values.z, values.w, keypos0, keypos1, freq, amp, (EnvelopeCurve::ModFunc)func);
                alpha.SetLimits(limits.x, limits.y);
                attrs.SetEnvelope(EmitterAttrs::Alpha, alpha);

                // вернуться к родительскому элементу
                reader->SetToParent();

                // перейти к частоте излучения
                reader->SetToFirstChild("Values");

                // теперь напишите атрибуты с одним значением
                float duration = reader->GetFloat("EmissionDuration");
                bool looping = reader->GetBool("Looping");
                float activityDistance = reader->GetFloat("ActivityDistance");
                bool oldestFirst = reader->GetBool("RenderOldestFirst");
                bool billboard = reader->GetBool("Billboard");
                float startRotationMin = reader->GetFloat("StartRotationMin");
                float startRotationMax = reader->GetFloat("StartRotationMax");
                float gravity = reader->GetFloat("Gravity");
                float particleStretch = reader->GetFloat("ParticleStretch");
                float textureTile = reader->GetFloat("TextureTile");
                bool stretchToStart = reader->GetBool("StretchToStart");
                float velocityRandomize = reader->GetFloat("VelocityRandomize");
                float rotationRandomize = reader->GetFloat("RotationRandomize");
                float sizeRandomize = reader->GetFloat("SizeRandomize");
                float precalcTime = reader->GetFloat("PrecalcTime");
                bool randomizeRotation = reader->GetBool("RandomizeRotation");
                int stretchDetail = reader->GetInt("StretchDetail");
                bool viewAngleFade = reader->GetBool("ViewAngleFade");
                float startDelay = reader->GetFloat("StartDelay");
                float phasesPerSecond = reader->GetFloat("PhasesPerSecond");
                int animPhases = reader->GetInt("AnimPhases");
                Math::vec4 windDirection = reader->GetVec4("WindDirection");

                // теперь установите атрибуты...
                attrs.SetFloat(EmitterAttrs::EmissionDuration, duration);
                attrs.SetBool(EmitterAttrs::Looping, looping);
                attrs.SetFloat(EmitterAttrs::ActivityDistance, activityDistance);
                attrs.SetBool(EmitterAttrs::RenderOldestFirst, oldestFirst);
                attrs.SetBool(EmitterAttrs::Billboard, billboard);
                attrs.SetFloat(EmitterAttrs::StartRotationMin, startRotationMin);
                attrs.SetFloat(EmitterAttrs::StartRotationMax, startRotationMax);
                attrs.SetFloat(EmitterAttrs::Gravity, gravity);
                attrs.SetFloat(EmitterAttrs::ParticleStretch, particleStretch);
                attrs.SetFloat(EmitterAttrs::TextureTile, textureTile);
                attrs.SetBool(EmitterAttrs::StretchToStart, stretchToStart);
                attrs.SetFloat(EmitterAttrs::VelocityRandomize, velocityRandomize);
                attrs.SetFloat(EmitterAttrs::RotationRandomize, rotationRandomize);
                attrs.SetFloat(EmitterAttrs::SizeRandomize, sizeRandomize);
                attrs.SetFloat(EmitterAttrs::PrecalcTime, precalcTime);
                attrs.SetBool(EmitterAttrs::RandomizeRotation, randomizeRotation);
                attrs.SetInt(EmitterAttrs::StretchDetail, stretchDetail);
                attrs.SetBool(EmitterAttrs::ViewAngleFade, viewAngleFade);
                attrs.SetFloat(EmitterAttrs::StartDelay, startDelay);
                attrs.SetFloat(EmitterAttrs::PhasesPerSecond, phasesPerSecond);
                attrs.SetInt(EmitterAttrs::AnimPhases, animPhases);
                attrs.SetVec4(EmitterAttrs::WindDirection, windDirection);

                // добавить узел к узлам
                this->particleAttrMap.Add(nodeName, attrs);

                // добавить имя сетки к узлам
                this->particleMeshMap.Add(nodeName, mesh);

                // вернуться к родительскому элементу
                reader->SetToParent();
            } 
            while (reader->SetToNextChild("Particle"));

            // перейти к родителю
            reader->SetToParent();
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
ModelAttributes::Clear()
{
    this->particleAttrMap.Clear();
    this->particleMeshMap.Clear();
    this->nodeStateMap.Clear();
    this->jointMasks.Clear();

    IndexT i;
    for (i = 0; i < this->takes.Size(); i++)
    {
        this->takes[i]->Cleanup();
    }

    // очистка занимает
    this->ClearTakes();
}

//------------------------------------------------------------------------------
/**
*/
void 
ModelAttributes::ClearTakes()
{
    // очистка занимает
    this->takes.Clear();
}

} // Импортер пространства имен