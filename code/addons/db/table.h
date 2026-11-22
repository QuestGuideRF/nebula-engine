#pragma once
#ifndef DB_TABLE_H
#define DB_TABLE_H
#include "core/refcounted.h"
#include "db/column.h"
#include "util/array.h"
#include "util/string.h"
#include "util/dictionary.h"
#include "util/hashtable.h"
#include "db/valuetable.h"
namespace Db
{
class Database;
class Dataset;
class Table : public Core::RefCounted
{
    __DeclareClass(Table);
public:
    enum ConnectMode
    {
        ForceCreate,
        AssumeExists,
        Default,
    };
    Table();
    virtual ~Table();
    virtual void SetName(const Util::String& n);
    const Util::String& GetName() const;
    bool IsConnected() const;
    const Ptr<Database>& GetDatabase() const;
    virtual void CreateMultiColumnIndex(const Util::Array<Attr::AttrId>& columnIds);
    virtual Ptr<Dataset> CreateDataset();
    virtual void CommitChanges(bool resetModifiedState=true, bool useTransaction=true);
    bool HasUncommittedColumns() const;
    virtual void CommitUncommittedColumns();
    virtual void CommitDeletedRows();
    void AddColumn(const Column& c);
    SizeT GetNumColumns() const;
    const Column& GetColumn(IndexT i) const;
    bool HasColumn(const Attr::AttrId& id) const;
    bool HasColumn(const Util::String& name) const;
    bool HasColumn(const Util::FourCC& fcc) const;
    const Column& GetColumn(const Attr::AttrId& id) const;
    const Column& GetColumn(const Util::String& name) const;
    const Column& GetColumn(const Util::FourCC& fcc) const;
    const Util::Array<Column>& GetColumns() const;
    bool HasPrimaryColumn() const;
    const Column& GetPrimaryColumn() const;
protected:
    friend class Database;
    friend class Dataset;
    friend class Writer;
    virtual void Connect(const Ptr<Database>& db, ConnectMode connectMode=Default, bool ignoreUnknownColumns=false);
    virtual void Disconnect(bool dropTable=false);
    virtual void BindValueTable(const Ptr<ValueTable>& valueTable);
    virtual void UnbindValueTable();
    bool isConnected;
    Ptr<Database> database;
    Ptr<ValueTable> valueTable;
    Util::String name;
    Util::Array<Column> columns;
    Util::HashTable<Util::String,IndexT> nameIndexMap;
    Util::Dictionary<Util::FourCC,IndexT> fourccIndexMap;
    Util::Dictionary<Attr::AttrId,IndexT> attrIdIndexMap;
    IndexT primaryColumnIndex;
};
inline
const Ptr<Database>&
Table::GetDatabase() const
{
    return this->database;
}
inline
bool
Table::HasPrimaryColumn() const
{
    return (InvalidIndex != this->primaryColumnIndex);
}
inline
const Column&
Table::GetPrimaryColumn() const
{
    n_assert(InvalidIndex != this->primaryColumnIndex);
    return this->columns[this->primaryColumnIndex];
}
inline
bool
Table::IsConnected() const
{
    return this->isConnected;
}
inline
const Util::String&
Table::GetName() const
{
    return this->name;
}
inline
SizeT
Table::GetNumColumns() const
{
    return this->columns.Size();
}
inline
const Column&
Table::GetColumn(IndexT i) const
{
    return this->columns[i];
}
inline
bool
Table::HasColumn(const Attr::AttrId& id) const
{
    return this->attrIdIndexMap.Contains(id);
}
inline
bool
Table::HasColumn(const Util::String& name) const
{
    return this->nameIndexMap.Contains(name);
}
inline
bool
Table::HasColumn(const Util::FourCC& fcc) const
{
    return this->fourccIndexMap.Contains(fcc);
}
inline
const Column&
Table::GetColumn(const Attr::AttrId& id) const
{
    n_assert(this->HasColumn(id));
    return this->columns[this->attrIdIndexMap[id]];
}
inline
const Column&
Table::GetColumn(const Util::String& name) const
{
    n_assert(this->HasColumn(name));
    return this->columns[this->nameIndexMap[name]];
}
inline
const Column&
Table::GetColumn(const Util::FourCC& fcc) const
{
    n_assert(this->HasColumn(fcc));
    return this->columns[this->fourccIndexMap[fcc]];
}
inline
const Util::Array<Column>&
Table::GetColumns() const
{
    return this->columns;
}
}
#endif