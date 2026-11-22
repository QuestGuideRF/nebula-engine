#pragma once
#ifndef DB_SQLITE3FACTORY_H
#define DB_SQLITE3FACTORY_H
#include "db/dbfactory.h"
namespace Db
{
class Sqlite3Factory : public DbFactory
{
    __DeclareClass(Sqlite3Factory);
    __DeclareSingleton(Sqlite3Factory);
public:
    Sqlite3Factory();
    virtual ~Sqlite3Factory();
    virtual Ptr<Database> CreateDatabase() const;
    virtual Ptr<Command> CreateCommand() const;
    virtual Ptr<Table> CreateTable() const;
protected:
    friend class Sqlite3Database;
    friend class Sqlite3Table;
    virtual Ptr<Dataset> CreateDataset() const;
    virtual Ptr<FilterSet> CreateFilterSet() const;
};
}
#endif