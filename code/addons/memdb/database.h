#pragma once
#include "core/refcounted.h"
#include "util/arrayallocator.h"
#include "table.h"
#include "util/stringatom.h"
#include "ids/idgenerationpool.h"
#include "attributeid.h"
#include "attributeregistry.h"
#include "dataset.h"
#include "filterset.h"
#include "util/blob.h"
namespace MemDb
{
class Database;
class Database : public Core::RefCounted
{
    __DeclareClass(MemDb::Database);
public:
    Database();
    ~Database();
    TableId CreateTable(TableCreateInfo const& info);
    void DeleteTable(TableId table);
    bool IsValid(TableId table) const;
    Table& GetTable(TableId tid);
    TableId FindTable(TableSignature const& signature) const;
    SizeT GetNumTables() const;
    void ForEachTable(std::function<void(TableId)> const& callback);
    void Reset();
    Dataset Query(FilterSet const& filterset);
    Util::Array<TableId> Query(TableSignature const& inclusive, TableSignature const& exclusive);
    void Copy(Ptr<MemDb::Database> const& dst) const;
    static constexpr uint32_t MAX_NUM_TABLES = 512;
private:
    Ids::IdGenerationPool tableIdPool;
    Table tables[MAX_NUM_TABLES];
    SizeT numTables = 0;
};
inline SizeT
Database::GetNumTables() const
{
    return this->numTables;
}
}