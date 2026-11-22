#pragma once
#include "util/fixedarray.h"
#include "util/array.h"
#include "util/keyvaluepair.h"
#include "math/scalar.h"
#include <type_traits>
namespace Util
{
template<class KEYTYPE, class VALUETYPE, int TABLE_SIZE = 128, int STACK_SIZE = 1> class HashTable
{
public:
    HashTable();
    HashTable(const HashTable<KEYTYPE, VALUETYPE, TABLE_SIZE, STACK_SIZE>& rhs);
    HashTable(HashTable<KEYTYPE, VALUETYPE, TABLE_SIZE, STACK_SIZE>&& rhs);
    void operator=(const HashTable<KEYTYPE, VALUETYPE, TABLE_SIZE, STACK_SIZE>& rhs);
    void operator=(HashTable<KEYTYPE, VALUETYPE, TABLE_SIZE, STACK_SIZE>&& rhs);
    VALUETYPE& operator[](const KEYTYPE& key) const;
    SizeT Size() const;
    SizeT Capacity() const;
    void Clear();
    void Reset();
    bool IsEmpty() const;
    void BeginBulkAdd();
    const bool IsBulkAdd() const;
    IndexT Add(const KeyValuePair<KEYTYPE, VALUETYPE>& kvp);
    IndexT Add(const KEYTYPE& key, const VALUETYPE& value);
    VALUETYPE& Emplace(const KEYTYPE& key);
    void EndBulkAdd();
    void Merge(const HashTable<KEYTYPE, VALUETYPE, TABLE_SIZE, STACK_SIZE>& rhs);
    void Erase(const KEYTYPE& key);
    void EraseIndex(const KEYTYPE& key, IndexT i);
    bool Contains(const KEYTYPE& key) const;
    IndexT FindIndex(const KEYTYPE& key) const;
    VALUETYPE& ValueAtIndex(const KEYTYPE& key, IndexT i) const;
    StackArray<KeyValuePair<KEYTYPE, VALUETYPE>, STACK_SIZE> Content() const;
    StackArray<KEYTYPE, STACK_SIZE> KeysAsArray() const;
    StackArray<VALUETYPE, STACK_SIZE> ValuesAsArray() const;
    class Iterator
    {
    public:
        Iterator& operator++(int);
        const bool operator==(const Iterator& rhs) const;
        const bool operator!=(const Iterator& rhs) const;
        VALUETYPE* val;
        KEYTYPE const* key;
    private:
        friend class HashTable<KEYTYPE, VALUETYPE, TABLE_SIZE, STACK_SIZE>;
        FixedArray<StackArray<KeyValuePair<KEYTYPE, VALUETYPE>, STACK_SIZE> >* arr;
        uint32_t hashIndex;
        IndexT bucketIndex;
    };
    Iterator Begin();
    Iterator End();
private:
    FixedArray<StackArray<KeyValuePair<KEYTYPE, VALUETYPE>, STACK_SIZE> > hashArray;
    int size;
    FixedArray<bool> bulkDirty;
    bool inBulkAdd;
    template <typename HASHKEY> const uint32_t GetHashCode(const typename std::enable_if<std::is_integral<HASHKEY>::value, HASHKEY>::type& key) const { return uint32_t(key % this->hashArray.Size()); };
    template <typename HASHKEY> const uint32_t GetHashCode(const HASHKEY& key) const { return key.HashCode() % this->hashArray.Size(); };
    template <typename HASHKEY> const uint32_t GetHashCode(const typename std::enable_if<std::is_pointer<HASHKEY>::value, HASHKEY>::type& key) const { return key->HashCode() % this->hashArray.Size(); }
};
template<class KEYTYPE, class VALUETYPE, int TABLE_SIZE, int STACK_SIZE>
StackArray<KEYTYPE, STACK_SIZE>
HashTable<KEYTYPE, VALUETYPE, TABLE_SIZE, STACK_SIZE>::KeysAsArray() const
{
    Util::StackArray<KEYTYPE, STACK_SIZE> keys;
    for (int i = 0; i < this->hashArray.Size(); i++)
    {
        for (int j = 0; j < this->hashArray[i].Size();j++)
        {
            keys.Append(this->hashArray[i][j].Key());
        }
    }
    return keys;
}
template<class KEYTYPE, class VALUETYPE, int TABLE_SIZE, int STACK_SIZE>
StackArray<VALUETYPE, STACK_SIZE>
HashTable<KEYTYPE, VALUETYPE, TABLE_SIZE, STACK_SIZE>::ValuesAsArray() const
{
    Util::StackArray<VALUETYPE, STACK_SIZE> vals;
    for (int i = 0; i < this->hashArray.Size(); i++)
    {
        for (int j = 0; j < this->hashArray[i].Size(); j++)
        {
            vals.Append(this->hashArray[i][j].Value());
        }
    }
    return vals;
}
template<class KEYTYPE, class VALUETYPE, int TABLE_SIZE, int STACK_SIZE>
typename HashTable<KEYTYPE, VALUETYPE, TABLE_SIZE, STACK_SIZE>::Iterator
HashTable<KEYTYPE, VALUETYPE, TABLE_SIZE, STACK_SIZE>::Begin()
{
    Iterator ret;
    ret.hashIndex = this->hashArray.Size();
    ret.bucketIndex = 0;
    ret.val = nullptr;
    ret.key = nullptr;
    ret.arr = &this->hashArray;
    IndexT i;
    for (i = 0; i < this->hashArray.Size(); i++)
    {
        if (this->hashArray[i].Size() != 0)
        {
            ret.hashIndex = i;
            ret.bucketIndex = 0;
            ret.val = &this->hashArray[i].Front().Value();
            ret.key = &this->hashArray[i].Front().Key();
            break;
        }
    }
    return ret;
}
template<class KEYTYPE, class VALUETYPE, int TABLE_SIZE, int STACK_SIZE>
typename HashTable<KEYTYPE, VALUETYPE, TABLE_SIZE, STACK_SIZE>::Iterator
HashTable<KEYTYPE, VALUETYPE, TABLE_SIZE, STACK_SIZE>::End()
{
    Iterator ret;
    ret.hashIndex = this->hashArray.Size();
    ret.bucketIndex = 0;
    ret.val = nullptr;
    ret.key = nullptr;
    ret.arr = &this->hashArray;
    return ret;
}
template<class KEYTYPE, class VALUETYPE, int TABLE_SIZE, int STACK_SIZE>
HashTable<KEYTYPE, VALUETYPE, TABLE_SIZE, STACK_SIZE>::HashTable() :
    hashArray(TABLE_SIZE),
    bulkDirty(TABLE_SIZE),
    inBulkAdd(false),
    size(0)
{
}
template<class KEYTYPE, class VALUETYPE, int TABLE_SIZE, int STACK_SIZE>
HashTable<KEYTYPE, VALUETYPE, TABLE_SIZE, STACK_SIZE>::HashTable(const HashTable<KEYTYPE, VALUETYPE, TABLE_SIZE, STACK_SIZE>& rhs) :
    hashArray(rhs.hashArray),
    bulkDirty(rhs.bulkDirty),
    inBulkAdd(rhs.inBulkAdd),
    size(rhs.size)
{
}
template<class KEYTYPE, class VALUETYPE, int TABLE_SIZE, int STACK_SIZE>
HashTable<KEYTYPE, VALUETYPE, TABLE_SIZE, STACK_SIZE>::HashTable(HashTable<KEYTYPE, VALUETYPE, TABLE_SIZE, STACK_SIZE>&& rhs) :
    hashArray(std::move(rhs.hashArray)),
    bulkDirty(rhs.bulkDirty),
    inBulkAdd(rhs.inBulkAdd),
    size(rhs.size)
{
    rhs.size = 0;
}
template<class KEYTYPE, class VALUETYPE, int TABLE_SIZE, int STACK_SIZE>
void
HashTable<KEYTYPE, VALUETYPE, TABLE_SIZE, STACK_SIZE>::operator=(const HashTable<KEYTYPE, VALUETYPE, TABLE_SIZE, STACK_SIZE>& rhs)
{
    if (this != &rhs)
    {
        this->hashArray = rhs.hashArray;
        this->bulkDirty = rhs.bulkDirty;
        this->size = rhs.size;
    }
}
template<class KEYTYPE, class VALUETYPE, int TABLE_SIZE, int STACK_SIZE>
void
HashTable<KEYTYPE, VALUETYPE, TABLE_SIZE, STACK_SIZE>::operator=(HashTable<KEYTYPE, VALUETYPE, TABLE_SIZE, STACK_SIZE>&& rhs)
{
    if (this != &rhs)
    {
        this->hashArray = std::move(rhs.hashArray);
        this->bulkDirty = rhs.bulkDirty;
        this->size = rhs.size;
        rhs.size = 0;
    }
}
template<class KEYTYPE, class VALUETYPE, int TABLE_SIZE, int STACK_SIZE>
VALUETYPE&
HashTable<KEYTYPE, VALUETYPE, TABLE_SIZE, STACK_SIZE>::operator[](const KEYTYPE& key) const
{
    n_assert(!this->inBulkAdd);
    uint32_t hashIndex = GetHashCode<KEYTYPE>(key);
    const StackArray<KeyValuePair<KEYTYPE, VALUETYPE>, STACK_SIZE>& hashElements = this->hashArray[hashIndex];
    int numHashElements = hashElements.Size();
    #if NEBULA_BOUNDSCHECKS
    n_assert(0 != numHashElements);
    #endif
    if (1 == numHashElements)
    {
        return hashElements[0].Value();
    }
    else
    {
    IndexT hashElementIndex = hashElements.template BinarySearchIndex<KEYTYPE>(key);
        #if NEBULA_BOUNDSCHECKS
        n_assert(InvalidIndex != hashElementIndex);
        #endif
        return hashElements[hashElementIndex].Value();
    }
}
template<class KEYTYPE, class VALUETYPE, int TABLE_SIZE, int STACK_SIZE>
SizeT
HashTable<KEYTYPE, VALUETYPE, TABLE_SIZE, STACK_SIZE>::Size() const
{
    return this->size;
}
template<class KEYTYPE, class VALUETYPE, int TABLE_SIZE, int STACK_SIZE>
SizeT
HashTable<KEYTYPE, VALUETYPE, TABLE_SIZE, STACK_SIZE>::Capacity() const
{
    return this->hashArray.Size();
}
template<class KEYTYPE, class VALUETYPE, int TABLE_SIZE, int STACK_SIZE>
void
HashTable<KEYTYPE, VALUETYPE, TABLE_SIZE, STACK_SIZE>::Clear()
{
    IndexT i;
    SizeT num = this->hashArray.Size();
    for (i = 0; i < num; i++)
    {
        this->hashArray[i].Clear();
    }
    this->size = 0;
}
template<class KEYTYPE, class VALUETYPE, int TABLE_SIZE, int STACK_SIZE>
void
HashTable<KEYTYPE, VALUETYPE, TABLE_SIZE, STACK_SIZE>::Reset()
{
    IndexT i;
    SizeT num = this->hashArray.Size();
    for (i = 0; i < num; i++)
    {
        this->hashArray[i].Reset();
    }
    this->size = 0;
}
template<class KEYTYPE, class VALUETYPE, int TABLE_SIZE, int STACK_SIZE>
bool
HashTable<KEYTYPE, VALUETYPE, TABLE_SIZE, STACK_SIZE>::IsEmpty() const
{
    return (0 == this->size);
}
template<class KEYTYPE, class VALUETYPE, int TABLE_SIZE, int STACK_SIZE>
void
HashTable<KEYTYPE, VALUETYPE, TABLE_SIZE, STACK_SIZE>::BeginBulkAdd()
{
    n_assert(!this->inBulkAdd);
    this->inBulkAdd = true;
    this->bulkDirty.Fill(false);
}
template<class KEYTYPE, class VALUETYPE, int TABLE_SIZE, int STACK_SIZE>
inline const bool
HashTable<KEYTYPE, VALUETYPE, TABLE_SIZE, STACK_SIZE>::IsBulkAdd() const
{
    return this->inBulkAdd;
}
template<class KEYTYPE, class VALUETYPE, int TABLE_SIZE, int STACK_SIZE>
IndexT
HashTable<KEYTYPE, VALUETYPE, TABLE_SIZE, STACK_SIZE>::Add(const KeyValuePair<KEYTYPE, VALUETYPE>& kvp)
{
#if NEBULA_BOUNDSCHECKS
    n_assert(!this->Contains(kvp.Key()));
#endif
    uint32_t hashIndex = GetHashCode<KEYTYPE>(kvp.Key());
    IndexT ret;
    if (this->inBulkAdd)
    {
        ret = this->hashArray[hashIndex].Size();
        this->hashArray[hashIndex].Append(kvp);
        this->bulkDirty[hashIndex] = true;
    }
    else
        ret = this->hashArray[hashIndex].InsertSorted(kvp);
    this->size++;
    return ret;
}
template<class KEYTYPE, class VALUETYPE, int TABLE_SIZE, int STACK_SIZE>
IndexT
HashTable<KEYTYPE, VALUETYPE, TABLE_SIZE, STACK_SIZE>::Add(const KEYTYPE& key, const VALUETYPE& value)
{
    KeyValuePair<KEYTYPE, VALUETYPE> kvp(key, value);
    return this->Add(kvp);
}
template<class KEYTYPE, class VALUETYPE, int TABLE_SIZE, int STACK_SIZE>
VALUETYPE&
HashTable<KEYTYPE, VALUETYPE, TABLE_SIZE, STACK_SIZE>::Emplace(const KEYTYPE& key)
{
    uint32_t hashIndex = GetHashCode<KEYTYPE>(key);
    IndexT elementIndex = InvalidIndex;
    StackArray<KeyValuePair<KEYTYPE, VALUETYPE>, STACK_SIZE>& hashElements = this->hashArray[hashIndex];
    if (hashElements.Size() == 0)
    {
        elementIndex = this->Add(key, VALUETYPE());
    }
    else
    {
        if (this->inBulkAdd)
            elementIndex = hashElements.template FindIndex<KEYTYPE>(key);
        else
            elementIndex = hashElements.template BinarySearchIndex<KEYTYPE>(key);
        if (elementIndex == InvalidIndex)
        {
            elementIndex = this->Add(key, VALUETYPE());
        }
    }
    return hashElements[elementIndex].Value();
}
template<class KEYTYPE, class VALUETYPE, int TABLE_SIZE, int STACK_SIZE>
void
HashTable<KEYTYPE, VALUETYPE, TABLE_SIZE, STACK_SIZE>::EndBulkAdd()
{
    n_assert(this->inBulkAdd);
    IndexT i;
    for (i = 0; i < this->bulkDirty.Size(); i++)
    {
        if (this->bulkDirty[i])
        {
            this->hashArray[i].Sort();
            this->bulkDirty[i] = false;
        }
    }
    this->inBulkAdd = false;
}
template<class KEYTYPE, class VALUETYPE, int TABLE_SIZE, int STACK_SIZE>
void
HashTable<KEYTYPE, VALUETYPE, TABLE_SIZE, STACK_SIZE>::Merge(const HashTable<KEYTYPE, VALUETYPE, TABLE_SIZE, STACK_SIZE>& rhs)
{
    n_assert(this->hashArray.Size() == rhs.hashArray.Size());
    IndexT i;
    for (i = 0; i < rhs.hashArray.Size(); i++)
    {
        IndexT j;
        for (j = 0; j < rhs.hashArray[i].Size(); j++)
            this->Add(rhs.hashArray[i][j]);
    }
}
template<class KEYTYPE, class VALUETYPE, int TABLE_SIZE, int STACK_SIZE>
void
HashTable<KEYTYPE, VALUETYPE, TABLE_SIZE, STACK_SIZE>::Erase(const KEYTYPE& key)
{
    #if NEBULA_BOUNDSCHECKS
    n_assert(!this->inBulkAdd);
    n_assert(this->size > 0);
    #endif
    uint32_t hashIndex = GetHashCode<KEYTYPE>(key);
    StackArray<KeyValuePair<KEYTYPE, VALUETYPE>, STACK_SIZE>& hashElements = this->hashArray[hashIndex];
    IndexT hashElementIndex = hashElements.template BinarySearchIndex<KEYTYPE>(key);
    #if NEBULA_BOUNDSCHECKS
    n_assert(InvalidIndex != hashElementIndex);
    #endif
    hashElements.EraseIndex(hashElementIndex);
    this->size--;
}
template<class KEYTYPE, class VALUETYPE, int TABLE_SIZE, int STACK_SIZE>
void
HashTable<KEYTYPE, VALUETYPE, TABLE_SIZE, STACK_SIZE>::EraseIndex(const KEYTYPE& key, const IndexT index)
{
#if NEBULA_BOUNDSCHECKS
    n_assert(this->size > 0);
#endif
    uint32_t hashIndex = GetHashCode<KEYTYPE>(key);
    StackArray<KeyValuePair<KEYTYPE, VALUETYPE>, STACK_SIZE>& hashElements = this->hashArray[hashIndex];
#if NEBULA_BOUNDSCHECKS
    n_assert(InvalidIndex != index);
#endif
    hashElements.EraseIndex(index);
    this->size--;
}
template<class KEYTYPE, class VALUETYPE, int TABLE_SIZE, int STACK_SIZE>
bool
HashTable<KEYTYPE, VALUETYPE, TABLE_SIZE, STACK_SIZE>::Contains(const KEYTYPE& key) const
{
    if (this->size > 0)
    {
        uint32_t hashIndex = GetHashCode<KEYTYPE>(key);
        StackArray<KeyValuePair<KEYTYPE, VALUETYPE>, STACK_SIZE>& hashElements = this->hashArray[hashIndex];
        if (hashElements.Size() == 0) return false;
        else
        {
            IndexT hashElementIndex;
            if (this->inBulkAdd)
                hashElementIndex = hashElements.template FindIndex<KEYTYPE>(key);
            else
                hashElementIndex = hashElements.template BinarySearchIndex<KEYTYPE>(key);
            return (InvalidIndex != hashElementIndex);
        }
    }
    else
    {
        return false;
    }
}
template<class KEYTYPE, class VALUETYPE, int TABLE_SIZE, int STACK_SIZE>
IndexT
HashTable<KEYTYPE, VALUETYPE, TABLE_SIZE, STACK_SIZE>::FindIndex(const KEYTYPE& key) const
{
    uint32_t hashIndex = GetHashCode<KEYTYPE>(key);
    StackArray<KeyValuePair<KEYTYPE, VALUETYPE>, STACK_SIZE>& hashElements = this->hashArray[hashIndex];
    IndexT hashElementIndex;
    if (this->inBulkAdd)
        hashElementIndex = hashElements.template FindIndex<KEYTYPE>(key);
    else
        hashElementIndex = hashElements.template BinarySearchIndex<KEYTYPE>(key);
    return hashElementIndex;
}
template<class KEYTYPE, class VALUETYPE, int TABLE_SIZE, int STACK_SIZE>
VALUETYPE&
HashTable<KEYTYPE, VALUETYPE, TABLE_SIZE, STACK_SIZE>::ValueAtIndex(const KEYTYPE& key, IndexT i) const
{
    uint32_t hashIndex = GetHashCode<KEYTYPE>(key);
    StackArray<KeyValuePair<KEYTYPE, VALUETYPE>, STACK_SIZE>& hashElements = this->hashArray[hashIndex];
    return hashElements[i].Value();
}
template<class KEYTYPE, class VALUETYPE, int TABLE_SIZE, int STACK_SIZE>
StackArray<KeyValuePair<KEYTYPE, VALUETYPE>, STACK_SIZE>
HashTable<KEYTYPE, VALUETYPE, TABLE_SIZE, STACK_SIZE>::Content() const
{
    StackArray<KeyValuePair<KEYTYPE, VALUETYPE>, STACK_SIZE> result;
    int i;
    int num = this->hashArray.Size();
    for (i = 0; i < num; i++)
    {
        if (this->hashArray[i].Size() > 0)
        {
            result.AppendArray(this->hashArray[i]);
        }
    }
    return result;
}
template<class KEYTYPE, class VALUETYPE, int TABLE_SIZE, int STACK_SIZE>
typename HashTable<KEYTYPE, VALUETYPE, TABLE_SIZE, STACK_SIZE>::Iterator&
HashTable<KEYTYPE, VALUETYPE, TABLE_SIZE, STACK_SIZE>::Iterator::operator++(int)
{
    const FixedArray<StackArray<KeyValuePair<KEYTYPE, VALUETYPE>, STACK_SIZE> >& arr = *this->arr;
    this->bucketIndex++;
    if (arr[this->hashIndex].Size() > this->bucketIndex)
    {
        this->val = &arr[this->hashIndex][this->bucketIndex].Value();
        this->key = &arr[this->hashIndex][this->bucketIndex].Key();
    }
    else
    {
        IndexT i;
        for (i = this->hashIndex + 1; i < arr.Size(); i++)
        {
            if (arr[i].Size() > 0)
            {
                this->hashIndex = i;
                this->bucketIndex = 0;
                this->val = &arr[i][this->bucketIndex].Value();
                this->key = &arr[i][this->bucketIndex].Key();
                break;
            }
        }
        if (i == arr.Size())
        {
            this->hashIndex = arr.Size();
            this->bucketIndex = 0;
            this->val = nullptr;
            this->key = nullptr;
        }
    }
    return *this;
}
template<class KEYTYPE, class VALUETYPE, int TABLE_SIZE, int STACK_SIZE>
const bool
HashTable<KEYTYPE, VALUETYPE, TABLE_SIZE, STACK_SIZE>::Iterator::operator==(const Iterator& rhs) const
{
    return this->key == rhs.key;
}
template<class KEYTYPE, class VALUETYPE, int TABLE_SIZE, int STACK_SIZE>
const bool
HashTable<KEYTYPE, VALUETYPE, TABLE_SIZE, STACK_SIZE>::Iterator::operator!=(const Iterator& rhs) const
{
    return this->key != rhs.key;
}
}