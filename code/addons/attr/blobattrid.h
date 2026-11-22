#pragma once
#include "attrid.h"
namespace Attr
{
class BlobAttrId : public AttrId
{
public:
    BlobAttrId(const AttrId& rhs);
    BlobAttrId(const AttributeDefinition<BlobTypeClass,const Util::Blob&>& rhs);
    BlobAttrId(const Util::String& rhs);
    BlobAttrId(const Util::FourCC& rhs);
    bool operator==(const BlobAttrId& rhs) const;
    bool operator!=(const BlobAttrId& rhs) const;
};
inline
BlobAttrId::BlobAttrId(const AttrId& rhs) :
    AttrId(rhs)
{
}
inline
BlobAttrId::BlobAttrId(const AttributeDefinition<BlobTypeClass,const Util::Blob&>& rhs) :
    AttrId(&rhs)
{
}
inline
BlobAttrId::BlobAttrId(const Util::String& rhs)
{
    this->defPtr = AttributeDefinitionBase::FindByName(rhs);
    n_assert(this->GetValueType() == BlobType);
}
inline
BlobAttrId::BlobAttrId(const Util::FourCC& rhs)
{
    this->defPtr = AttributeDefinitionBase::FindByFourCC(rhs);
    n_assert(this->GetValueType() == BlobType);
}
inline
bool
BlobAttrId::operator==(const BlobAttrId& rhs) const
{
    n_assert((0 != this->defPtr) && (0 != rhs.defPtr));
    return (this->defPtr == rhs.defPtr);
}
inline
bool
BlobAttrId::operator!=(const BlobAttrId& rhs) const
{
    n_assert((0 != this->defPtr) && (0 != rhs.defPtr));
    return (this->defPtr != rhs.defPtr);
}
}