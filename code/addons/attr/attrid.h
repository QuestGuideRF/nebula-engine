#pragma once
#include "attributedefinition.h"
namespace Attr
{
class AttrId
{
public:
    AttrId();
    AttrId(const AttrId& rhs);
    AttrId(const AttributeDefinitionBase& def);
    AttrId(const AttributeDefinitionBase* ptr);
    AttrId(const Util::String& name);
    AttrId(const Util::FourCC& fcc);
    static bool IsValidName(const Util::String& n);
    static bool IsValidFourCC(const Util::FourCC& fcc);
    bool operator==(const AttrId& rhs) const;
    bool operator!=(const AttrId& rhs) const;
    bool operator>(const AttrId& rhs) const;
    bool operator>=(const AttrId& rhs) const;
    bool operator<(const AttrId& rhs) const;
    bool operator<=(const AttrId& rhs) const;
    const Util::String& GetName() const;
    const Util::String& GetTypeName() const;
    const Util::FourCC& GetFourCC() const;
    AccessMode GetAccessMode() const;
    ValueType GetValueType() const;
    bool IsDynamic() const;
    bool IsValid() const;
    uint GetSizeOfType() const;
    bool GetBoolDefValue() const;
    int GetIntDefValue() const;
    uint GetUIntDefValue() const;
    float GetFloatDefValue() const;
    const Util::String& GetStringDefValue() const;
    Math::vec4 GetVec4DefValue() const;
    const Math::mat4& GetMat4DefValue() const;
    const Util::Blob& GetBlobDefValue() const;
    const Util::Guid& GetGuidDefValue() const;
    const Util::Variant& GetDefaultValue() const;
    static Util::FixedArray<AttrId> GetAllAttrIds();
protected:
    const AttributeDefinitionBase* defPtr;
};
inline
AttrId::AttrId() :
    defPtr(0)
{
}
inline
AttrId::AttrId(const AttrId& rhs) :
    defPtr(rhs.defPtr)
{
}
inline
AttrId::AttrId(const AttributeDefinitionBase& def) :
    defPtr(&def)
{
}
inline
AttrId::AttrId(const AttributeDefinitionBase* ptr) :
    defPtr(ptr)
{
}
inline
AttrId::AttrId(const Util::String& name)
{
    this->defPtr = AttributeDefinitionBase::FindByName(name);
    if (0 == this->defPtr)
    {
        n_error("AttrId::AttrId(): неверное имя атрибута '%s'\n", name.AsCharPtr());
    }
}
inline
AttrId::AttrId(const Util::FourCC& fcc)
{
    this->defPtr = AttributeDefinitionBase::FindByFourCC(fcc);
    n_assert(0 != this->defPtr);
}
inline bool
AttrId::IsValidName(const Util::String& n)
{
    return 0 != AttributeDefinitionBase::FindByName(n);
}
inline bool
AttrId::IsValidFourCC(const Util::FourCC& fcc)
{
    return 0 != AttributeDefinitionBase::FindByFourCC(fcc);
}
inline
bool
AttrId::IsValid() const
{
    return (0 != this->defPtr);
}
inline
uint
AttrId::GetSizeOfType() const
{
    n_assert(this->IsValid());
    return this->defPtr->GetSizeOfType();
}
inline
bool
AttrId::operator==(const AttrId& rhs) const
{
    n_assert(this->IsValid() && rhs.IsValid());
    return (this->defPtr == rhs.defPtr);
}
inline
bool
AttrId::operator!=(const AttrId& rhs) const
{
    n_assert(this->IsValid() && rhs.IsValid());
    return (this->defPtr != rhs.defPtr);
}
inline
bool
AttrId::operator>(const AttrId& rhs) const
{
    n_assert(this->IsValid() && rhs.IsValid());
    return (this->defPtr > rhs.defPtr);
}
inline
bool
AttrId::operator>=(const AttrId& rhs) const
{
    n_assert(this->IsValid() && rhs.IsValid());
    return (this->defPtr >= rhs.defPtr);
}
inline
bool
AttrId::operator<(const AttrId& rhs) const
{
    n_assert(this->IsValid() && rhs.IsValid());
    return (this->defPtr < rhs.defPtr);
}
inline
bool
AttrId::operator<=(const AttrId& rhs) const
{
    n_assert(this->IsValid() && rhs.IsValid());
    return (this->defPtr <= rhs.defPtr);
}
inline
const Util::String&
AttrId::GetName() const
{
    n_assert(this->IsValid());
    return this->defPtr->GetName();
}
inline
const Util::String&
AttrId::GetTypeName() const
{
    n_assert(this->IsValid());
    return this->defPtr->GetTypeName();
}
inline
const Util::FourCC&
AttrId::GetFourCC() const
{
    n_assert(this->IsValid());
    return this->defPtr->GetFourCC();
}
inline
AccessMode
AttrId::GetAccessMode() const
{
    n_assert(this->IsValid());
    return this->defPtr->GetAccessMode();
}
inline
ValueType
AttrId::GetValueType() const
{
    n_assert(this->IsValid());
    return this->defPtr->GetValueType();
}
inline
bool
AttrId::IsDynamic() const
{
    n_assert(0 != this->defPtr);
    return this->defPtr->IsDynamic();
}
inline
bool
AttrId::GetBoolDefValue() const
{
    n_assert(0 != this->defPtr);
    return this->defPtr->GetDefaultValue().GetBool();
}
inline
int
AttrId::GetIntDefValue() const
{
    n_assert(0 != this->defPtr);
    return this->defPtr->GetDefaultValue().GetInt();
}
inline
uint
AttrId::GetUIntDefValue() const
{
    n_assert(0 != this->defPtr);
    return this->defPtr->GetDefaultValue().GetUInt();
}
inline
float
AttrId::GetFloatDefValue() const
{
    n_assert(0 != this->defPtr);
    return this->defPtr->GetDefaultValue().GetFloat();
}
inline
const Util::String&
AttrId::GetStringDefValue() const
{
    n_assert(0 != this->defPtr);
    return this->defPtr->GetDefaultValue().GetString();
}
inline
Math::vec4
AttrId::GetVec4DefValue() const
{
    n_assert(0 != this->defPtr);
    return this->defPtr->GetDefaultValue().GetVec4();
}
inline
const Math::mat4&
AttrId::GetMat4DefValue() const
{
    n_assert(0 != this->defPtr);
    return this->defPtr->GetDefaultValue().GetMat4();
}
inline
const Util::Blob&
AttrId::GetBlobDefValue() const
{
    n_assert(0 != this->defPtr);
    return this->defPtr->GetDefaultValue().GetBlob();
}
inline
const Util::Guid&
AttrId::GetGuidDefValue() const
{
    n_assert(0 != this->defPtr);
    return this->defPtr->GetDefaultValue().GetGuid();
}
inline
const Util::Variant&
AttrId::GetDefaultValue() const
{
    n_assert(0 != this->defPtr);
    return this->defPtr->GetDefaultValue();
}
}