#pragma once
#include "stringattrid.h"
namespace Attr
{
class StringAttrId : public AttrId
{
public:
    StringAttrId(const AttrId& rhs);
    StringAttrId(const AttributeDefinition<StringTypeClass, Util::String>& rhs);
    StringAttrId(const Util::String& rhs);
    StringAttrId(const Util::FourCC& rhs);
    bool operator==(const StringAttrId& rhs) const;
    bool operator!=(const StringAttrId& rhs) const;
};
inline
StringAttrId::StringAttrId(const AttrId& rhs) :
    AttrId(rhs)
{
}
inline
StringAttrId::StringAttrId(const AttributeDefinition<StringTypeClass, Util::String>& rhs) :
    AttrId(&rhs)
{
}
inline
StringAttrId::StringAttrId(const Util::String& rhs)
{
    this->defPtr = AttributeDefinitionBase::FindByName(rhs);
    n_assert(this->GetValueType() == StringType);
}
inline
StringAttrId::StringAttrId(const Util::FourCC& rhs)
{
    this->defPtr = AttributeDefinitionBase::FindByFourCC(rhs);
    n_assert(this->GetValueType() == StringType);
}
inline
bool
StringAttrId::operator==(const StringAttrId& rhs) const
{
    n_assert((0 != this->defPtr) && (0 != rhs.defPtr));
    return (this->defPtr == rhs.defPtr);
}
inline
bool
StringAttrId::operator!=(const StringAttrId& rhs) const
{
    n_assert((0 != this->defPtr) && (0 != rhs.defPtr));
    return (this->defPtr != rhs.defPtr);
}
}