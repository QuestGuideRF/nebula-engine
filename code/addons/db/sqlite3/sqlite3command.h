#pragma once
#ifndef DB_SQLITE3COMMAND_H
#define DB_SQLITE3COMMAND_H
#include "db/command.h"
#include "sqlite3.h"
namespace Db
{
class Sqlite3Command : public Command
{
    __DeclareClass(Sqlite3Command);
public:
    Sqlite3Command();
    virtual ~Sqlite3Command();
    virtual bool Compile(const Ptr<Database>& db, const Util::String& sqlCommand, ValueTable* resultTable=0);
    virtual bool Execute();
    virtual void Clear();
    virtual bool IsValid() const;
    virtual IndexT IndexOf(const Util::String& name) const;
    virtual IndexT IndexOf(const Attr::AttrId& attrId) const;
    virtual void BindInt(IndexT index, int val);
    virtual void BindFloat(IndexT index, float val);
    virtual void BindBool(IndexT index, bool val);
    virtual void BindVec4(IndexT index, const Math::vec4& val);
    virtual void BindString(IndexT index, const Util::String& val);
    virtual void BindMat4(IndexT index, const Math::mat4& val);
    virtual void BindBlob(IndexT index, const Util::Blob& val);
    virtual void BindGuid(IndexT index, const Util::Guid& val);
    virtual void BindInt(const Util::String& name, int val);
    virtual void BindFloat(const Util::String& name, float val);
    virtual void BindBool(const Util::String& name, bool val);
    virtual void BindVec4(const Util::String& name, const Math::vec4& val);
    virtual void BindString(const Util::String& name, const Util::String& val);
    virtual void BindMat4(const Util::String& name, const Math::mat4& val);
    virtual void BindBlob(const Util::String& name, const Util::Blob& val);
    virtual void BindGuid(const Util::String& name, const Util::Guid& val);
    virtual void BindInt(const Attr::AttrId& id, int val);
    virtual void BindFloat(const Attr::AttrId& id, float val);
    virtual void BindBool(const Attr::AttrId& id, bool val);
    virtual void BindVec4(const Attr::AttrId& id, const Math::vec4& val);
    virtual void BindString(const Attr::AttrId& id, const Util::String& val);
    virtual void BindMat4(const Attr::AttrId& id, const Math::mat4& val);
    virtual void BindBlob(const Attr::AttrId& id, const Util::Blob& val);
    virtual void BindGuid(const Attr::AttrId& id, const Util::Guid& val);
private:
    void SetSqliteError();
    void ReadRow();
    sqlite3_stmt* sqliteStatement;
    Util::Array<IndexT> resultIndexMap;
};
}
#endif