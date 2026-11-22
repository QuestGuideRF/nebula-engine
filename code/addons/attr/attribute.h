#pragma once
#include "boolattrid.h"
#include "blobattrid.h"
#include "guidattrid.h"
#include "floatattrid.h"
#include "intattrid.h"
#include "uintattrid.h"
#include "mat4attrid.h"
#include "vec4attrid.h"
#include "stringattrid.h"
#include "util/variant.h"
#include "math/vec4.h"
#include "math/mat4.h"
#include "util/keyvaluepair.h"
#include "util/guid.h"
#include "util/blob.h"
namespace Attr
{
class Attribute : public Util::KeyValuePair<AttrId,Util::Variant>
{
public:
    Attribute();
    Attribute(const Attribute& rhs);
    Attribute(const AttrId& id);
    Attribute(Util::KeyValuePair<AttrId, Util::Variant> const& kvp);
    Attribute(const BoolAttrId& id, bool val);
    Attribute(const FloatAttrId& id, float val);
    Attribute(const IntAttrId& id, int val);
    Attribute(const UIntAttrId& id, uint val);
    Attribute(const Mat4AttrId& id, const Math::mat4& val);
    Attribute(const StringAttrId& id, const Util::String& val);
    Attribute(const Vec4AttrId& id, const Math::vec4& val);
    Attribute(const GuidAttrId& id, const Util::Guid& val);
    Attribute(const BlobAttrId& id, const Util::Blob& val);
    void SetAttrId(const AttrId& id);
    const AttrId& GetAttrId() const;
    const Util::String& GetName() const;
    const Util::FourCC& GetFourCC() const;
    ValueType GetValueType() const;
    AccessMode GetAccessMode() const;
    void Clear();
    void operator=(const Attribute& rhs);
    void operator=(bool rhs);
    void operator=(float rhs);
    void operator=(int rhs);
    void operator=(const Math::mat4& rhs);
    void operator=(const Util::String& rhs);
    void operator=(const Math::vec4& rhs);
    void operator=(const Util::Guid& rhs);
    void operator=(const Util::Blob& rhs);
    bool operator==(const Attribute& rhs) const;
    bool operator==(const Util::String& rhs) const;
    bool operator==(int rhs) const;
    bool operator==(float rhs) const;
    bool operator==(bool rhs) const;
    bool operator==(const Math::vec4& rhs) const;
    bool operator==(const Util::Guid& rhs) const;
    bool operator!=(const Attribute& rhs) const;
    bool operator!=(const Util::String& rhs) const;
    bool operator!=(int rhs) const;
    bool operator!=(float rhs) const;
    bool operator!=(bool rhs) const;
    bool operator!=(const Math::vec4& rhs) const;
    bool operator!=(const Util::Guid& rhs) const;
    void SetBool(bool val);
    bool GetBool() const;
    void SetInt(int val);
    int GetInt() const;
    void SetUInt(uint val);
    uint GetUInt() const;
    void SetFloat(float val);
    float GetFloat() const;
    void SetString(const Util::String& val);
    const Util::String& GetString() const;
    void SetMat4(const Math::mat4& val);
    const Math::mat4& GetMat4() const;
    void SetVec4(const Math::vec4& val);
    Math::vec4 GetVec4() const;
    void SetGuid(const Util::Guid& val);
    const Util::Guid& GetGuid() const;
    void SetBlob(const Util::Blob& val);
    const Util::Blob& GetBlob() const;
    void SetValue(const Util::Variant& val);
    const Util::Variant& GetValue() const;
    void SetValueFromString(const Util::String& str);
    Util::String ValueAsString() const;
    static ValueType StringToValueType(const Util::String& s);
    static Util::String ValueTypeToString(ValueType t);
};
inline
Attribute::Attribute()
{
}
inline
Attribute::Attribute(const Attribute& rhs) :
    Util::KeyValuePair<AttrId,Util::Variant>(rhs.keyData, rhs.valueData)
{
}
inline
Attribute::Attribute(const AttrId& id) :
    Util::KeyValuePair<AttrId,Util::Variant>(id)
{
}
inline
Attribute::Attribute(Util::KeyValuePair<AttrId, Util::Variant> const& kvp) :
    Util::KeyValuePair<AttrId, Util::Variant>(kvp)
{
}
inline
Attribute::Attribute(const BoolAttrId& id, bool val) :
    Util::KeyValuePair<AttrId,Util::Variant>(id, Util::Variant(val))
{
}
inline
Attribute::Attribute(const FloatAttrId& id, float val) :
    Util::KeyValuePair<AttrId,Util::Variant>(id, Util::Variant(val))
{
}
inline
Attribute::Attribute(const IntAttrId& id, int val) :
    Util::KeyValuePair<AttrId,Util::Variant>(id, Util::Variant(val))
{
}
inline
Attribute::Attribute(const Mat4AttrId& id, const Math::mat4& val) :
    Util::KeyValuePair<AttrId,Util::Variant>(id, Util::Variant(val))
{
}
inline
Attribute::Attribute(const StringAttrId& id, const Util::String& val) :
    Util::KeyValuePair<AttrId,Util::Variant>(id, Util::Variant(val))
{
}
inline
Attribute::Attribute(const Vec4AttrId& id, const Math::vec4& val) :
    Util::KeyValuePair<AttrId,Util::Variant>(id, Util::Variant(val))
{
}
inline
Attribute::Attribute(const GuidAttrId& id, const Util::Guid& val) :
    Util::KeyValuePair<AttrId,Util::Variant>(id, Util::Variant(val))
{
}
inline
Attribute::Attribute(const BlobAttrId& id, const Util::Blob& val) :
    Util::KeyValuePair<AttrId,Util::Variant>(id, Util::Variant(val))
{
}
inline
Attribute::Attribute(const UIntAttrId& id, uint val) :
    Util::KeyValuePair<AttrId, Util::Variant>(id, Util::Variant(val))
{
}
inline void
Attribute::SetAttrId(const AttrId& id)
{
    this->keyData = id;
}
inline const AttrId&
Attribute::GetAttrId() const
{
    return this->keyData;
}
inline const Util::String&
Attribute::GetName() const
{
    return this->keyData.GetName();
}
inline const Util::FourCC&
Attribute::GetFourCC() const
{
    return this->keyData.GetFourCC();
}
inline ValueType
Attribute::GetValueType() const
{
    return this->keyData.GetValueType();
}
inline AccessMode
Attribute::GetAccessMode() const
{
    return this->keyData.GetAccessMode();
}
inline void
Attribute::Clear()
{
    this->valueData.Clear();
}
inline void
Attribute::operator=(const Attribute& rhs)
{
    this->keyData = rhs.keyData;
    this->valueData = rhs.valueData;
}
inline void
Attribute::operator=(bool rhs)
{
    n_assert(this->GetValueType() == BoolType);
    n_assert(this->GetAccessMode() == ReadWrite);
    this->valueData = rhs;
}
inline void
Attribute::operator=(float rhs)
{
    n_assert(this->GetValueType() == FloatType);
    n_assert(this->GetAccessMode() == ReadWrite);
    this->valueData = rhs;
}
inline void
Attribute::operator=(int rhs)
{
    n_assert(this->GetValueType() == IntType);
    n_assert(this->GetAccessMode() == ReadWrite);
    this->valueData = rhs;
}
inline void
Attribute::operator=(const Math::mat4& rhs)
{
    n_assert(this->GetValueType() == Mat4Type);
    n_assert(this->GetAccessMode() == ReadWrite);
    this->valueData = rhs;
}
inline void
Attribute::operator=(const Util::String& rhs)
{
    n_assert(this->GetValueType() == StringType);
    n_assert(this->GetAccessMode() == ReadWrite);
    this->valueData = rhs;
}
inline void
Attribute::operator=(const Math::vec4& rhs)
{
    n_assert(this->GetValueType() == Vec4Type);
    n_assert(this->GetAccessMode() == ReadWrite);
    this->valueData = rhs;
}
inline void
Attribute::operator=(const Util::Guid& rhs)
{
    n_assert(this->GetValueType() == GuidType);
    n_assert(this->GetAccessMode() == ReadWrite);
    this->valueData = rhs;
}
inline void
Attribute::operator=(const Util::Blob& rhs)
{
    n_assert(this->GetValueType() == BlobType);
    n_assert(this->GetAccessMode() == ReadWrite);
    this->valueData = rhs;
}
inline bool
Attribute::operator==(const Attribute& rhs) const
{
    return ((this->keyData == rhs.keyData) && (this->valueData == rhs.valueData));
}
inline bool
Attribute::operator==(const Util::String& rhs) const
{
    n_assert(this->GetValueType() == StringType);
    return this->valueData == rhs;
}
inline bool
Attribute::operator==(int rhs) const
{
    n_assert(this->GetValueType() == IntType);
    return this->valueData == rhs;
}
inline bool
Attribute::operator==(float rhs) const
{
    n_assert(this->GetValueType() == FloatType);
    return this->valueData == rhs;
}
inline bool
Attribute::operator==(bool rhs) const
{
    n_assert(this->GetValueType() == BoolType);
    return this->valueData == rhs;
}
inline bool
Attribute::operator==(const Math::vec4& rhs) const
{
    n_assert(this->GetValueType() == Vec4Type);
    return this->valueData == rhs;
}
inline bool
Attribute::operator==(const Util::Guid& rhs) const
{
    n_assert(this->GetValueType() == GuidType);
    return this->valueData == rhs;
}
inline bool
Attribute::operator!=(const Attribute& rhs) const
{
    return (this->keyData != rhs.keyData) || (this->valueData != rhs.valueData);
}
inline bool
Attribute::operator!=(const Util::String& rhs) const
{
    n_assert(this->GetValueType() == StringType);
    return this->valueData != rhs;
}
inline bool
Attribute::operator!=(int rhs) const
{
    n_assert(this->GetValueType() == IntType);
    return this->valueData != rhs;
}
inline bool
Attribute::operator!=(float rhs) const
{
    n_assert(this->GetValueType() == FloatType);
    return this->valueData != rhs;
}
inline bool
Attribute::operator!=(const Math::vec4& rhs) const
{
    n_assert(this->GetValueType() == Vec4Type);
    return this->valueData != rhs;
}
inline bool
Attribute::operator!=(const Util::Guid& rhs) const
{
    n_assert(this->GetValueType() == GuidType);
    return this->valueData != rhs;
}
inline void
Attribute::SetBool(bool val)
{
    n_assert(this->GetValueType() == BoolType);
    n_assert(this->GetAccessMode() == ReadWrite);
    this->valueData = val;
}
inline bool
Attribute::GetBool() const
{
    n_assert(this->GetValueType() == BoolType);
    return this->valueData.GetBool();
}
inline void
Attribute::SetInt(int val)
{
    n_assert(this->GetValueType() == IntType);
    n_assert(this->GetAccessMode() == ReadWrite);
    this->valueData = val;
}
inline int
Attribute::GetInt() const
{
    n_assert(this->GetValueType() == IntType);
    return this->valueData.GetInt();
}
inline void
Attribute::SetUInt(uint val)
{
    n_assert(this->GetValueType() == UIntType);
    n_assert(this->GetAccessMode() == ReadWrite);
    this->valueData = val;
}
inline uint
Attribute::GetUInt() const
{
    n_assert(this->GetValueType() == UIntType);
    return this->valueData.GetUInt();
}
inline void
Attribute::SetFloat(float val)
{
    n_assert(this->GetValueType() == FloatType);
    n_assert(this->GetAccessMode() == ReadWrite);
    this->valueData = val;
}
inline float
Attribute::GetFloat() const
{
    n_assert(this->GetValueType() == FloatType);
    return this->valueData.GetFloat();
}
inline void
Attribute::SetString(const Util::String& val)
{
    n_assert(this->GetValueType() == StringType);
    n_assert(this->GetAccessMode() == ReadWrite);
    this->valueData = val;
}
inline const Util::String&
Attribute::GetString() const
{
    n_assert(this->GetValueType() == StringType);
    return this->valueData.GetString();
}
inline void
Attribute::SetMat4(const Math::mat4& val)
{
    n_assert(this->GetValueType() == Mat4Type);
    n_assert(this->GetAccessMode() == ReadWrite);
    this->valueData = val;
}
inline const Math::mat4&
Attribute::GetMat4() const
{
    n_assert(this->GetValueType() == Mat4Type);
    return this->valueData.GetMat4();
}
inline void
Attribute::SetVec4(const Math::vec4& val)
{
    n_assert(this->GetValueType() == Vec4Type);
    n_assert(this->GetAccessMode() == ReadWrite);
    this->valueData = val;
}
inline Math::vec4
Attribute::GetVec4() const
{
    n_assert(this->GetValueType() == Vec4Type);
    return this->valueData.GetVec4();
}
inline void
Attribute::SetGuid(const Util::Guid& val)
{
    n_assert(this->GetValueType() == GuidType);
    n_assert(this->GetAccessMode() == ReadWrite);
    this->valueData = val;
}
inline const Util::Guid&
Attribute::GetGuid() const
{
    n_assert(this->GetValueType() == GuidType);
    return this->valueData.GetGuid();
}
inline void
Attribute::SetBlob(const Util::Blob& val)
{
    n_assert(this->GetValueType() == BlobType);
    n_assert(this->GetAccessMode() == ReadWrite);
    this->valueData = val;
}
inline const Util::Blob&
Attribute::GetBlob() const
{
    n_assert(this->GetValueType() == BlobType);
    return this->valueData.GetBlob();
}
inline void
Attribute::SetValueFromString(const Util::String& str)
{
    n_assert(this->GetValueType() != VoidType);
    switch (this->GetValueType())
    {
        case IntType:
            this->valueData = str.AsInt();
            break;
        case FloatType:
            this->valueData = str.AsFloat();
            break;
        case BoolType:
            this->valueData = str.AsBool();
            break;
        case Vec4Type:
            this->valueData = str.AsVec4();
            break;
        case StringType:
            this->valueData = str;
            break;
        case Mat4Type:
            this->valueData = str.AsMat4();
            break;
        case GuidType:
            this->valueData = Util::Guid::FromString(str);
            break;
        default:
            n_error("Attribute::SetValueFromString(): невозможно преобразовать строку в тип атрибута!");
            break;
    }
}
inline Util::String
Attribute::ValueAsString() const
{
    n_assert(this->GetValueType() != VoidType);
    Util::String str;
    switch (this->GetValueType())
    {
        case IntType:
            str.SetInt(this->GetInt());
            break;
        case FloatType:
            str.SetFloat(this->GetFloat());
            break;
        case BoolType:
            str.SetBool(this->GetBool());
            break;
        case Vec4Type:
            str.SetVec4(this->GetVec4());
            break;
        case StringType:
            str = this->GetString();
            break;
        case Mat4Type:
            str.SetMat4(this->GetMat4());
            break;
        case GuidType:
            str = this->GetGuid().AsString();
            break;
        case BlobType:
            n_error("Атрибут: невозможно преобразовать большой двоичный объект в строку (attrid='%s')!", this->GetName().AsCharPtr());
            break;
        case VoidType:
            n_error("Атрибут: невозможно преобразовать атрибут void в строку (attrid='%s')!", this->GetName().AsCharPtr());
            break;
        default: n_error("Атрибут: необработанное перечисление"); break;
    }
    return str;
}
inline ValueType
Attribute::StringToValueType(const Util::String& s)
{
    return (ValueType) Util::Variant::StringToType(s);
}
inline Util::String
Attribute::ValueTypeToString(ValueType t)
{
    return Util::Variant::TypeToString((Util::Variant::Type)t);
}
inline void
Attribute::SetValue(const Util::Variant& val)
{
    this->valueData = val;
}
inline const Util::Variant&
Attribute::GetValue() const
{
    return this->valueData;
}
}