#pragma once
#include "attrid.h"
namespace Attr
{
class UIntAttrId : public AttrId
{
public:
    UIntAttrId(const AttrId& rhs);
    UIntAttrId(const AttributeDefinition<IntTypeClass,int>& rhs);
    UIntAttrId(const Util::String& rhs);
    UIntAttrId(const Util::FourCC& rhs);
    bool operator==(const UIntAttrId& rhs) const;
    bool operator!=(const UIntAttrId& rhs) const;
};
inline
UIntAttrId::UIntAttrId(const AttrId& rhs) :
    AttrId(rhs)
{
}
inline
UIntAttrId::UIntAttrId(const AttributeDefinition<IntTypeClass,int>& rhs) :
    AttrId(&rhs)
{
}
inline
UIntAttrId::UIntAttrId(const Util::String& rhs)
{
    this->defPtr = AttributeDefinitionBase::FindByName(rhs);
    n_assert(this->GetValueType() == UIntType);
}
inline
UIntAttrId::UIntAttrId(const Util::FourCC& rhs)
{
    this->defPtr = AttributeDefinitionBase::FindByFourCC(rhs);
    n_assert(this->GetValueType() == UIntType);
}
inline
bool
UIntAttrId::operator==(const UIntAttrId& rhs) const
{
    n_assert((0 != this->defPtr) && (0 != rhs.defPtr));
    return (this->defPtr == rhs.defPtr);
}
inline
bool
UIntAttrId::operator!=(const UIntAttrId& rhs) const
{
    n_assert((0 != this->defPtr) && (0 != rhs.defPtr));
    return (this->defPtr != rhs.defPtr);
}
}