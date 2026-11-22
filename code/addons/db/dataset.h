#pragma once
#ifndef DB_DATASET_H
#define DB_DATASET_H
#include "core/refcounted.h"
#include "db/filterset.h"
#include "db/valuetable.h"
#include "db/relation.h"
namespace Db
{
class Database;
class Table;
class Dataset : public Core::RefCounted
{
    __DeclareClass(Dataset);
public:
    Dataset();
    virtual ~Dataset();
    void AddColumn(const Attr::AttrId& attrId);
    void AddAllTableColumns();
    const Ptr<Table>& GetTable() const;
    const Ptr<FilterSet>& Filter();
    const Ptr<ValueTable>& Values();
    virtual void PerformQuery(bool appendResult=false);
    virtual void CommitChanges(bool newRowsAsUpdate=false);
    virtual void CommitDeletedRows();
protected:
    friend class Table;
    void SetTable(const Ptr<Table>& t);
    virtual bool Connect();
    virtual void Disconnect();
    bool IsConnected() const;
    bool isConnected;
    Ptr<Table> table;
    Ptr<FilterSet> filter;
    Ptr<ValueTable> values;
};
inline void
Dataset::SetTable(const Ptr<Table>& t)
{
    n_assert(0 != t);
    this->table = t;
}
inline const Ptr<Table>&
Dataset::GetTable() const
{
    return this->table;
}
inline bool
Dataset::IsConnected() const
{
    return this->isConnected;
}
inline const Ptr<FilterSet>&
Dataset::Filter()
{
    return this->filter;
}
inline const Ptr<ValueTable>&
Dataset::Values()
{
    return this->values;
}
}
#endif