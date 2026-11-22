#pragma once
#include "util/keyvaluepair.h"
namespace IO
{
class Assign : public Util::KeyValuePair<Util::String, Util::String>
{
public:
    Assign();
    Assign(const Util::String& name, const Util::String& path);
    const Util::String& GetName() const;
    const Util::String& GetPath() const;
};
inline
Assign::Assign()
{
}
inline
Assign::Assign(const Util::String& name, const Util::String& path) :
    Util::KeyValuePair<Util::String,Util::String>::KeyValuePair(name, path)
{
    n_assert(name.IsValid());
    n_assert(path.IsValid());
}
inline
const Util::String&
Assign::GetName() const
{
    n_assert(this->keyData.IsValid());
    return this->keyData;
}
inline
const Util::String&
Assign::GetPath() const
{
    n_assert(this->valueData.IsValid());
    return this->valueData;
}
}