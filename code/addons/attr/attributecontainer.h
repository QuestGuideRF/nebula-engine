#pragma once
#include "util/dictionary.h"
#include "attribute.h"
namespace Attr
{
class AttributeContainer
{
public:
    AttributeContainer();
    ~AttributeContainer();
    bool HasAttr(const AttrId& attrId) const;
    void SetAttr(const Attribute& attr);
    const Attribute& GetAttr(const AttrId& attrId) const;
    const Util::Dictionary<AttrId, Attribute>& GetAttrs() const;
    void RemoveAttr(const AttrId& attrId);
    void Clear();
    void AddAttr(const Attribute& attr);
    void SetBool(const BoolAttrId& attrId, bool val);
    bool GetBool(const BoolAttrId& attrId) const;
    bool GetBool(const BoolAttrId& attrId, bool defaultValue) const;
    void SetFloat(const FloatAttrId& attrId, float val);
    float GetFloat(const FloatAttrId& attrId) const;
    float GetFloat(const FloatAttrId& attrId, float defaultValue) const;
    void SetInt(const IntAttrId& attrId, int val);
    int GetInt(const IntAttrId& attrId) const;
    int GetInt(const IntAttrId& attrId, int defaultValue) const;
    void SetString(const StringAttrId& attrId, const Util::String& val);
    const Util::String& GetString(const StringAttrId& attrId) const;
    const Util::String& GetString(const StringAttrId& attrId, const Util::String& defaultValue) const;
    void SetFloat4(const Vec4AttrId& attrId, const Math::vec4& val);
    Math::vec4 GetVec4(const Vec4AttrId& attrId) const;
    Math::vec4 GetVec4(const Vec4AttrId& attrId, const Math::vec4& defaultValue) const;
    void SetMatrix44(const Mat4AttrId& attrId, const Math::mat4& val);
    const Math::mat4& GetMat4(const Mat4AttrId& attrId) const;
    const Math::mat4& GetMat4(const Mat4AttrId& attrId, const Math::mat4& defaultValue) const;
    void SetGuid(const GuidAttrId& attrId, const Util::Guid& guid);
    const Util::Guid& GetGuid(const GuidAttrId& attrId) const;
    const Util::Guid& GetGuid(const GuidAttrId& attrId, const Util::Guid& defaultValue) const;
    void SetBlob(const BlobAttrId& attrId, const Util::Blob& blob);
    const Util::Blob& GetBlob(const BlobAttrId& attrId) const;
    const Util::Blob& GetBlob(const BlobAttrId& attrId, const Util::Blob& defaultValue) const;
private:
    int FindAttrIndex(const Attr::AttrId& attrId) const;
    Util::Dictionary<Attr::AttrId, Attribute> attrs;
};
inline void
AttributeContainer::SetBool(const BoolAttrId& attrId, bool val)
{
    this->SetAttr(Attribute(attrId, val));
}
inline bool
AttributeContainer::GetBool(const BoolAttrId& attrId) const
{
    n_assert(this->attrs.Contains(attrId));
    return this->attrs[attrId].GetBool();
}
inline bool
AttributeContainer::GetBool(const BoolAttrId& attrId, bool defaultValue) const
{
    if (this->HasAttr(attrId))
    {
        return this->GetBool(attrId);
    }
    else
    {
        return defaultValue;
    }
}
inline void
AttributeContainer::SetFloat(const FloatAttrId& attrId, float val)
{
    this->SetAttr(Attribute(attrId, val));
}
inline float
AttributeContainer::GetFloat(const FloatAttrId& attrId) const
{
    n_assert(this->attrs.Contains(attrId));
    return this->attrs[attrId].GetFloat();
}
inline float
AttributeContainer::GetFloat(const FloatAttrId& attrId, float defaultValue) const
{
    if (this->HasAttr(attrId))
    {
        return this->GetFloat(attrId);
    }
    else
    {
        return defaultValue;
    }
}
inline void
AttributeContainer::SetInt(const IntAttrId& attrId, int val)
{
    this->SetAttr(Attribute(attrId, val));
}
inline int
AttributeContainer::GetInt(const IntAttrId& attrId) const
{
    n_assert(this->attrs.Contains(attrId));
    return this->attrs[attrId].GetInt();
}
inline int
AttributeContainer::GetInt(const IntAttrId& attrId, int defaultValue) const
{
    if (this->HasAttr(attrId))
    {
        return this->GetInt(attrId);
    }
    else
    {
        return defaultValue;
    }
}
inline void
AttributeContainer::SetString(const StringAttrId& attrId, const Util::String& val)
{
    this->SetAttr(Attribute(attrId, val));
}
inline const Util::String&
AttributeContainer::GetString(const StringAttrId& attrId) const
{
    n_assert(this->attrs.Contains(attrId));
    return this->attrs[attrId].GetString();
}
inline const Util::String&
AttributeContainer::GetString(const StringAttrId& attrId, const Util::String& defaultValue) const
{
    if (this->HasAttr(attrId))
    {
        return this->GetString(attrId);
    }
    else
    {
        return defaultValue;
    }
}
inline void
AttributeContainer::SetFloat4(const Vec4AttrId& attrId, const Math::vec4& val)
{
    this->SetAttr(Attribute(attrId, val));
}
inline Math::vec4
AttributeContainer::GetVec4(const Vec4AttrId& attrId) const
{
    n_assert(this->attrs.Contains(attrId));
    return this->attrs[attrId].GetVec4();
}
inline Math::vec4
AttributeContainer::GetVec4(const Vec4AttrId& attrId, const Math::vec4& defaultValue) const
{
    if (this->HasAttr(attrId))
    {
        return this->GetVec4(attrId);
    }
    else
    {
        return defaultValue;
    }
}
inline void
AttributeContainer::SetMatrix44(const Mat4AttrId& attrId, const Math::mat4& val)
{
    this->SetAttr(Attribute(attrId, val));
}
inline const Math::mat4&
AttributeContainer::GetMat4(const Mat4AttrId& attrId) const
{
    n_assert(this->attrs.Contains(attrId));
    return this->attrs[attrId].GetMat4();
}
inline const Math::mat4&
AttributeContainer::GetMat4(const Mat4AttrId& attrId, const Math::mat4& defaultValue) const
{
    if (this->HasAttr(attrId))
    {
        return this->GetMat4(attrId);
    }
    else
    {
        return defaultValue;
    }
}
inline void
AttributeContainer::SetGuid(const GuidAttrId& attrId, const Util::Guid& val)
{
    this->SetAttr(Attribute(attrId, val));
}
inline const Util::Guid&
AttributeContainer::GetGuid(const GuidAttrId& attrId) const
{
    n_assert(this->attrs.Contains(attrId));
    return this->attrs[attrId].GetGuid();
}
inline const Util::Guid&
AttributeContainer::GetGuid(const GuidAttrId& attrId, const Util::Guid& defaultValue) const
{
    if (this->HasAttr(attrId))
    {
        return this->GetGuid(attrId);
    }
    else
    {
        return defaultValue;
    }
}
inline void
AttributeContainer::SetBlob(const BlobAttrId& attrId, const Util::Blob& val)
{
    this->SetAttr(Attribute(attrId, val));
}
inline const Util::Blob&
AttributeContainer::GetBlob(const BlobAttrId& attrId) const
{
    n_assert(this->attrs.Contains(attrId));
    return this->attrs[attrId].GetBlob();
}
inline const Util::Blob&
AttributeContainer::GetBlob(const BlobAttrId& attrId, const Util::Blob& defaultValue) const
{
    if (this->HasAttr(attrId))
    {
        return this->GetBlob(attrId);
    }
    else
    {
        return defaultValue;
    }
}
}