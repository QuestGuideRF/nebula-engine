#pragma once
#ifndef DB_SQLITE3DATASET_H
#define DB_SQLITE3DATASET_H
#include "db/dataset.h"
namespace Db
{
class Command;
class Sqlite3Dataset : public Dataset
{
    __DeclareClass(Sqlite3Dataset);
public:
    Sqlite3Dataset();
    virtual ~Sqlite3Dataset();
    virtual void PerformQuery(bool appendResult=false);
    virtual void CommitChanges(bool newRowsAsUpdate=false);
    virtual void CommitDeletedRows();
private:
    virtual bool Connect();
    virtual void Disconnect();
    Util::String GetSqlSelectColumns() const;
    Ptr<Command> queryCommand;
    static const Util::String SelectFrag;
    static const Util::String FromFrag;
    static const Util::String CommaFrag;
    static const Util::String WhereFrag;
    static const Util::String StarFrag;
    static const Util::String TickFrag;
};
}
#endif