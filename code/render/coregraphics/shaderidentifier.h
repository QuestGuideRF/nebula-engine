#pragma once
#include "core/types.h"
#include "util/stringatom.h"
#include "util/array.h"
#include "util/dictionary.h"
namespace Base
{
    class ShaderServerBase;
}
namespace CoreGraphics
{
class ShaderIdentifier
{
public:
    typedef Util::StringAtom Name;
    typedef IndexT Code;
    static Code FromName(const Name& name);
    static Name ToName(Code c);
    static const IndexT MaxNumShaderIdentifiers = 128;
    static const IndexT InvalidBatchType = InvalidIndex;
private:
    friend class Base::ShaderServerBase;
    ShaderIdentifier();
    Util::Dictionary<Name, IndexT> nameToCode;
    Util::Array<Name> codeToName;
};
}