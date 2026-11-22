#pragma once
#ifndef DB_COLUMN_H
#define DB_COLUMN_H
#include "core/types.h"
#include "attr/attrid.h"
namespace Db
{
class Column
{
public:
    enum Type
    {
        Default,
        Primary,
        Indexed,
    };
    Column();
    Column(const Attr::AttrId& attrId, Type=Default);
    void SetAttrId(const Attr::AttrId& attrId);
    const Attr::AttrId& GetAttrId() const;
    const Util::String& GetName() const;
    const Util::FourCC& GetFourCC() const;
    Attr::ValueType GetValueType() const;
    Attr::AccessMode GetAccessMode() const;
    void SetType(Type t);
    Type GetType() const;
    void SetCommitted(bool b);
    bool IsCommitted() const;
private:
    Attr::AttrId attrId;
    Type type;
    bool committed;
};
inline void
Column::SetAttrId(const Attr::AttrId& id)
{
    this->attrId = id;
}
inline const Attr::AttrId&
Column::GetAttrId() const
{
    return this->attrId;
}
inline const Util::String&
Column::GetName() const
{
    return this->attrId.GetName();
}
inline const Util::FourCC&
Column::GetFourCC() const
{
    return this->attrId.GetFourCC();
}
inline Attr::ValueType
Column::GetValueType() const
{
    return this->attrId.GetValueType();
}
inline Attr::AccessMode
Column::GetAccessMode() const
{
    return this->attrId.GetAccessMode();
}
inline void
Column::SetType(Type t)
{
    this->type = t;
}
inline Column::Type
Column::GetType() const
{
    return this->type;
}
inline void
Column::SetCommitted(bool b)
{
    this->committed = b;
}
inline bool
Column::IsCommitted() const
{
    return this->committed;
}
}
#endif