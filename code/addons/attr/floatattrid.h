#pragma once
#include "attrid.h"
namespace Attr
{
class FloatAttrId : public AttrId
{
public:
    FloatAttrId(const AttrId& rhs);
    FloatAttrId(const AttributeDefinition<FloatTypeClass,float>& rhs);
    FloatAttrId(const Util::String& rhs);
    FloatAttrId(const Util::FourCC& rhs);
    bool operator==(const FloatAttrId& rhs) const;
    bool operator!=(const FloatAttrId& rhs) const;
};
inline
FloatAttrId::FloatAttrId(const AttrId& rhs) :
    AttrId(rhs)
{
}
inline
FloatAttrId::FloatAttrId(const AttributeDefinition<FloatTypeClass, float>& rhs) :
    AttrId(&rhs)
{
}
inline
FloatAttrId::FloatAttrId(const Util::String& rhs)
{
    this->defPtr = AttributeDefinitionBase::FindByName(rhs);
    n_assert(this->GetValueType() == FloatType);
}
inline
FloatAttrId::FloatAttrId(const Util::FourCC& rhs)
{
    this->defPtr = AttributeDefinitionBase::FindByFourCC(rhs);
    n_assert(this->GetValueType() == FloatType);
}
inline
bool
FloatAttrId::operator==(const FloatAttrId& rhs) const
{
    n_assert((0 != this->defPtr) && (0 != rhs.defPtr));
    return (this->defPtr == rhs.defPtr);
}
inline
bool
FloatAttrId::operator!=(const FloatAttrId& rhs) const
{
    n_assert((0 != this->defPtr) && (0 != rhs.defPtr));
    return (this->defPtr != rhs.defPtr);
}
}