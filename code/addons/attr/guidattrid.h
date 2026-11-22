#pragma once
#include "attrid.h"
namespace Attr
{
class GuidAttrId : public AttrId
{
public:
    GuidAttrId(const AttrId& rhs);
    GuidAttrId(const AttributeDefinition<GuidTypeClass, Util::Guid>& rhs);
    GuidAttrId(const Util::String& rhs);
    GuidAttrId(const Util::FourCC& rhs);
    bool operator==(const GuidAttrId& rhs) const;
    bool operator!=(const GuidAttrId& rhs) const;
};
inline
GuidAttrId::GuidAttrId(const AttrId& rhs) :
    AttrId(rhs)
{
}
inline
GuidAttrId::GuidAttrId(const AttributeDefinition<GuidTypeClass, Util::Guid>& rhs) :
    AttrId(&rhs)
{
}
inline
GuidAttrId::GuidAttrId(const Util::String& rhs)
{
    this->defPtr = AttributeDefinitionBase::FindByName(rhs);
    n_assert(this->GetValueType() == GuidType);
}
inline
GuidAttrId::GuidAttrId(const Util::FourCC& rhs)
{
    this->defPtr = AttributeDefinitionBase::FindByFourCC(rhs);
    n_assert(this->GetValueType() == GuidType);
}
inline
bool
GuidAttrId::operator==(const GuidAttrId& rhs) const
{
    n_assert((0 != this->defPtr) && (0 != rhs.defPtr));
    return (this->defPtr == rhs.defPtr);
}
inline
bool
GuidAttrId::operator!=(const GuidAttrId& rhs) const
{
    n_assert((0 != this->defPtr) && (0 != rhs.defPtr));
    return (this->defPtr != rhs.defPtr);
}
}