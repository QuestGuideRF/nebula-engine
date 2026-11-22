//------------------------------------------------------------------------------
// modelnode.cc
// (C) 2017-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "modelnode.h"

using namespace Util;
using namespace Math;
namespace Models
{

IndexT ModelNode::ModelNodeUniqueIdCounter = 0;
//------------------------------------------------------------------------------
/**
*/
ModelNode::ModelNode() :
    uniqueId(ModelNodeUniqueIdCounter++)
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
ModelNode::~ModelNode()
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
bool
ModelNode::Load(const Util::FourCC& fourcc, const Util::StringAtom& tag, const Ptr<IO::BinaryReader>& reader, bool immediate)
{
    if (FourCC('LBOX') == fourcc)
    {
        // ограничивающая рамка
        vec3 center = xyz(reader->ReadVec4());
        vec3 extents = xyz(reader->ReadVec4());
        this->boundingBox.set(center, extents);
    }
    else if (FourCC('MNTP') == fourcc)
    {
        // тип узла модели, устаревший
        reader->ReadString();
    }

    else if (FourCC('SSTA') == fourcc)
    {
        // строковый атрибут, устаревший
        UNUSED(StringAtom) key = reader->ReadString();
        UNUSED(String) value = reader->ReadString();
        //это->SetStringAttr(ключ, значение);
    }
    else
    {
        // ошибка выдачи неизвестного тега (мы не можем пропускать неизвестные теги)
        n_error("ModelNode::Load: неизвестный тег данных «%s» в «%s»!",
            fourcc.AsString().AsCharPtr(),
            reader->GetStream()->GetURI().AsString().AsCharPtr());
        return false;
    }
    return true;
}

//------------------------------------------------------------------------------
/**
*/
void
ModelNode::Unload()
{
    // переопределить в подклассе
}

//------------------------------------------------------------------------------
/**
*/
void
ModelNode::OnFinishedLoading(ModelStreamingData* streamingData)
{
    this->uniqueId = ModelNodeUniqueIdCounter++;
}

//------------------------------------------------------------------------------
/**
*/
void
ModelNode::Discard()
{
    // переопределить в подклассе
}

//------------------------------------------------------------------------------
/**
*/
bool 
ModelNode::GetImplicitHierarchyActivation() const
{
    return true;
}

} // Модели пространства имен
