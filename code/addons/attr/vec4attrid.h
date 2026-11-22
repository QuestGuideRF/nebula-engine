#pragma once
#include "attrid.h"
namespace Attr
{
class Vec4AttrId : public AttrId
{
public:
    Vec4AttrId(const AttrId& rhs);
    Vec4AttrId(const AttributeDefinition<Float4TypeClass, const Math::vec4&>& rhs);
    Vec4AttrId(const Util::String& rhs);
    Vec4AttrId(const Util::FourCC& rhs);
    bool operator==(const Vec4AttrId& rhs) const;
    bool operator!=(const Vec4AttrId& rhs) const;
};
inline
Vec4AttrId::Vec4AttrId(const AttrId& rhs) :
    AttrId(rhs)
{
}
inline
Vec4AttrId::Vec4AttrId(const AttributeDefinition<Float4TypeClass, const Math::vec4&>& rhs) :
    AttrId(&rhs)
{
}
inline
Vec4AttrId::Vec4AttrId(const Util::String& rhs)
{
    this->defPtr = AttributeDefinitionBase::FindByName(rhs);
    n_assert(this->GetValueType() == Vec4Type);
}
inline
Vec4AttrId::Vec4AttrId(const Util::FourCC& rhs)
{
    this->defPtr = AttributeDefinitionBase::FindByFourCC(rhs);
    n_assert(this->GetValueType() == Vec4Type);
}
inline
bool
Vec4AttrId::operator==(const Vec4AttrId& rhs) const
{
    n_assert((0 != this->defPtr) && (0 != rhs.defPtr));
    return (this->defPtr == rhs.defPtr);
}
inline
bool
Vec4AttrId::operator!=(const Vec4AttrId& rhs) const
{
    n_assert((0 != this->defPtr) && (0 != rhs.defPtr));
    return (this->defPtr != rhs.defPtr);
}
}