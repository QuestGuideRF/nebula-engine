#pragma once
#include "util/stringatom.h"
#include "util/hashtable.h"
namespace Base
{
    class ShaderServerBase;
}
namespace CoreGraphics
{
class ShaderFeature
{
public:
    typedef uint Mask;
    typedef Util::StringAtom Name;
private:
    friend class Base::ShaderServerBase;
    ShaderFeature();
    Mask StringToMask(const Util::String& str);
    Util::String MaskToString(Mask mask);
    Util::HashTable<Mask, Name> nameHash;
};
}