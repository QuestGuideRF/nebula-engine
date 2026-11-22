#pragma once
#include "core/refcounted.h"
#include "attributeid.h"
#include "memdb/table.h"
namespace MemDb
{
class Database;
struct Dataset
{
    struct View
    {
#ifdef NEBULA_DEBUG
        Util::String tableName;
#endif
        TableId tid;
        SizeT numInstances = 0;
        Util::StackArray<void*, 16> buffers;
    };
    Util::Array<View> tables;
private:
    friend class Database;
    Ptr<Database> db;
};
}