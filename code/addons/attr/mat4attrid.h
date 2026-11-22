#pragma once
#include "attrid.h"
namespace Attr
{
class Mat4AttrId : public AttrId
{
public:
    Mat4AttrId(const AttrId& rhs);
    Mat4AttrId(const AttributeDefinition<Matrix44TypeClass,const Math::mat4&>& rhs);
    Mat4AttrId(const Util::String& rhs);
    Mat4AttrId(const Util::FourCC& rhs);
    bool operator==(const Mat4AttrId& rhs) const;
    bool operator!=(const Mat4AttrId& rhs) const;
};
inline
Mat4AttrId::Mat4AttrId(const AttrId& rhs) :
    AttrId(rhs)
{
}
inline
Mat4AttrId::Mat4AttrId(const AttributeDefinition<Matrix44TypeClass,const Math::mat4&>& rhs) :
    AttrId(&rhs)
{
}
inline
Mat4AttrId::Mat4AttrId(const Util::String& rhs)
{
    this->defPtr = AttributeDefinitionBase::FindByName(rhs);
    n_assert(this->GetValueType() == Mat4Type);
}
inline
Mat4AttrId::Mat4AttrId(const Util::FourCC& rhs)
{
    this->defPtr = AttributeDefinitionBase::FindByFourCC(rhs);
    n_assert(this->GetValueType() == Mat4Type);
}
inline
bool
Mat4AttrId::operator==(const Mat4AttrId& rhs) const
{
    n_assert((0 != this->defPtr) && (0 != rhs.defPtr));
    return (this->defPtr == rhs.defPtr);
}
inline
bool
Mat4AttrId::operator!=(const Mat4AttrId& rhs) const
{
    n_assert((0 != this->defPtr) && (0 != rhs.defPtr));
    return (this->defPtr != rhs.defPtr);
}
}