#ifndef DB_WRITER_H
#define DB_WRITER_H
#include "core/refcounted.h"
#include "attr/attrid.h"
#include "db/column.h"
#include "db/valuetable.h"
#include "db/database.h"
namespace Db
{
class Writer : public Core::RefCounted
{
    __DeclareClass(Writer);
public:
    Writer();
    virtual ~Writer();
    void SetDatabase(const Ptr<Database>& db);
    void SetTableName(const Util::String& n);
    void AddColumn(const Db::Column& col);
    void SetFlushTable(bool flushTable);
    bool Open();
    bool OpenFromValueTable(const Ptr<ValueTable>& values);
    bool IsOpen() const;
    void Close();
    void BeginRow();
    void SetBool(Attr::BoolAttrId id, bool b);
    void SetInt(Attr::IntAttrId id, int i);
    void SetFloat(Attr::FloatAttrId id, float f);
    void SetString(Attr::StringAttrId id, const Util::String& s);
    void SetVec4(Attr::Vec4AttrId id, const Math::vec4& v);
    void SetMat4(Attr::Mat4AttrId id, const Math::mat4& m);
    void SetGuid(Attr::GuidAttrId id, const Util::Guid& guid);
    void SetBlob(Attr::BlobAttrId id, const Util::Blob& blob);
    void EndRow();
private:
    IndexT FindAttrIndex(Attr::AttrId id) const;
    Ptr<Database> database;
    bool isOpen;
    bool inBeginRow;
    Util::String tableName;
    Util::Array<Column> columns;
    Util::Dictionary<Attr::AttrId,IndexT> columnMap;
    Ptr<ValueTable> valueTable;
    bool flushTable;
    IndexT rowIndex;
};
inline void
Writer::SetDatabase(const Ptr<Database>& db)
{
    this->database = db;
}
inline void
Writer::SetTableName(const Util::String& t)
{
    this->tableName = t;
}
inline void
Writer::AddColumn(const Db::Column& col)
{
    n_assert(!this->columnMap.Contains(col.GetAttrId()));
    this->columns.Append(col);
    this->columnMap.Add(col.GetAttrId(), this->columns.Size() - 1);
}
inline bool
Writer::IsOpen() const
{
    return this->isOpen;
}
inline void
Writer::SetBool(Attr::BoolAttrId id, bool b)
{
    n_assert(this->inBeginRow);
    this->valueTable->SetBool(id, this->rowIndex, b);
}
inline void
Writer::SetInt(Attr::IntAttrId id, int i)
{
    n_assert(this->inBeginRow);
    this->valueTable->SetInt(id, this->rowIndex, i);
}
inline void
Writer::SetFloat(Attr::FloatAttrId id, float f)
{
    n_assert(this->inBeginRow);
    this->valueTable->SetFloat(id, this->rowIndex, f);
}
inline void
Writer::SetString(Attr::StringAttrId id, const Util::String& s)
{
    n_assert(this->inBeginRow);
    this->valueTable->SetString(id, this->rowIndex, s);
}
inline void
Writer::SetVec4(Attr::Vec4AttrId id, const Math::vec4& v)
{
    n_assert(this->inBeginRow);
    this->valueTable->SetVec4(id, this->rowIndex, v);
}
inline void
Writer::SetMat4(Attr::Mat4AttrId id, const Math::mat4& m)
{
    n_assert(this->inBeginRow);
    this->valueTable->SetMat4(id, this->rowIndex, m);
}
inline void
Writer::SetGuid(Attr::GuidAttrId id, const Util::Guid& guid)
{
    n_assert(this->inBeginRow);
    this->valueTable->SetGuid(id, this->rowIndex, guid);
}
inline void
Writer::SetBlob(Attr::BlobAttrId id, const Util::Blob& blob)
{
    n_assert(this->inBeginRow);
    this->valueTable->SetBlob(id, this->rowIndex, blob);
}
inline void
Writer::SetFlushTable(bool flush)
{
    this->flushTable = flush;
}
};
#endif