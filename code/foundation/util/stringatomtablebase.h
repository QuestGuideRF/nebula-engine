#pragma once
#include "util/array.h"
namespace Util
{
class StringAtomTableBase
{
public:
    StringAtomTableBase();
    ~StringAtomTableBase();
protected:
    friend class StringAtom;
    const char* Find(const char* str) const;
    struct StaticString
    {
        bool operator==(const StaticString& rhs) const;
        bool operator!=(const StaticString& rhs) const;
        bool operator<(const StaticString& rhs) const;
        bool operator>(const StaticString& rhs) const;
        char* ptr;
    };
    Util::Array<StaticString> table;
};
}