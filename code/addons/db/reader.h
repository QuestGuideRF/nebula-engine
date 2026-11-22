#ifndef DB_READER_H
#define DB_READER_H
#include "core/refcounted.h"
#include "attr/attribute.h"
#include "db/dataset.h"
#include "db/valuetable.h"
#include "util/guid.h"
#include "util/blob.h"
#include "db/database.h"
namespace Db
{
class Reader : public Core::RefCounted
{
    __DeclareClass(Reader);
public:
    Reader();
    virtual ~Reader();
    void SetDatabase(const Ptr<Database>& db);
    void SetTableName(const Util::String& n);
    void AddFilterAttr(const Attr::Attribute& attr);
    bool Open();
    bool IsOpen() const;
    void Close();
    int GetNumRows() const;
    void SetToRow(int rowIndex);
    int GetCurrentRowIndex() const;
    bool HasAttr(Attr::AttrId attrId) const;
    bool GetBool(Attr::BoolAttrId attrId) const;
    int GetInt(Attr::IntAttrId attrId) const;
    float GetFloat(Attr::FloatAttrId attrId) const;
    const Util::String& GetString(Attr::StringAttrId attrId) const;
    const Math::vec4 GetVec4(Attr::Vec4AttrId attrId) const;
    const Math::mat4 GetMat4(Attr::Mat4AttrId attrId) const;
    const Util::Guid& GetGuid(Attr::GuidAttrId attrId) const;
    const Util::Blob& GetBlob(Attr::BlobAttrId attrId) const;
    const Ptr<Db::ValueTable>& GetValueTable() const;
private:
    bool isOpen;
    Ptr<Database> database;
    Util::String tableName;
    Util::Array<Attr::Attribute> filterAttrs;
    Ptr<Dataset> dataset;
    Ptr<ValueTable> valueTable;
    IndexT curRowIndex;
};
inline void
Reader::SetDatabase(const Ptr<Database>& db)
{
    this->database = db;
}
inline bool
Reader::IsOpen() const
{
    return this->isOpen;
}
inline const Ptr<Db::ValueTable>&
Reader::GetValueTable() const
{
    return this->valueTable;
}
inline void
Reader::SetTableName(const Util::String& t)
{
    this->tableName = t;
}
inline void
Reader::AddFilterAttr(const Attr::Attribute& attr)
{
    this->filterAttrs.Append(attr);
}
inline int
Reader::GetNumRows() const
{
    n_assert(this->isOpen);
    return this->dataset->Values()->GetNumRows();
}
inline void
Reader::SetToRow(int rowIndex)
{
    n_assert(this->isOpen);
    n_assert((rowIndex >= 0) && (rowIndex < this->GetNumRows()));
    this->curRowIndex = rowIndex;
}
inline int
Reader::GetCurrentRowIndex() const
{
    return this->curRowIndex;
}
};
#endif