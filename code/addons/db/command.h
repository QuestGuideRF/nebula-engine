#pragma once
#ifndef DB_COMMAND_H
#define DB_COMMAND_H
#include "core/refcounted.h"
#include "attr/attrid.h"
#include "util/string.h"
#include "db/valuetable.h"
namespace Db
{
class Database;
class Command : public Core::RefCounted
{
public:
    Command();
    virtual ~Command();
    virtual bool Compile(const Ptr<Database>& db, const Util::String& sqlCommand, ValueTable* resultTable=0);
    virtual bool Execute() = 0;
    bool CompileAndExecute(const Ptr<Database>& db, const Util::String& sqlCommand, ValueTable* resultTable=0);
    virtual void Clear() = 0;
    virtual bool IsValid() const = 0;
    const Util::String& GetError() const;
    const Ptr<Database>& GetDatabase() const;
    const Util::String& GetSqlCommand() const;
    const Ptr<ValueTable>& GetValueTable() const;
    virtual IndexT IndexOf(const Util::String& name) const =  0;
    virtual IndexT IndexOf(const Attr::AttrId& attrId) const = 0;
    virtual void BindInt(IndexT index, int val) = 0;
    virtual void BindFloat(IndexT index, float val) = 0;
    virtual void BindBool(IndexT index, bool val) = 0;
    virtual void BindVec4(IndexT index, const Math::vec4& val) = 0;
    virtual void BindString(IndexT index, const Util::String& val) = 0;
    virtual void BindMat4(IndexT index, const Math::mat4& val) = 0;
    virtual void BindBlob(IndexT index, const Util::Blob& val) = 0;
    virtual void BindGuid(IndexT index, const Util::Guid& val) = 0;
    virtual void BindInt(const Util::String& name, int val) = 0;
    virtual void BindFloat(const Util::String& name, float val) = 0;
    virtual void BindBool(const Util::String& name, bool val) = 0;
    virtual void BindVec4(const Util::String& name, const Math::vec4& val) = 0;
    virtual void BindString(const Util::String& name, const Util::String& val) = 0;
    virtual void BindMat4(const Util::String& name, const Math::mat4& val) = 0;
    virtual void BindBlob(const Util::String& name, const Util::Blob& val) = 0;
    virtual void BindGuid(const Util::String& name, const Util::Guid& val) = 0;
    virtual void BindInt(const Attr::AttrId& id, int val) = 0;
    virtual void BindFloat(const Attr::AttrId& id, float val) = 0;
    virtual void BindBool(const Attr::AttrId& id, bool val) = 0;
    virtual void BindVec4(const Attr::AttrId& id, const Math::vec4& val) = 0;
    virtual void BindString(const Attr::AttrId& id, const Util::String& val) = 0;
    virtual void BindMat4(const Attr::AttrId& id, const Math::mat4& val) = 0;
    virtual void BindBlob(const Attr::AttrId& id, const Util::Blob& val) = 0;
    virtual void BindGuid(const Attr::AttrId& id, const Util::Guid& val) = 0;
protected:
    void SetError(const Util::String& err);
    void SetSqlCommand(const Util::String& cmd);
    Util::String sqlCommand;
    Util::String error;
    Ptr<Database> database;
    Ptr<ValueTable> valueTable;
};
inline const Ptr<ValueTable>&
Command::GetValueTable() const
{
    return this->valueTable;
}
inline const Ptr<Database>&
Command::GetDatabase() const
{
    return this->database;
}
inline void
Command::SetSqlCommand(const Util::String& cmd)
{
    n_assert(cmd.IsValid());
    this->sqlCommand = cmd;
}
inline const Util::String&
Command::GetSqlCommand() const
{
    return this->sqlCommand;
}
inline void
Command::SetError(const Util::String& err)
{
    this->error = err;
}
inline const Util::String&
Command::GetError() const
{
    return this->error;
}
}
#endif