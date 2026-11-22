#pragma once
#include "memdb/table.h"
namespace Game
{
struct Dataset
{
    static const uint32_t MAX_COMPONENT_BUFFERS = 64;
    struct View
    {
        MemDb::TableId tableId = MemDb::TableId::Invalid();
        uint16_t partitionId = 0xFFFF;
        uint16_t numInstances = 0;
        void* buffers[MAX_COMPONENT_BUFFERS];
        decltype(MemDb::Table::Partition::validRows) validInstances;
        decltype(MemDb::Table::Partition::modifiedRows) modifiedInstances;
    };
    uint32_t numViews = 0;
    View* views = nullptr;
};
}