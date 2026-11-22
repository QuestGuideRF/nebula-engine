#pragma once
#ifndef DB_DbFactory_H
#define DB_DbFactory_H
#include "core/refcounted.h"
#include "core/singleton.h"
namespace Db
{
class Database;
class Command;
class Table;
class Dataset;
class FilterSet;
class Relation;
class ValueTable;
class Reader;
class Writer;
class DbFactory : public Core::RefCounted
{
    __DeclareClass(DbFactory);
    __DeclareSingleton(DbFactory);
public:
    DbFactory();
    virtual ~DbFactory();
    virtual Ptr<Table> CreateTable() const;
    virtual Ptr<Database> CreateDatabase() const;
    virtual Ptr<Command> CreateCommand() const;
    virtual Ptr<Relation> CreateRelation() const;
    virtual Ptr<Reader> CreateReader() const;
    virtual Ptr<Writer> CreateWriter() const;
protected:
    friend class Database;
    friend class Table;
    friend class Dataset;
    friend class Writer;
    friend class Reader;
    virtual Ptr<Dataset> CreateDataset() const;
    virtual Ptr<ValueTable> CreateValueTable() const;
    virtual Ptr<FilterSet> CreateFilterSet() const;
};
}
#endif