#pragma once
#ifndef DB_RELATION_H
#define DB_RELATION_H
#include "core/refcounted.h"
#include "db/table.h"
namespace Db
{
class Relation : public Core::RefCounted
{
    __DeclareClass(Relation);
public:
    Relation();
    virtual ~Relation();
    void SetParent(const Ptr<Table>& table, const Attr::AttrId& columnId);
    const Ptr<Table>& GetParentTable() const;
    const Attr::AttrId& GetParentColumnId() const;
    void SetChild(const Ptr<Table>& table, const Attr::AttrId& columnId);
    const Ptr<Table>& GetChildTable() const;
    const Attr::AttrId& GetChildColumnId() const;
private:
    Ptr<Table> parentTable;
    Attr::AttrId parentColumnId;
    Ptr<Table> childTable;
    Attr::AttrId childColumnId;
};
inline void
Relation::SetParent(const Ptr<Table>& table, const Attr::AttrId& columnId)
{
    n_assert(0 != table);
    this->parentTable = table;
    this->parentColumnId = columnId;
}
inline const Ptr<Table>&
Relation::GetParentTable() const
{
    return this->parentTable;
}
inline const Attr::AttrId&
Relation::GetParentColumnId() const
{
    return this->parentColumnId;
}
inline void
Relation::SetChild(const Ptr<Table>& table, const Attr::AttrId& columnId)
{
    n_assert(0 != table);
    this->childTable = table;
    this->childColumnId = columnId;
}
inline const Ptr<Table>&
Relation::GetChildTable() const
{
    return this->childTable;
}
inline const Attr::AttrId&
Relation::GetChildColumnId() const
{
    return this->childColumnId;
}
}
#endif