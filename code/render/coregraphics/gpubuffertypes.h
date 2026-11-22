#pragma once
#include "resources/resource.h"
namespace CoreGraphics
{
namespace GpuBufferTypes
{
    enum Usage
    {
        UsageImmutable,
        UsageDynamic,
        UsageCpu,
    };
    enum Access
    {
        AccessNone          = 0,
        AccessWrite         = 1,
        AccessRead          = 2,
        AccessReadWrite     = AccessWrite + AccessRead,
    };
    enum MapType
    {
        MapRead                 = 1,
        MapWrite                = 2,
        MapReadWrite            = MapRead + MapWrite,
        MapWriteDiscard         = 4,
        MapWriteNoOverwrite     = 8,
    };
    struct SetupFlags
    {
        CoreGraphics::GpuBufferTypes::Usage usage;
        CoreGraphics::GpuBufferTypes::Access access;
    };
};
}