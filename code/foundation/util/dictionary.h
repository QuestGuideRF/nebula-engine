#pragma once
#include "util/array.h"
#include "util/keyvaluepair.h"
namespace Util
{
template<class KEYTYPE, class VALUETYPE> class Dictionary
{
public:
    Dictionary();
    Dictionary(const Dictionary<KEYTYPE, VALUETYPE>& rhs);
    Dictionary(Dictionary<KEYTYPE, VALUETYPE>&& rhs) noexcept;
    void operator=(const Dictionary<KEYTYPE, VALUETYPE>& rhs);
    void operator=(Dictionary<KEYTYPE, VALUETYPE>&& rhs) noexcept;
    VALUETYPE& operator[](const KEYTYPE& key);
    const VALUETYPE& operator[](const KEYTYPE& key) const;
    SizeT Size() const;
    void Clear();
    bool IsEmpty() const;
    void Reserve(SizeT numElements);
    void BeginBulkAdd();
    IndexT Add(const KeyValuePair<KEYTYPE, VALUETYPE>& kvp);
    IndexT Add(const KEYTYPE& key, const VALUETYPE& value);
    VALUETYPE& Emplace(const KEYTYPE& key);
    void EndBulkAdd();
    void Merge(const Dictionary<KEYTYPE, VALUETYPE>& rhs);
    void Erase(const KEYTYPE& key);
    void EraseAtIndex(IndexT index);
    IndexT FindIndex(const KEYTYPE& key) const;
    bool Contains(const KEYTYPE& key) const;
    bool Contains(const KEYTYPE& key, IndexT& index) const;
    const KEYTYPE& KeyAtIndex(IndexT index) const;
    VALUETYPE& ValueAtIndex(IndexT index);
    const VALUETYPE& ValueAtIndex(IndexT index) const;
    KeyValuePair<KEYTYPE, VALUETYPE>& KeyValuePairAtIndex(IndexT index) const;
    Array<KEYTYPE> KeysAsArray() const;
    Array<VALUETYPE> ValuesAsArray() const;
    template<class RETURNTYPE> RETURNTYPE KeysAs() const;
    template<class RETURNTYPE> RETURNTYPE ValuesAs() const;
    KeyValuePair<KEYTYPE, VALUETYPE>* begin() const;
    KeyValuePair<KEYTYPE, VALUETYPE>* end() const;
    void clear();
    void emplace(KEYTYPE&&key, VALUETYPE&&value);
protected:
    void SortIfDirty() const;
    Array<KeyValuePair<KEYTYPE, VALUETYPE> > keyValuePairs;
    bool inBulkInsert;
};
template<class KEYTYPE, class VALUETYPE>
Dictionary<KEYTYPE, VALUETYPE>::Dictionary() :
    inBulkInsert(false)
{
}
template<class KEYTYPE, class VALUETYPE>
Dictionary<KEYTYPE, VALUETYPE>::Dictionary(const Dictionary<KEYTYPE, VALUETYPE>& rhs) :
    keyValuePairs(rhs.keyValuePairs),
    inBulkInsert(false)
{
    #if NEBULA_BOUNDSCHECKS
    n_assert(!rhs.inBulkInsert);
    #endif
}
template<class KEYTYPE, class VALUETYPE>
Dictionary<KEYTYPE, VALUETYPE>::Dictionary(Dictionary<KEYTYPE, VALUETYPE>&& rhs) noexcept :
    keyValuePairs(std::move(rhs.keyValuePairs)),
    inBulkInsert(false)
{
#if NEBULA_BOUNDSCHECKS
    n_assert(!rhs.inBulkInsert);
#endif
}
template<class KEYTYPE, class VALUETYPE>
inline void
Dictionary<KEYTYPE, VALUETYPE>::operator=(const Dictionary<KEYTYPE, VALUETYPE>& rhs)
{
    #if NEBULA_BOUNDSCHECKS
    n_assert(!this->inBulkInsert);
    n_assert(!rhs.inBulkInsert);
    #endif
    this->keyValuePairs = rhs.keyValuePairs;
}
template<class KEYTYPE, class VALUETYPE>
inline void
Dictionary<KEYTYPE, VALUETYPE>::operator=(Dictionary<KEYTYPE, VALUETYPE>&& rhs) noexcept
{
#if NEBULA_BOUNDSCHECKS
    n_assert(!this->inBulkInsert);
    n_assert(!rhs.inBulkInsert);
#endif
    this->keyValuePairs = std::move(rhs.keyValuePairs);
}
template<class KEYTYPE, class VALUETYPE>
inline void
Dictionary<KEYTYPE, VALUETYPE>::Clear()
{
    #if NEBULA_BOUNDSCHECKS
    n_assert(!this->inBulkInsert);
    #endif
    this->keyValuePairs.Clear();
}
template<class KEYTYPE, class VALUETYPE>
inline SizeT
Dictionary<KEYTYPE, VALUETYPE>::Size() const
{
    return this->keyValuePairs.Size();
}
template<class KEYTYPE, class VALUETYPE>
inline bool
Dictionary<KEYTYPE, VALUETYPE>::IsEmpty() const
{
    return (0 == this->keyValuePairs.Size());
}
template<class KEYTYPE, class VALUETYPE>
inline void
Dictionary<KEYTYPE, VALUETYPE>::Reserve(SizeT numElements)
{
    this->keyValuePairs.Reserve(numElements);
}
template<class KEYTYPE, class VALUETYPE>
inline void
Dictionary<KEYTYPE, VALUETYPE>::BeginBulkAdd()
{
    #if NEBULA_BOUNDSCHECKS
    n_assert(!this->inBulkInsert);
    #endif
    this->inBulkInsert = true;
}
template<class KEYTYPE, class VALUETYPE>
inline void
Dictionary<KEYTYPE, VALUETYPE>::EndBulkAdd()
{
    #if NEBULA_BOUNDSCHECKS
    n_assert(this->inBulkInsert);
    #endif
    this->keyValuePairs.Sort();
    this->inBulkInsert = false;
}
template<class KEYTYPE, class VALUETYPE>
inline void
Dictionary<KEYTYPE, VALUETYPE>::Merge(const Dictionary<KEYTYPE, VALUETYPE>& rhs)
{
    this->BeginBulkAdd();
    IndexT i;
    for (i = 0; i < rhs.keyValuePairs.Size(); i++)
    {
        this->keyValuePairs.Append(rhs.keyValuePairs[i]);
    }
    this->EndBulkAdd();
}
template<class KEYTYPE, class VALUETYPE>
inline IndexT
Dictionary<KEYTYPE, VALUETYPE>::Add(const KeyValuePair<KEYTYPE, VALUETYPE>& kvp)
{
    if (this->inBulkInsert)
    {
        this->keyValuePairs.Append(kvp);
        return this->keyValuePairs.Size() - 1;
    }
    else
    {
        return this->keyValuePairs.InsertSorted(kvp);
    }
}
template<class KEYTYPE, class VALUETYPE>
inline IndexT
Dictionary<KEYTYPE, VALUETYPE>::Add(const KEYTYPE& key, const VALUETYPE& value)
{
#if NEBULA_BOUNDSCHECKS
#endif
    KeyValuePair<KEYTYPE, VALUETYPE> kvp(key, value);
    if (this->inBulkInsert)
    {
        this->keyValuePairs.Append(kvp);
        return this->keyValuePairs.Size() - 1;
    }
    else
    {
        return this->keyValuePairs.InsertSorted(kvp);
    }
}
template<class KEYTYPE, class VALUETYPE>
inline VALUETYPE&
Dictionary<KEYTYPE, VALUETYPE>::Emplace(const KEYTYPE& key)
{
    IndexT i = this->FindIndex(key);
    if (i == InvalidIndex)
    {
        return this->ValueAtIndex(this->Add(key, VALUETYPE()));
    }
    else
    {
        return this->ValueAtIndex(i);
    }
}
template<class KEYTYPE, class VALUETYPE>
inline void
Dictionary<KEYTYPE, VALUETYPE>::Erase(const KEYTYPE& key)
{
    #if NEBULA_BOUNDSCHECKS
    n_assert(!this->inBulkInsert);
    #endif
    IndexT eraseIndex = this->keyValuePairs.template BinarySearchIndex<KEYTYPE>(key);
    #if NEBULA_BOUNDSCHECKS
    n_assert(InvalidIndex != eraseIndex);
    #endif
    this->keyValuePairs.EraseIndex(eraseIndex);
}
template<class KEYTYPE, class VALUETYPE>
inline void
Dictionary<KEYTYPE, VALUETYPE>::EraseAtIndex(IndexT index)
{
    #if NEBULA_BOUNDSCHECKS
    n_assert(!this->inBulkInsert);
    #endif
    this->keyValuePairs.EraseIndex(index);
}
template<class KEYTYPE, class VALUETYPE>
inline IndexT
Dictionary<KEYTYPE, VALUETYPE>::FindIndex(const KEYTYPE& key) const
{
    #if NEBULA_BOUNDSCHECKS
    n_assert(!this->inBulkInsert);
    #endif
    return this->keyValuePairs.template BinarySearchIndex<KEYTYPE>(key);
}
template<class KEYTYPE, class VALUETYPE>
inline bool
Dictionary<KEYTYPE, VALUETYPE>::Contains(const KEYTYPE& key) const
{
    #if NEBULA_BOUNDSCHECKS
    n_assert(!this->inBulkInsert);
    #endif
    return (InvalidIndex != this->keyValuePairs.template BinarySearchIndex<KEYTYPE>(key));
}
template<class KEYTYPE, class VALUETYPE>
inline bool
Dictionary<KEYTYPE, VALUETYPE>::Contains(const KEYTYPE& key, IndexT& index) const
{
#if NEBULA_BOUNDSCHECKS
    n_assert(!this->inBulkInsert);
#endif
    index = this->keyValuePairs.template BinarySearchIndex<KEYTYPE>(key);
    return (InvalidIndex != index);
}
template<class KEYTYPE, class VALUETYPE>
inline const KEYTYPE&
Dictionary<KEYTYPE, VALUETYPE>::KeyAtIndex(IndexT index) const
{
    #if NEBULA_BOUNDSCHECKS
    n_assert(!this->inBulkInsert);
    #endif
    return this->keyValuePairs[index].Key();
}
template<class KEYTYPE, class VALUETYPE>
inline VALUETYPE&
Dictionary<KEYTYPE, VALUETYPE>::ValueAtIndex(IndexT index)
{
    #if NEBULA_BOUNDSCHECKS
    n_assert(!this->inBulkInsert);
    #endif
    return this->keyValuePairs[index].Value();
}
template<class KEYTYPE, class VALUETYPE>
inline const VALUETYPE&
Dictionary<KEYTYPE, VALUETYPE>::ValueAtIndex(IndexT index) const
{
    #if NEBULA_BOUNDSCHECKS
    n_assert(!this->inBulkInsert);
    #endif
    return this->keyValuePairs[index].Value();
}
template<class KEYTYPE, class VALUETYPE>
inline KeyValuePair<KEYTYPE, VALUETYPE>&
Dictionary<KEYTYPE, VALUETYPE>::KeyValuePairAtIndex(IndexT index) const
{
    #if NEBULA_BOUNDSCHECKS
    n_assert(!this->inBulkInsert);
    #endif
    return this->keyValuePairs[index];
}
template<class KEYTYPE, class VALUETYPE>
inline VALUETYPE&
Dictionary<KEYTYPE, VALUETYPE>::operator[](const KEYTYPE& key)
{
    int keyValuePairIndex = this->FindIndex(key);
    #if NEBULA_BOUNDSCHECKS
    n_assert(InvalidIndex != keyValuePairIndex);
    #endif
    return this->keyValuePairs[keyValuePairIndex].Value();
}
template<class KEYTYPE, class VALUETYPE>
inline const VALUETYPE&
Dictionary<KEYTYPE, VALUETYPE>::operator[](const KEYTYPE& key) const
{
    int keyValuePairIndex = this->FindIndex(key);
    #if NEBULA_BOUNDSCHECKS
    n_assert(InvalidIndex != keyValuePairIndex);
    #endif
    return this->keyValuePairs[keyValuePairIndex].Value();
}
template<class KEYTYPE, class VALUETYPE>
template<class RETURNTYPE>
RETURNTYPE
Dictionary<KEYTYPE, VALUETYPE>::ValuesAs() const
{
    #if NEBULA_BOUNDSCHECKS
    n_assert(!this->inBulkInsert);
    #endif
    RETURNTYPE result(this->Size(),this->Size());
    IndexT i;
    for (i = 0; i < this->keyValuePairs.Size(); i++)
    {
        result.Append(this->keyValuePairs[i].Value());
    }
    return result;
}
template<class KEYTYPE, class VALUETYPE>
inline Array<VALUETYPE>
Dictionary<KEYTYPE, VALUETYPE>::ValuesAsArray() const
{
    return this->ValuesAs<Array<VALUETYPE> >();
}
template<class KEYTYPE, class VALUETYPE>
template<class RETURNTYPE>
inline RETURNTYPE
Dictionary<KEYTYPE, VALUETYPE>::KeysAs() const
{
    #if NEBULA_BOUNDSCHECKS
    n_assert(!this->inBulkInsert);
    #endif
    RETURNTYPE result(this->Size(),this->Size());
    IndexT i;
    for (i = 0; i < this->keyValuePairs.Size(); i++)
    {
        result.Append(this->keyValuePairs[i].Key());
    }
    return result;
}
template<class KEYTYPE, class VALUETYPE>
inline Array<KEYTYPE>
Dictionary<KEYTYPE, VALUETYPE>::KeysAsArray() const
{
    return this->KeysAs<Array<KEYTYPE> >();
}
template<class KEYTYPE, class VALUETYPE>
inline KeyValuePair<KEYTYPE, VALUETYPE>*
Dictionary<KEYTYPE, VALUETYPE>::begin() const
{
    return this->keyValuePairs.begin();
}
template<class KEYTYPE, class VALUETYPE>
inline KeyValuePair<KEYTYPE, VALUETYPE>*
Dictionary<KEYTYPE, VALUETYPE>::end() const
{
    return this->keyValuePairs.end();
}
template<class KEYTYPE, class VALUETYPE>
inline void
Dictionary<KEYTYPE, VALUETYPE>::clear()
{
    this->Clear();
}
template<class KEYTYPE, class VALUETYPE>
inline void
Dictionary<KEYTYPE, VALUETYPE>::emplace(KEYTYPE&&key, VALUETYPE&&value)
{
    this->Add(key, value);
}
}