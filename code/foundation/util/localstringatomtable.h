#pragma once
#include "util/stringatomtablebase.h"
#include "core/singleton.h"
namespace Util
{
class LocalStringAtomTable : public StringAtomTableBase
{
    __DeclareSingleton(LocalStringAtomTable);
public:
    LocalStringAtomTable();
    ~LocalStringAtomTable();
private:
    friend class StringAtom;
    void Add(const char* str);
};
}