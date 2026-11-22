#pragma once
#include "util/stringatomtablebase.h"
#include "core/singleton.h"
#include "threading/criticalsection.h"
#include "util/stringbuffer.h"
namespace Util
{
class GlobalStringAtomTable : public StringAtomTableBase
{
    __DeclareInterfaceSingleton(GlobalStringAtomTable);
public:
    GlobalStringAtomTable();
    ~GlobalStringAtomTable();
    StringBuffer* GetGlobalStringBuffer() const;
    struct DebugInfo
    {
        Util::Array<const char*> strings;
        size_t chunkSize;
        size_t numChunks;
        size_t allocSize;
        size_t usedSize;
        bool growthEnabled;
    };
    DebugInfo GetDebugInfo() const;
private:
    friend class StringAtom;
    void Lock();
    const char* Add(const char* str);
    void Unlock();
    Threading::CriticalSection critSect;
    StringBuffer stringBuffer;
};
}