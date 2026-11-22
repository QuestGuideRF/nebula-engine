#pragma once
#ifndef DB_SQLITE3DATABASE_H
#define DB_SQLITE3DATABASE_H
#include "core/config.h"
#include "attr/attribute.h"
#include "db/database.h"
#include "sqlite3.h"
namespace Attr
{
    DeclareString(name, 'name', ReadOnly);
    DeclareString(type, 'type', ReadOnly);
    DeclareInt(cid, 'cid_', ReadOnly);
    DeclareInt(notnull, 'ntnl', ReadOnly);
    DeclareBlob(dflt_value, 'dflv', ReadOnly);
    DeclareBool(pk, 'pk__', ReadOnly);
    DeclareInt(seq, 'seq_', ReadOnly);
    DeclareBool(unique, 'uniq', ReadOnly);
    DeclareInt(seqno, 'sqno', ReadOnly);
}
namespace Db
{
class Sqlite3Database : public Database
{
    __DeclareClass(Sqlite3Database);
public:
    enum TempStore
    {
        File,
        Memory,
    };
    Sqlite3Database();
    virtual ~Sqlite3Database();
    void SetCacheNumPages(SizeT numCachePages);
    SizeT GetCacheNumPages() const;
    void SetTempStore(TempStore s);
    TempStore GetTempStore() const;
    void SetSynchronousMode(bool b);
    bool GetSynchronousMode() const;
    void SetBusyTimeout(int ms);
    int GetBusyTimeout() const;
    virtual bool Open();
    virtual void Close();
    virtual bool AttachDatabase(const IO::URI& uri, const Util::String& dbName);
    virtual void DetachDatabase(const Util::String& dbName);
    virtual void BeginTransaction();
    virtual void EndTransaction();
    sqlite3* GetSqliteHandle() const;
    virtual void CopyInMemoryDatabaseToFile(const IO::URI& fileUri);
private:
    void ReadTableLayouts();
    void RegisterAttributes(Ptr<Table>& attrTable);
    SizeT cacheNumPages;
    TempStore tempStore;
    bool syncMode;
    int busyTimeout;
    sqlite3* sqliteHandle;
    Ptr<Command> beginTransactionCmd;
    Ptr<Command> endTransactionCmd;
};
inline sqlite3*
Sqlite3Database::GetSqliteHandle() const
{
    n_assert(0 != this->sqliteHandle);
    return this->sqliteHandle;
}
inline void
Sqlite3Database::SetCacheNumPages(SizeT p)
{
    n_assert(!this->IsOpen());
    n_assert(p > 0);
    this->cacheNumPages = p;
}
inline SizeT
Sqlite3Database::GetCacheNumPages() const
{
    return this->cacheNumPages;
}
inline void
Sqlite3Database::SetTempStore(TempStore s)
{
    this->tempStore = s;
}
inline Sqlite3Database::TempStore
Sqlite3Database::GetTempStore() const
{
    return this->tempStore;
}
inline void
Sqlite3Database::SetSynchronousMode(bool b)
{
    this->syncMode = b;
}
inline bool
Sqlite3Database::GetSynchronousMode() const
{
    return this->syncMode;
}
inline void
Sqlite3Database::SetBusyTimeout(int ms)
{
    this->busyTimeout = ms;
}
inline int
Sqlite3Database::GetBusyTimeout() const
{
    return this->busyTimeout;
}
}
#endif