#pragma once
#include "core/types.h"
#include "util/list.h"
#include "util/string.h"
namespace Core
{
class RefCounted;
class RefCountedList : public Util::List<RefCounted*>
{
public:
    void DumpLeaks();
    void SetDebugName(RefCounted* ptr, const Util::String& name);
private:
    Util::Dictionary<RefCounted*, Util::String> refcountedDebugNames;
};
}