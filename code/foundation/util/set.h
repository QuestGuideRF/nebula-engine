#pragma once
#include "util/array.h"
#include "util/keyvaluepair.h"
namespace Util
{
template<class KEYTYPE> class Set
{
public:
    Set();
    Set(const Set<KEYTYPE>& rhs);
    void operator=(const Set<KEYTYPE>& rhs);
    SizeT Size() const;
    void Clear();
    bool IsEmpty() const;
    void Reserve(SizeT numElements);
    void Add(const KEYTYPE& value);
    void Erase(const KEYTYPE& key);
    void EraseAtIndex(IndexT index);
    IndexT FindIndex(const KEYTYPE& key) const;
    bool Contains(const KEYTYPE& key) const;
    const KEYTYPE& KeyAtIndex(IndexT index) const;
    const Array<KEYTYPE>& KeysAsArray() const;
    template<class RETURNTYPE> RETURNTYPE KeysAs() const;
protected:
    void SortIfDirty() const;
    Array<KEYTYPE> values;
};
template<class KEYTYPE>
Set<KEYTYPE>::Set()
{
}
template<class KEYTYPE>
Set<KEYTYPE>::Set(const Set<KEYTYPE>& rhs) :
    values(rhs.values)
{
}
template<class KEYTYPE> void
Set<KEYTYPE>::operator=(const Set<KEYTYPE>& rhs)
{
    this->values = rhs.values;
}
template<class KEYTYPE> void
Set<KEYTYPE>::Clear()
{
    this->values.Clear();
}
template<class KEYTYPE> SizeT
Set<KEYTYPE>::Size() const
{
    return this->values.Size();
}
template<class KEYTYPE> bool
Set<KEYTYPE>::IsEmpty() const
{
    return (0 == this->values.Size());
}
template<class KEYTYPE> void
Set<KEYTYPE>::Reserve(SizeT numElements)
{
    this->values.Reserve(numElements);
}
template<class KEYTYPE> void
Set<KEYTYPE>::Add(const KEYTYPE& key)
{
    if (!this->Contains(key))
    {
        this->values.InsertSorted(key);
    }
}
template<class KEYTYPE> void
Set<KEYTYPE>::Erase(const KEYTYPE& key)
{
    IndexT eraseIndex = this->values.BinarySearchIndex(key);
    #if NEBULA_BOUNDSCHECKS
    n_assert(InvalidIndex != eraseIndex);
    #endif
    this->values.EraseIndex(eraseIndex);
}
template<class KEYTYPE> void
Set<KEYTYPE>::EraseAtIndex(IndexT index)
{
    this->values.EraseIndex(index);
}
template<class KEYTYPE> IndexT
Set<KEYTYPE>::FindIndex(const KEYTYPE& key) const
{
    return this->values.BinarySearchIndex(key);
}
template<class KEYTYPE> bool
Set<KEYTYPE>::Contains(const KEYTYPE& key) const
{
    return (InvalidIndex != this->values.BinarySearchIndex(key));
}
template<class KEYTYPE> const KEYTYPE&
Set<KEYTYPE>::KeyAtIndex(IndexT index) const
{
    return this->values[index];
}
template<class KEYTYPE>
template<class RETURNTYPE>
RETURNTYPE
Set<KEYTYPE>::KeysAs() const
{
    RETURNTYPE result(this->Size(), this->Size());
    IndexT i;
    for (i = 0; i < this->values.Size(); i++)
    {
        result.Append(this->values[i]);
    }
    return result;
}
template<class KEYTYPE>
const Array<KEYTYPE>&
Set<KEYTYPE>::KeysAsArray() const
{
    return this->values;
}
}