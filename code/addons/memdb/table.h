#pragma once
#include "ids/id.h"
#include "util/arrayallocator.h"
#include "util/fixedarray.h"
#include "util/string.h"
#include "util/stringatom.h"
#include "util/hashtable.h"
#include "attributeid.h"
#include "tablesignature.h"
#include "util/bitfield.h"
#include "util/queue.h"
#include "ids/idgenerationpool.h"
#include "tableid.h"
#include <functional>
namespace MemDb
{
struct TableCreateInfo
{
    Util::String name;
    AttributeId const* attributeIds;
    SizeT numAttributes;
};
class Table
{
    using ColumnBuffer = void*;
private:
    friend class Database;
    Table() = default;
    ~Table();
public:
    class Partition;
    bool HasAttribute(AttributeId attribute) const;
    AttributeId GetAttributeId(ColumnIndex columnIndex) const;
    ColumnIndex GetAttributeIndex(AttributeId attribute) const;
    Util::Array<AttributeId> const& GetAttributes() const;
    TableSignature const& GetSignature() const;
    ColumnIndex AddAttribute(AttributeId attribute, bool updateSignature = true);
    RowId AddRow();
    void RemoveRow(RowId row);
    SizeT GetNumRows() const;
    void SetNumRows(SizeT value);
    void SetToDefault(RowId row);
    SizeT Defragment(std::function<void(Partition*, RowId, RowId)> const& moveCallback);
    void Clean();
    void Reset();
    Partition* GetFirstActivePartition();
    uint16_t GetNumActivePartitions() const;
    uint16_t GetNumPartitions() const;
    Partition* GetCurrentPartition();
    Partition* GetPartition(uint16_t partitionId);
    void* GetValuePointer(ColumnIndex cid, RowId row);
    void* GetBuffer(uint16_t partition, ColumnIndex cid);
    Util::Blob SerializeInstance(RowId row) const;
    void DeserializeInstance(Util::Blob const& data, RowId row);
    static RowId MigrateInstance(
        Table& src,
        RowId srcRow,
        Table& dst,
        bool defragment = true,
        std::function<void(Partition*, RowId, RowId)> const& moveCallback = nullptr
    );
    static RowId DuplicateInstance(Table const& src, RowId srcRow, Table& dst);
    static void MigrateInstances(
        Table& src,
        Util::Array<RowId> const& srcRows,
        Table& dst,
        Util::FixedArray<RowId>& dstRows,
        bool defragment = true,
        std::function<void(Partition*, IndexT, IndexT)> const& moveCallback = nullptr
    );
    static void DuplicateInstances(Table& src, Util::Array<RowId> const& srcRows, Table& dst, Util::FixedArray<RowId>& dstRows);
    static constexpr Memory::HeapType HEAP_MEMORY_TYPE = Memory::HeapType::DefaultHeap;
    Util::StringAtom name;
    Partition* NewPartition();
private:
    TableSignature signature;
    TableId tid = TableId::Invalid();
    uint32_t totalNumRows = 0;
    Partition* currentPartition = nullptr;
    Util::Array<Partition*> partitions;
    Util::Array<Partition*> freePartitions;
    Util::Array<uint16_t> nullPartitions;
    Partition* firstActivePartition = nullptr;
    uint16_t numActivePartitions = 0;
    Util::Array<AttributeId> attributes;
    Util::HashTable<AttributeId, IndexT, 32, 1> columnRegistry;
};
class Table::Partition
{
private:
    Partition() = default;
    ~Partition();
public:
    static constexpr uint CAPACITY = 256;
    Table* table = nullptr;
    Partition* next = nullptr;
    Partition* previous = nullptr;
    uint16_t partitionId = 0xFFFF;
    uint32_t numRows = 0;
    uint64_t version = 0;
    Util::Array<uint16_t> freeIds;
    Util::Array<ColumnBuffer> columns;
    Util::BitField<CAPACITY> modifiedRows;
    Util::BitField<CAPACITY> validRows;
private:
    friend Table;
    uint16_t AllocateRowIndex();
    void FreeIndex(uint16_t instance);
    void EraseSwapIndex(uint16_t instance);
};
}