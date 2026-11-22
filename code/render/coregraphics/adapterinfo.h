#pragma once
#include "core/types.h"
#include "util/string.h"
#include "util/guid.h"
namespace CoreGraphics
{
class AdapterInfo
{
public:
    AdapterInfo();
    void SetDriverName(const Util::String& s);
    const Util::String& GetDriverName() const;
    void SetDescription(const Util::String& s);
    const Util::String& GetDescription() const;
    void SetDeviceName(const Util::String& s);
    const Util::String& GetDeviceName() const;
    void SetDriverVersionLowPart(uint v);
    uint GetDriverVersionLowPart() const;
    void SetDriverVersionHighPart(uint v);
    uint GetDriverVersionHighPart() const;
    void SetVendorId(uint id);
    uint GetVendorId() const;
    void SetDeviceId(uint id);
    uint GetDeviceId() const;
    void SetSubSystemId(uint id);
    uint GetSubSystemId() const;
    void SetRevision(uint r);
    uint GetRevision() const;
    void SetGuid(const Util::Guid& g);
    const Util::Guid& GetGuid() const;
private:
    Util::String driverName;
    Util::String description;
    Util::String deviceName;
    uint driverVersionLowPart;
    uint driverVersionHighPart;
    uint vendorId;
    uint deviceId;
    uint subSystemId;
    uint revision;
    Util::Guid guid;
};
inline
AdapterInfo::AdapterInfo() :
    driverVersionLowPart(0),
    driverVersionHighPart(0),
    vendorId(0),
    deviceId(0),
    subSystemId(0),
    revision(0)
{
}
inline void
AdapterInfo::SetDriverName(const Util::String& s)
{
    this->driverName = s;
}
inline const Util::String&
AdapterInfo::GetDriverName() const
{
    return this->driverName;
}
inline void
AdapterInfo::SetDescription(const Util::String& s)
{
    this->description = s;
}
inline const Util::String&
AdapterInfo::GetDescription() const
{
    return this->description;
}
inline void
AdapterInfo::SetDeviceName(const Util::String& s)
{
    this->deviceName = s;
}
inline const Util::String&
AdapterInfo::GetDeviceName() const
{
    return this->deviceName;
}
inline void
AdapterInfo::SetDriverVersionLowPart(uint v)
{
    this->driverVersionLowPart = v;
}
inline uint
AdapterInfo::GetDriverVersionLowPart() const
{
    return this->driverVersionLowPart;
}
inline void
AdapterInfo::SetDriverVersionHighPart(uint v)
{
    this->driverVersionHighPart = v;
}
inline uint
AdapterInfo::GetDriverVersionHighPart() const
{
    return this->driverVersionHighPart;
}
inline void
AdapterInfo::SetVendorId(uint id)
{
    this->vendorId = id;
}
inline uint
AdapterInfo::GetVendorId() const
{
    return this->vendorId;
}
inline void
AdapterInfo::SetDeviceId(uint id)
{
    this->deviceId = id;
}
inline uint
AdapterInfo::GetDeviceId() const
{
    return this->deviceId;
}
inline void
AdapterInfo::SetSubSystemId(uint id)
{
    this->subSystemId = id;
}
inline uint
AdapterInfo::GetSubSystemId() const
{
    return this->subSystemId;
}
inline void
AdapterInfo::SetRevision(uint r)
{
    this->revision = r;
}
inline uint
AdapterInfo::GetRevision() const
{
    return this->revision;
}
inline void
AdapterInfo::SetGuid(const Util::Guid& g)
{
    this->guid = g;
}
inline const Util::Guid&
AdapterInfo::GetGuid() const
{
    return this->guid;
}
}