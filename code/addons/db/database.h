#pragma once
#include "core/refcounted.h"
#include "io/uri.h"
#include "util/dictionary.h"
#include "db/table.h"
#include "db/command.h"
namespace Attr
{
    DeclareString(AttrName, 'ATNA', Attr::ReadWrite);
    DeclareString(AttrType, 'ATTY', Attr::ReadWrite);
    DeclareBool(AttrReadWrite, 'ATRW', Attr::ReadWrite);
    DeclareBool(AttrDynamic, 'ATDY', Attr::ReadWrite);
}
namespace Db
{
class Database : public Core::RefCounted
{
    __DeclareClass(Database);
public:
    enum AccessMode
    {
        ReadOnly,
        ReadWriteExisting,
        ReadWriteCreate,
    };
    Database();
    virtual ~Database();
    void SetURI(const IO::URI& uri);
    const IO::URI& GetURI() const;
    void SetAccessMode(AccessMode m);
    AccessMode GetAccessMode() const;
    void SetExclusiveMode(bool b);
    bool GetExclusiveMode() const;
    void SetIgnoreUnknownColumns(bool b);
    bool GetIgnoreUnknownColumns() const;
    virtual bool Open();
    virtual void Close();
    bool IsOpen() const;
    const Util::String& GetError() const;
    virtual bool AttachDatabase(const IO::URI& uri, const Util::String& dbName);
    virtual void DetachDatabase(const Util::String& dbName);
    virtual void BeginTransaction();
    virtual void EndTransaction();
    virtual void AddTable(const Ptr<Table>& table);
    virtual void DeleteTable(const Util::String& tableName);
    bool HasTable(const Util::String& tableName) const;
    SizeT GetNumTables() const;
    const Ptr<Table>& GetTableByIndex(IndexT i) const;
    const Ptr<Table>& GetTableByName(const Util::String& tableName) const;
    void SetInMemoryDatabase(bool b);
    virtual void CopyInMemoryDatabaseToFile(const IO::URI& fileUri);
protected:
    void SetError(const Util::String& error);
    IndexT FindTableIndex(const Util::String& tableName) const;
    bool isOpen;
    bool ignoreUnknownColumns;
    IO::URI uri;
    Util::String error;
    AccessMode accessMode;
    bool memoryDatabase;
    bool exclusiveMode;
    Util::Array<Ptr<Table> > tables;
};
inline void
Database::SetIgnoreUnknownColumns(bool b)
{
    this->ignoreUnknownColumns = b;
}
inline bool
Database::GetIgnoreUnknownColumns() const
{
    return this->ignoreUnknownColumns;
}
inline void
Database::SetError(const Util::String& err)
{
    this->error = err;
}
inline const Util::String&
Database::GetError() const
{
    return this->error;
}
inline bool
Database::IsOpen() const
{
    return this->isOpen;
}
inline void
Database::SetURI(const IO::URI& u)
{
    this->uri = u;
}
inline const IO::URI&
Database::GetURI() const
{
    return this->uri;
}
inline void
Database::SetAccessMode(AccessMode m)
{
    this->accessMode = m;
}
inline Database::AccessMode
Database::GetAccessMode() const
{
    return this->accessMode;
}
inline void
Database::SetInMemoryDatabase(bool b)
{
    this->memoryDatabase = b;
}
inline void
Database::SetExclusiveMode(bool b)
{
    this->exclusiveMode = b;
}
inline bool
Database::GetExclusiveMode() const
{
    return this->exclusiveMode;
}
}