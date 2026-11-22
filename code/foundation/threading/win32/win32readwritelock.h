#pragma once
namespace Win32
{
class Win32ReadWriteLock
{
public:
    Win32ReadWriteLock();
    ~Win32ReadWriteLock();
    void LockRead();
    void LockWrite();
    void UnlockRead();
    void UnlockWrite();
private:
    SRWLOCK lock;
};
}