#pragma once
#include "core/ptr.h"
#include "category.h"
#include "entity.h"
#include "memdb/tableid.h"
#include "filter.h"
#include "dataset.h"
namespace MemDb
{
class Database;
}
namespace Game
{
class World;
#define WORLD_DEFAULT Game::WorldHash{'DWLD'}
void DestroyFilter(Filter);
Dataset Query(Ptr<MemDb::Database> const& db, Util::Array<MemDb::TableId>& tables, Filter filter);
void ReleaseDatasets();
BlueprintId GetBlueprintId(Util::StringAtom name);
TemplateId GetTemplateId(Util::StringAtom name);
}