#pragma once
#include "core/types.h"
#include "util/fourcc.h"
#include "util/string.h"
#include "util/hashtable.h"
#include "util/dictionary.h"
#include "valuetype.h"
#include "accessmode.h"
#include "attrexithandler.h"
#include "util/string.h"
#include "util/blob.h"
#include "math/quat.h"
#include "math/transform44.h"
namespace Attr
{
class AttributeDefinitionBase
{
public:
    explicit AttributeDefinitionBase(const Util::String& name, const Util::String& typeName, const Util::FourCC& fourCC, AccessMode accessMode, bool isDynamic);
    explicit AttributeDefinitionBase(const Util::String& name, const Util::String& typeName, const Util::FourCC& fourCC, AccessMode accessMode, const byte& defVal, bool isDynamic);
    explicit AttributeDefinitionBase(const Util::String& name, const Util::String& typeName, const Util::FourCC& fourCC, AccessMode accessMode, const short& defVal, bool isDynamic);
    explicit AttributeDefinitionBase(const Util::String& name, const Util::String& typeName, const Util::FourCC& fourCC, AccessMode accessMode, const ushort& defVal, bool isDynamic);
    explicit AttributeDefinitionBase(const Util::String& name, const Util::String& typeName, const Util::FourCC& fourCC, AccessMode accessMode, const int& defVal, bool isDynamic);
    explicit AttributeDefinitionBase(const Util::String& name, const Util::String& typeName, const Util::FourCC& fourCC, AccessMode accessMode, const uint& defVal, bool isDynamic);
    explicit AttributeDefinitionBase(const Util::String& name, const Util::String& typeName, const Util::FourCC& fourCC, AccessMode accessMode, const int64_t& defVal, bool isDynamic);
    explicit AttributeDefinitionBase(const Util::String& name, const Util::String& typeName, const Util::FourCC& fourCC, AccessMode accessMode, const uint64_t& defVal, bool isDynamic);
    explicit AttributeDefinitionBase(const Util::String& name, const Util::String& typeName, const Util::FourCC& fourCC, AccessMode accessMode, const float& defVal, bool isDynamic);
    explicit AttributeDefinitionBase(const Util::String& name, const Util::String& typeName, const Util::FourCC& fourCC, AccessMode accessMode, const double& defVal, bool isDynamic);
    explicit AttributeDefinitionBase(const Util::String& name, const Util::String& typeName, const Util::FourCC& fourCC, AccessMode accessMode, const bool& defVal, bool isDynamic);
    explicit AttributeDefinitionBase(const Util::String& name, const Util::String& typeName, const Util::FourCC& fourCC, AccessMode accessMode, const Math::vec2& defVal, bool isDynamic);
    explicit AttributeDefinitionBase(const Util::String& name, const Util::String& typeName, const Util::FourCC& fourCC, AccessMode accessMode, const Math::vec4& defVal, bool isDynamic);
    explicit AttributeDefinitionBase(const Util::String& name, const Util::String& typeName, const Util::FourCC& fourCC, AccessMode accessMode, const Math::quat& defVal, bool isDynamic);
    explicit AttributeDefinitionBase(const Util::String& name, const Util::String& typeName, const Util::FourCC& fourCC, AccessMode accessMode, const Util::String& defVal, bool isDynamic);
    explicit AttributeDefinitionBase(const Util::String& name, const Util::String& typeName, const Util::FourCC& fourCC, AccessMode accessMode, const Math::mat4& defVal, bool isDynamic);
    explicit AttributeDefinitionBase(const Util::String& name, const Util::String& typeName, const Util::FourCC& fourCC, AccessMode accessMode, const Math::transform44& defVal, bool isDynamic);
    explicit AttributeDefinitionBase(const Util::String& name, const Util::String& typeName, const Util::FourCC& fourCC, AccessMode accessMode, const Util::Blob& defVal, bool isDynamic);
    explicit AttributeDefinitionBase(const Util::String& name, const Util::String& typeName, const Util::FourCC& fourCC, AccessMode accessMode, const Util::Guid& defVal, bool isDynamic);
    explicit AttributeDefinitionBase(const Util::String& name, const Util::String& typeName, const Util::FourCC& fourCC, AccessMode accessMode, void* defVal, bool isDynamic);
    explicit AttributeDefinitionBase(const Util::String& name, const Util::String& typeName, const Util::FourCC& fourCC, AccessMode accessMode, const Util::Array<int>& defVal, bool isDynamic);
    explicit AttributeDefinitionBase(const Util::String& name, const Util::String& typeName, const Util::FourCC& fourCC, AccessMode accessMode, const Util::Array<float>& defVal, bool isDynamic);
    explicit AttributeDefinitionBase(const Util::String& name, const Util::String& typeName, const Util::FourCC& fourCC, AccessMode accessMode, const Util::Array<bool>& defVal, bool isDynamic);
    explicit AttributeDefinitionBase(const Util::String& name, const Util::String& typeName, const Util::FourCC& fourCC, AccessMode accessMode, const Util::Array<Math::vec2>& defVal, bool isDynamic);
    explicit AttributeDefinitionBase(const Util::String& name, const Util::String& typeName, const Util::FourCC& fourCC, AccessMode accessMode, const Util::Array<Math::vec4>& defVal, bool isDynamic);
    explicit AttributeDefinitionBase(const Util::String& name, const Util::String& typeName, const Util::FourCC& fourCC, AccessMode accessMode, const Util::Array<Util::String>& defVal, bool isDynamic);
    explicit AttributeDefinitionBase(const Util::String& name, const Util::String& typeName, const Util::FourCC& fourCC, AccessMode accessMode, const Util::Array<Math::mat4>& defVal, bool isDynamic);
    explicit AttributeDefinitionBase(const Util::String& name, const Util::String& typeName, const Util::FourCC& fourCC, AccessMode accessMode, const Util::Array<Util::Blob>& defVal, bool isDynamic);
    explicit AttributeDefinitionBase(const Util::String& name, const Util::String& typeName, const Util::FourCC& fourCC, AccessMode accessMode, const Util::Array<Util::Guid>& defVal, bool isDynamic);
    explicit AttributeDefinitionBase(const Util::String& name, const Util::String& typeName, const Util::FourCC& fourCC, AccessMode accessMode, const Util::Variant& defVal, ValueType type, bool isDynamic);
    ~AttributeDefinitionBase();
    static void Destroy();
    bool IsDynamic() const;
    const Util::String& GetName() const;
    const Util::FourCC& GetFourCC() const;
    const Util::String& GetTypeName() const;
    const Util::Variant& GetDefaultValue() const;
    const uint GetSizeOfType() const;
    AccessMode GetAccessMode() const;
    ValueType GetValueType() const;
    static const AttributeDefinitionBase* FindByName(const Util::String& n);
    static const AttributeDefinitionBase* FindByFourCC(const Util::FourCC& fcc);
    static void RegisterDynamicAttribute(const Util::String& name, const Util::String& typeName, const Util::FourCC& fourCC, ValueType valueType, AccessMode accessMode);
    static void ClearDynamicAttributes();
    static void InitializeRegistry();
protected:
    void Register();
private:
    AttributeDefinitionBase();
    static void CheckCreateNameRegistry();
    static void CheckCreateFourCCRegistry();
    static void CheckCreateDynamicAttributesArray();
    Util::String name;
    Util::String typeName;
    Util::FourCC fourCC;
    AccessMode accessMode;
    Util::Variant defaultValue;
    const Attr::ValueType valueType;
    bool isDynamic;
    friend class AttrId;
    static AttrExitHandler attrExitHandler;
    static Util::HashTable<Util::String, const AttributeDefinitionBase*>* NameRegistry;
    static Util::Dictionary<Util::FourCC, const AttributeDefinitionBase*>* FourCCRegistry;
    static Util::Array<const AttributeDefinitionBase*>* DynamicAttributes;
};
inline
const AttributeDefinitionBase*
AttributeDefinitionBase::FindByName(const Util::String& n)
{
    n_assert(0 != NameRegistry);
    if (!NameRegistry->Contains(n))
    {
        return nullptr;
    }
    else
    {
        return (*NameRegistry)[n];
    }
}
inline
const AttributeDefinitionBase*
AttributeDefinitionBase::FindByFourCC(const Util::FourCC& fcc)
{
    n_assert(0 != FourCCRegistry);
    if (!FourCCRegistry->Contains(fcc))
    {
        return nullptr;
    }
    else
    {
        return (*FourCCRegistry)[fcc];
    }
}
inline
const Util::String&
AttributeDefinitionBase::GetName() const
{
    return this->name;
}
inline
const Util::FourCC&
AttributeDefinitionBase::GetFourCC() const
{
    return this->fourCC;
}
inline
const Util::String&
AttributeDefinitionBase::GetTypeName() const
{
    return this->typeName;
}
inline
const Util::Variant&
AttributeDefinitionBase::GetDefaultValue() const
{
    return this->defaultValue;
}
inline
const uint
AttributeDefinitionBase::GetSizeOfType() const
{
    switch (this->valueType)
    {
    case VoidType:          return 0;
    case ByteType:          return sizeof(uint8_t);
    case ShortType:         return sizeof(uint16_t);
    case UShortType:        return sizeof(uint16_t);
    case IntType:           return sizeof(uint32_t);
    case UIntType:          return sizeof(uint32_t);
    case Int64Type:         return sizeof(uint64_t);
    case UInt64Type:        return sizeof(uint64_t);
    case FloatType:         return sizeof(float);
    case DoubleType:        return sizeof(double);
    case BoolType:          return sizeof(bool);
    case Vec2Type:          return sizeof(float) * 2;
    case Vec4Type:          return sizeof(float) * 4;
    case QuaternionType:    return sizeof(float) * 4;
    case StringType:        return sizeof(Util::String);
    case Mat4Type:          return sizeof(Math::mat4);
    case Transform44Type:   return sizeof(Math::transform44);
    case BlobType:          return sizeof(Util::Blob);
    case GuidType:          return sizeof(Util::Guid);
    case VoidPtrType:       return sizeof(void*);
    case IntArrayType:      return sizeof(Util::Array<int>);
    case FloatArrayType:    return sizeof(Util::Array<float>);
    case BoolArrayType:     return sizeof(Util::Array<bool>);
    case Vec2ArrayType:     return sizeof(Util::Array<Math::vec2>);
    case Vec4ArrayType:     return sizeof(Util::Array<Math::vec4>);
    case Mat4ArrayType: return sizeof(Util::Array<Math::mat4>);
    case StringArrayType:   return sizeof(Util::Array<Util::String>);
    case GuidArrayType:     return sizeof(Util::Array<Util::Guid>);
    case BlobArrayType:     return sizeof(Util::Array<Util::Blob>);
    default:
        n_error("AttributeDefinitionBase::GetSizeOfType(): invalid type enum '%d'!", this->valueType);
        return 0;
    }
}
inline
ValueType
AttributeDefinitionBase::GetValueType() const
{
    return (ValueType)this->defaultValue.GetType();
}
inline
AccessMode
AttributeDefinitionBase::GetAccessMode() const
{
    return this->accessMode;
}
inline
bool
AttributeDefinitionBase::IsDynamic() const
{
    return this->isDynamic;
}
}