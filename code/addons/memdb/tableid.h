#pragma once
namespace MemDb
{
ID_32_TYPE(TableId);
struct RowId
{
    uint16_t partition;
    uint16_t index;
    bool
    operator!=(RowId const& rhs)
    {
        return partition != rhs.partition || index != rhs.index;
    }
    bool
    operator==(RowId const& rhs)
    {
        return partition == rhs.partition && index == rhs.index;
    }
};
constexpr RowId InvalidRow = {0xFFFF, 0xFFFF};
ID_16_TYPE(ColumnIndex);
}