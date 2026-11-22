#pragma once
#include "attrid.h"
namespace Attr
{
class BoolAttrId : public AttrId
{
public:
    BoolAttrId(const AttrId& rhs);
    BoolAttrId(const AttributeDefinition<BoolTypeClass, bool>& rhs);
    BoolAttrId(const Util::String& rhs);
    BoolAttrId(const Util::FourCC& rhs);
    bool operator==(const BoolAttrId& rhs) const;
    bool operator!=(const BoolAttrId& rhs) const;
};
inline
BoolAttrId::BoolAttrId(const AttrId& rhs) :
    AttrId(rhs)
{
}
inline
BoolAttrId::BoolAttrId(const AttributeDefinition<BoolTypeClass, bool>& rhs) :
    AttrId(&rhs)
{
}
inline
BoolAttrId::BoolAttrId(const Util::String& rhs)
{
    this->defPtr = AttributeDefinitionBase::FindByName(rhs);
    n_assert(this->GetValueType() == BoolType);
}
inline
BoolAttrId::BoolAttrId(const Util::FourCC& rhs)
{
    this->defPtr = AttributeDefinitionBase::FindByFourCC(rhs);
    n_assert(this->GetValueType() == BoolType);
}
inline
bool
BoolAttrId::operator==(const BoolAttrId& rhs) const
{
    n_assert((0 != this->defPtr) && (0 != rhs.defPtr));
    return (this->defPtr == rhs.defPtr);
}
inline
bool
BoolAttrId::operator!=(const BoolAttrId& rhs) const
{
    n_assert((0 != this->defPtr) && (0 != rhs.defPtr));
    return (this->defPtr != rhs.defPtr);
}
}