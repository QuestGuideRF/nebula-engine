#pragma once
#include "threading/linux/linuxthreadlocaldata.h"
namespace Linux
{
class LinuxThreadLocalPtr
{
public:
    LinuxThreadLocalPtr();
    void Set(void* ptr);
    void* Get() const;
    void Clear();
private:
    IndexT slot;
};
inline void
LinuxThreadLocalPtr::Set(void* ptr)
{
    LinuxThreadLocalData::SetPointer(this->slot, ptr);
}
inline void*
LinuxThreadLocalPtr::Get() const
{
    return LinuxThreadLocalData::GetPointer(this->slot);
}
inline void
LinuxThreadLocalPtr::Clear()
{
    LinuxThreadLocalData::ClearPointer(this->slot);
}
}