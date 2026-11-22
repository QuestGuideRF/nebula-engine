#pragma once
#include "core/refcounted.h"
#include "attribute.h"
namespace Attr
{
class AttributeTable : public Core::RefCounted
{
    __DeclareClass(AttributeTable);
public:
    AttributeTable();
    AttributeTable(std::initializer_list<Attr::AttrId> columns, bool recordColumns = false);
    virtual ~AttributeTable();
    void SetModifiedTracking(bool b);
    bool GetModifiedTracking() const;
    bool IsModified() const;
    void ResetModifiedState();
    void Clear();
    void PrintDebug();
    void BeginAddColumns(bool recordNewColumns=true);
    void AddColumn(const AttrId& id, bool recordNewColumn=true);
    void EndAddColumns();
    bool HasColumn(const AttrId& id) const;
    IndexT GetColumnIndex(const AttrId& id) const;
    SizeT GetNumColumns() const;
    const AttrId& GetColumnId(IndexT colIndex) const;
    const Util::String& GetColumnName(IndexT colIndex) const;
    const Util::FourCC& GetColumnFourCC(IndexT colIndex) const;
    AccessMode GetColumnAccessMode(IndexT colIndex) const;
    ValueType GetColumnValueType(IndexT colIndex) const;
    const Util::Array<IndexT>& GetNewColumnIndices() const;
    const Util::Array<IndexT>& GetReadWriteColumnIndices() const;
    IndexT AddRow();
    void ClearNewRowFlags();
    void ClearDeletedRowsFlags();
    void DeleteRow(IndexT rowIndex);
    void DeleteAllRows();
    bool IsRowDeleted(IndexT rowIndex) const;
    IndexT CopyRow(IndexT rowIndex);
    void CopyRow(IndexT srcRowIndex, IndexT dstRowIndex);
    IndexT CopyExtRow(AttributeTable* other, IndexT otherRowIndex, bool createMissingRows = false);
    SizeT GetNumRows() const;
    bool HasNewRows() const;
    bool HasModifiedRows() const;
    bool HasDeletedRows() const;
    bool IsRowModified(IndexT rowIndex) const;
    const Util::Array<IndexT>& GetNewRowIndices() const;
    const Util::Array<IndexT>& GetDeletedRowIndices() const;
    Util::Array<IndexT> GetModifiedRowsExcludeNewAndDeletedRows() const;
    void ReserveRows(SizeT numRows);
    Util::Array<IndexT> FindRowIndicesByAttr(const Attribute& attr, bool firstMatchOnly) const;
    Util::Array<IndexT> FindRowIndicesByAttrs(const Util::Array<Attribute>& attrs, bool firstMatchOnly) const;
    IndexT FindRowIndexByAttr(const Attribute& attr) const;
    IndexT FindRowIndexByAttrs(const Util::Array<Attribute>& attrs) const;
    void SetRowUserData(IndexT rowIndex, void* p);
    void* GetRowUserData(IndexT rowIndex) const;
    void SetAttr(const Attr::Attribute& attr, IndexT rowIndex);
    Attr::Attribute GetAttr(IndexT rowIndex, IndexT colIndex) const;
    void SetVariant(const Attr::AttrId& attrId, IndexT rowIndex, const Util::Variant& val);
    void SetBool(const BoolAttrId& colAttrId, IndexT rowIndex, bool val);
    bool GetBool(const BoolAttrId& colAttrId, IndexT rowIndex) const;
    void SetFloat(const FloatAttrId& colAttrId, IndexT rowIndex, float val);
    float GetFloat(const FloatAttrId& colAttrId, IndexT rowIndex) const;
    void SetInt(const IntAttrId& colAttrId, IndexT rowIndex, int val);
    int GetInt(const IntAttrId& colAttrId, IndexT rowIndex) const;
    void SetString(const StringAttrId& colAttrId, IndexT rowIndex, const Util::String& val);
    const Util::String& GetString(const StringAttrId& colAttrId, IndexT rowIndex) const;
    void SetVec4(const Vec4AttrId& colAttrId, IndexT rowIndex, const Math::vec4& val);
    Math::vec4 GetVec4(const Vec4AttrId& colAttrId, IndexT rowIndex) const;
    void SetMat4(const Mat4AttrId& colAttrId, IndexT rowIndex, const Math::mat4& val);
    Math::mat4 GetMat4(const Mat4AttrId& colAttrId, IndexT rowIndex) const;
    void SetGuid(const GuidAttrId& colAttrId, IndexT rowIndex, const Util::Guid& guid);
    const Util::Guid& GetGuid(const GuidAttrId& colAttrId, IndexT rowIndex) const;
    void SetBlob(const BlobAttrId& colAttrId, IndexT rowIndex, const Util::Blob& blob);
    const Util::Blob& GetBlob(const BlobAttrId& colAttrId, IndexT rowIndex) const;
    void SetVariant(IndexT colIndex, IndexT rowIndex, const Util::Variant& val);
    void SetBool(IndexT colIndex, IndexT rowIndex, bool val);
    bool GetBool(IndexT colIndex, IndexT rowIndex) const;
    void SetFloat(IndexT colIndex, IndexT rowIndex, float val);
    float GetFloat(IndexT colIndex, IndexT rowIndex) const;
    void SetInt(IndexT colIndex, IndexT rowIndex, int val);
    int GetInt(IndexT colIndex, IndexT rowIndex) const;
    void SetUInt(IndexT colIndex, IndexT rowIndex, uint val);
    uint GetUInt(IndexT colIndex, IndexT rowIndex) const;
    void SetString(IndexT colIndex, IndexT rowIndex, const Util::String& val);
    const Util::String& GetString(IndexT colIndex, IndexT rowIndex) const;
    void SetVec4(IndexT colIndex, IndexT rowIndex, const Math::vec4& val);
    Math::vec4 GetVec4(IndexT colIndex, IndexT rowIndex) const;
    void SetMat4(IndexT colIndex, IndexT rowIndex, const Math::mat4& val);
    Math::mat4 GetMat4(IndexT colIndex, IndexT rowIndex) const;
    void SetGuid(IndexT colIndex, IndexT rowIndex, const Util::Guid& guid);
    const Util::Guid& GetGuid(IndexT colIndex, IndexT rowIndex) const;
    void SetBlob(IndexT colIndex, IndexT rowIndex, const Util::Blob& blob);
    const Util::Blob& GetBlob(IndexT colIndex, IndexT rowIndex) const;
    bool LoadXmlTable(const Util::String& fileName);
private:
    void Realloc(SizeT newPitch, SizeT newAllocRows);
    SizeT UpdateColumnOffsets();
    SizeT GetValueTypeSize(ValueType type) const;
    void* GetValuePtr(IndexT colIndex, IndexT rowIndex) const;
    void Delete();
    void DeleteRowData(IndexT rowIndex);
    void DeleteString(IndexT colIndex, IndexT rowIndex);
    void CopyString(IndexT colIndex, IndexT rowIndex, const Util::String& val);
    void DeleteBlob(IndexT colIndex, IndexT rowIndex);
    void CopyBlob(IndexT colIndex, IndexT rowIndex, const Util::Blob& val);
    void DeleteGuid(IndexT colIndex, IndexT rowIndex);
    void CopyGuid(IndexT colIndex, IndexT rowIndex, const Util::Guid& val);
    Util::Array<IndexT> InternalFindRowIndicesByAttr(const Attr::Attribute& attr, bool firstMatchOnly) const;
    Util::Array<IndexT> InternalFindRowIndicesByAttrs(const Util::Array<Attr::Attribute>& attr, bool firstMatchOnly) const;
    void InternalAddColumnHelper(const AttrId& id, bool recordAsNewColumn);
    void SetColumnToDefaultValues(IndexT colIndex);
    void SetRowToDefaultValues(IndexT rowIndex);
    void SetColumnBool(const BoolAttrId& attrId, bool val);
    void SetColumnFloat(const FloatAttrId& attrId, float val);
    void SetColumnInt(const IntAttrId& attrId, int val);
    void SetColumnString(const StringAttrId& attrId, const Util::String& val);
    void SetColumnVec4(const Vec4AttrId& attrId, const Math::vec4& val);
    void SetColumnMat4(const Mat4AttrId& attrId, const Math::mat4& val);
    void SetColumnGuid(const GuidAttrId& attrId, const Util::Guid& guid);
    void SetColumnBlob(const BlobAttrId& attrId, const Util::Blob& blob);
    struct ColumnInfo
    {
        AttrId attrId;
        IndexT byteOffset;
    };
    Util::Array<ColumnInfo> columns;
    Util::Dictionary<AttrId,IndexT> indexMap;
    Util::Array<IndexT> readWriteColumnIndices;
    Util::Array<IndexT> newColumnIndices;
    Util::Array<IndexT> newRowIndices;
    Util::Array<IndexT> deletedRowIndices;
    Util::Array<void*>  userData;
    SizeT rowPitch;
    SizeT numRows;
    SizeT allocatedRows;
    void* valueBuffer;
    uchar* rowModifiedBuffer;
    uchar* rowDeletedBuffer;
    uchar* rowNewBuffer;
    bool trackModifications;
    bool isModified;
    bool rowsModified;
    bool inBeginAddColumns;
    bool addColumnsRecordAsNewColumns;
    IndexT firstNewColumnIndex;
};
inline
void
AttributeTable::SetModifiedTracking(bool b)
{
    this->trackModifications = b;
}
inline
bool
AttributeTable::GetModifiedTracking() const
{
    return this->trackModifications;
}
inline
bool
AttributeTable::HasNewRows() const
{
    return this->newRowIndices.Size() > 0;
}
inline
bool
AttributeTable::HasDeletedRows() const
{
    return this->deletedRowIndices.Size() > 0;
}
inline
bool
AttributeTable::HasModifiedRows() const
{
    return this->rowsModified;
}
inline
const Util::Array<IndexT>&
AttributeTable::GetReadWriteColumnIndices() const
{
    return this->readWriteColumnIndices;
}
inline bool
AttributeTable::IsRowModified(IndexT rowIndex) const
{
    n_assert(rowIndex < this->numRows);
    n_assert(0 != this->rowModifiedBuffer);
    return (0 != this->rowModifiedBuffer[rowIndex]);
}
inline bool
AttributeTable::IsRowDeleted(IndexT rowIndex) const
{
    n_assert(rowIndex < this->numRows);
    n_assert(0 != this->rowDeletedBuffer);
    return (this->rowDeletedBuffer[rowIndex] == 1);
}
inline bool
AttributeTable::IsModified() const
{
    return this->isModified;
}
inline const Util::Array<IndexT>&
AttributeTable::GetNewRowIndices() const
{
    return this->newRowIndices;
}
inline const Util::Array<IndexT>&
AttributeTable::GetDeletedRowIndices() const
{
    return this->deletedRowIndices;
}
inline const Util::Array<IndexT>&
AttributeTable::GetNewColumnIndices() const
{
    return this->newColumnIndices;
}
inline bool
AttributeTable::HasColumn(const AttrId& id) const
{
    return this->indexMap.Contains(id);
}
inline IndexT
AttributeTable::GetColumnIndex(const AttrId& id) const
{
    return this->indexMap[id];
}
inline SizeT
AttributeTable::GetNumColumns() const
{
    return this->columns.Size();
}
inline const AttrId&
AttributeTable::GetColumnId(IndexT colIndex) const
{
    return this->columns[colIndex].attrId;
}
inline const Util::String&
AttributeTable::GetColumnName(IndexT colIndex) const
{
    return this->columns[colIndex].attrId.GetName();
}
inline const Util::FourCC&
AttributeTable::GetColumnFourCC(IndexT colIndex) const
{
    return this->columns[colIndex].attrId.GetFourCC();
}
inline AccessMode
AttributeTable::GetColumnAccessMode(IndexT colIndex) const
{
    return this->columns[colIndex].attrId.GetAccessMode();
}
inline ValueType
AttributeTable::GetColumnValueType(IndexT colIndex) const
{
    return this->columns[colIndex].attrId.GetValueType();
}
inline SizeT
AttributeTable::GetNumRows() const
{
    return this->numRows;
}
inline void*
AttributeTable::GetValuePtr(IndexT colIndex, IndexT rowIndex) const
{
    n_assert((colIndex < this->columns.Size()) && (rowIndex < this->numRows));
    IndexT bufferOffset = (rowIndex * this->rowPitch) + this->columns[colIndex].byteOffset;
    return (void*)((char*)this->valueBuffer + bufferOffset);
}
inline void
AttributeTable::SetBool(IndexT colIndex, IndexT rowIndex, bool val)
{
    n_assert(this->GetColumnValueType(colIndex) == BoolType);
    n_assert(!this->IsRowDeleted(rowIndex));
    int* valuePtr = (int*) this->GetValuePtr(colIndex, rowIndex);
    *valuePtr = val ? 1 : 0;
    this->rowModifiedBuffer[rowIndex] = 1;
    this->isModified = true;
}
inline bool
AttributeTable::GetBool(IndexT colIndex, IndexT rowIndex) const
{
    n_assert(this->GetColumnValueType(colIndex) == BoolType);
    int* valuePtr = (int*) this->GetValuePtr(colIndex, rowIndex);
    return (*valuePtr != 0);
}
inline void
AttributeTable::SetFloat(IndexT colIndex, IndexT rowIndex, float val)
{
    n_assert(this->GetColumnValueType(colIndex) == FloatType);
    n_assert(!this->IsRowDeleted(rowIndex));
    float* valuePtr = (float*) this->GetValuePtr(colIndex, rowIndex);
    *valuePtr = val;
    this->rowModifiedBuffer[rowIndex] = 1;
    this->isModified = true;
}
inline float
AttributeTable::GetFloat(IndexT colIndex, IndexT rowIndex) const
{
    n_assert(this->GetColumnValueType(colIndex) == FloatType);
    float* valuePtr = (float*) this->GetValuePtr(colIndex, rowIndex);
    return *valuePtr;
}
inline void
AttributeTable::SetInt(IndexT colIndex, IndexT rowIndex, int val)
{
    n_assert(this->GetColumnValueType(colIndex) == IntType);
    n_assert(!this->IsRowDeleted(rowIndex));
    int* valuePtr = (int*) this->GetValuePtr(colIndex, rowIndex);
    *valuePtr = val;
    this->rowModifiedBuffer[rowIndex] = 1;
    this->isModified = true;
}
inline int
AttributeTable::GetInt(IndexT colIndex, IndexT rowIndex) const
{
    n_assert(this->GetColumnValueType(colIndex) == IntType);
    int* valuePtr = (int*) this->GetValuePtr(colIndex, rowIndex);
    return *valuePtr;
}
inline void
AttributeTable::SetUInt(IndexT colIndex, IndexT rowIndex, uint val)
{
    n_assert(this->GetColumnValueType(colIndex) == UIntType);
    n_assert(!this->IsRowDeleted(rowIndex));
    uint* valuePtr = (uint*)this->GetValuePtr(colIndex, rowIndex);
    *valuePtr = val;
    this->rowModifiedBuffer[rowIndex] = 1;
    this->isModified = true;
}
inline uint
AttributeTable::GetUInt(IndexT colIndex, IndexT rowIndex) const
{
    n_assert(this->GetColumnValueType(colIndex) == UIntType);
    uint* valuePtr = (uint*)this->GetValuePtr(colIndex, rowIndex);
    return *valuePtr;
}
inline void
AttributeTable::SetVec4(IndexT colIndex, IndexT rowIndex, const Math::vec4& val)
{
    n_assert(this->GetColumnValueType(colIndex) == Vec4Type);
    n_assert(!this->IsRowDeleted(rowIndex));
    Math::scalar* valuePtr = (Math::scalar*) this->GetValuePtr(colIndex, rowIndex);
    val.storeu(valuePtr);
    if (this->trackModifications)
    {
        this->rowModifiedBuffer[rowIndex] = 1;
        this->isModified = true;
        this->rowsModified = true;
    }
}
inline Math::vec4
AttributeTable::GetVec4(IndexT colIndex, IndexT rowIndex) const
{
    n_assert(this->GetColumnValueType(colIndex) == Vec4Type);
    Math::scalar* valuePtr = (Math::scalar*) this->GetValuePtr(colIndex, rowIndex);
    Math::vec4 vec;
    vec.loadu(valuePtr);
    return vec;
}
inline void
AttributeTable::SetMat4(IndexT colIndex, IndexT rowIndex, const Math::mat4& val)
{
    n_assert(this->GetColumnValueType(colIndex) == Mat4Type);
    n_assert(!this->IsRowDeleted(rowIndex));
    Math::scalar* valuePtr = (Math::scalar*) this->GetValuePtr(colIndex, rowIndex);
    val.storeu(valuePtr);
    if (this->trackModifications)
    {
        this->rowModifiedBuffer[rowIndex] = 1;
        this->isModified = true;
        this->rowsModified = true;
    }
}
inline Math::mat4
AttributeTable::GetMat4(IndexT colIndex, IndexT rowIndex) const
{
    n_assert(this->GetColumnValueType(colIndex) == Mat4Type);
    Math::scalar* valuePtr = (Math::scalar*) this->GetValuePtr(colIndex, rowIndex);
    Math::mat4 mx;
    mx.loadu(valuePtr);
    return mx;
}
inline void
AttributeTable::SetString(IndexT colIndex, IndexT rowIndex, const Util::String& val)
{
    n_assert(this->GetColumnValueType(colIndex) == StringType);
    n_assert(!this->IsRowDeleted(rowIndex));
    this->CopyString(colIndex, rowIndex, val);
    if (this->trackModifications)
    {
        this->rowModifiedBuffer[rowIndex] = 1;
        this->isModified = true;
        this->rowsModified = true;
    }
}
inline const Util::String&
AttributeTable::GetString(IndexT colIndex, IndexT rowIndex) const
{
    n_assert(this->GetColumnValueType(colIndex) == StringType);
    Util::String** valuePtr = (Util::String**) this->GetValuePtr(colIndex, rowIndex);
    return **valuePtr;
}
inline void
AttributeTable::SetGuid(IndexT colIndex, IndexT rowIndex, const Util::Guid& val)
{
    n_assert(this->GetColumnValueType(colIndex) == GuidType);
    n_assert(!this->IsRowDeleted(rowIndex));
    this->CopyGuid(colIndex, rowIndex, val);
    if (this->trackModifications)
    {
        this->rowModifiedBuffer[rowIndex] = 1;
        this->isModified = true;
        this->rowsModified = true;
    }
}
inline const Util::Guid&
AttributeTable::GetGuid(IndexT colIndex, IndexT rowIndex) const
{
    n_assert(this->GetColumnValueType(colIndex) == GuidType);
    Util::Guid** valuePtr = (Util::Guid**) this->GetValuePtr(colIndex, rowIndex);
    return **valuePtr;
}
inline void
AttributeTable::SetBlob(IndexT colIndex, IndexT rowIndex, const Util::Blob& val)
{
    n_assert(this->GetColumnValueType(colIndex) == BlobType);
    n_assert(!this->IsRowDeleted(rowIndex));
    this->CopyBlob(colIndex, rowIndex, val);
    if (this->trackModifications)
    {
        this->rowModifiedBuffer[rowIndex] = 1;
        this->isModified = true;
        this->rowsModified = true;
    }
}
inline const Util::Blob&
AttributeTable::GetBlob(IndexT colIndex, IndexT rowIndex) const
{
    n_assert(this->GetColumnValueType(colIndex) == BlobType);
    Util::Blob** valuePtr = (Util::Blob**) this->GetValuePtr(colIndex, rowIndex);
    return **valuePtr;
}
inline void
AttributeTable::SetBool(const BoolAttrId& colAttrId, IndexT rowIndex, bool val)
{
    this->SetBool(this->indexMap[colAttrId], rowIndex, val);
}
inline bool
AttributeTable::GetBool(const BoolAttrId& colAttrId, IndexT rowIndex) const
{
    return this->GetBool(this->indexMap[colAttrId], rowIndex);
}
inline void
AttributeTable::SetFloat(const FloatAttrId& colAttrId, IndexT rowIndex, float val)
{
    this->SetFloat(this->indexMap[colAttrId], rowIndex, val);
}
inline float
AttributeTable::GetFloat(const FloatAttrId& colAttrId, IndexT rowIndex) const
{
    return this->GetFloat(this->indexMap[colAttrId], rowIndex);
}
inline void
AttributeTable::SetInt(const IntAttrId& colAttrId, IndexT rowIndex, int val)
{
    this->SetInt(this->indexMap[colAttrId], rowIndex, val);
}
inline int
AttributeTable::GetInt(const IntAttrId& colAttrId, IndexT rowIndex) const
{
    return this->GetInt(this->indexMap[colAttrId], rowIndex);
}
inline void
AttributeTable::SetVec4(const Vec4AttrId& colAttrId, IndexT rowIndex, const Math::vec4& val)
{
    this->SetVec4(this->indexMap[colAttrId], rowIndex, val);
}
inline Math::vec4
AttributeTable::GetVec4(const Vec4AttrId& colAttrId, IndexT rowIndex) const
{
    return this->GetVec4(this->indexMap[colAttrId], rowIndex);
}
inline void
AttributeTable::SetMat4(const Mat4AttrId& colAttrId, IndexT rowIndex, const Math::mat4& val)
{
    this->SetMat4(this->indexMap[colAttrId], rowIndex, val);
}
inline Math::mat4
AttributeTable::GetMat4(const Mat4AttrId& colAttrId, IndexT rowIndex) const
{
    return this->GetMat4(this->indexMap[colAttrId], rowIndex);
}
inline void
AttributeTable::SetString(const StringAttrId& colAttrId, IndexT rowIndex, const Util::String& val)
{
    this->SetString(this->indexMap[colAttrId], rowIndex, val);
}
inline const Util::String&
AttributeTable::GetString(const StringAttrId& colAttrId, IndexT rowIndex) const
{
    return this->GetString(this->indexMap[colAttrId], rowIndex);
}
inline void
AttributeTable::SetGuid(const GuidAttrId& colAttrId, IndexT rowIndex, const Util::Guid& val)
{
    this->SetGuid(this->indexMap[colAttrId], rowIndex, val);
}
inline const Util::Guid&
AttributeTable::GetGuid(const GuidAttrId& colAttrId, IndexT rowIndex) const
{
    return this->GetGuid(this->indexMap[colAttrId], rowIndex);
}
inline void
AttributeTable::SetBlob(const BlobAttrId& colAttrId, IndexT rowIndex, const Util::Blob& val)
{
    this->SetBlob(this->indexMap[colAttrId], rowIndex, val);
}
inline const Util::Blob&
AttributeTable::GetBlob(const BlobAttrId& colAttrId, IndexT rowIndex) const
{
    return this->GetBlob(this->indexMap[colAttrId], rowIndex);
}
}