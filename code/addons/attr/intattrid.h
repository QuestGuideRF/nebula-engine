#pragma once
#include "attrid.h"
namespace Attr
{
class IntAttrId : public AttrId
{
public:
    IntAttrId(const AttrId& rhs);
    IntAttrId(const AttributeDefinition<IntTypeClass,int>& rhs);
    IntAttrId(const Util::String& rhs);
    IntAttrId(const Util::FourCC& rhs);
    bool operator==(const IntAttrId& rhs) const;
    bool operator!=(const IntAttrId& rhs) const;
};
inline
IntAttrId::IntAttrId(const AttrId& rhs) :
    AttrId(rhs)
{
}
inline
IntAttrId::IntAttrId(const AttributeDefinition<IntTypeClass,int>& rhs) :
    AttrId(&rhs)
{
}
inline
IntAttrId::IntAttrId(const Util::String& rhs)
{
    this->defPtr = AttributeDefinitionBase::FindByName(rhs);
    n_assert(this->GetValueType() == IntType);
}
inline
IntAttrId::IntAttrId(const Util::FourCC& rhs)
{
    this->defPtr = AttributeDefinitionBase::FindByFourCC(rhs);
    n_assert(this->GetValueType() == IntType);
}
inline
bool
IntAttrId::operator==(const IntAttrId& rhs) const
{
    n_assert((0 != this->defPtr) && (0 != rhs.defPtr));
    return (this->defPtr == rhs.defPtr);
}
inline
bool
IntAttrId::operator!=(const IntAttrId& rhs) const
{
    n_assert((0 != this->defPtr) && (0 != rhs.defPtr));
    return (this->defPtr != rhs.defPtr);
}
}