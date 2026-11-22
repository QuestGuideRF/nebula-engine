#pragma once
#include "core/types.h"
namespace Util
{
template<class TYPE> class FixedTable
{
public:
    FixedTable();
    FixedTable(SizeT w, SizeT h);
    FixedTable(SizeT w, SizeT h, const TYPE& val);
    FixedTable(const FixedTable<TYPE>& rhs);
    FixedTable(FixedTable<TYPE>&& rhs) noexcept;
    ~FixedTable();
    void operator=(const FixedTable<TYPE>& rhs);
    void operator=(FixedTable<TYPE>&& rhs) noexcept;
    bool operator==(const FixedTable<TYPE>& rhs) const;
    bool operator!=(const FixedTable<TYPE>& rhs) const;
    void SetSize(SizeT w, SizeT h);
    SizeT Width() const;
    SizeT Height() const;
    void Clear(const TYPE& val);
    void Set(IndexT x, IndexT y, const TYPE& val);
    TYPE& At(IndexT x, IndexT y) const;
private:
    void Delete();
    void Allocate(SizeT w, SizeT h);
    void Copy(const FixedTable<TYPE>& src);
    SizeT width;
    SizeT height;
    TYPE* elements;
};
template<class TYPE>
FixedTable<TYPE>::FixedTable() :
    width(0),
    height(0),
    elements(0)
{
}
template<class TYPE>
void
FixedTable<TYPE>::Delete()
{
    if (this->elements)
    {
        delete[] this->elements;
        this->elements = 0;
    }
    this->width = 0;
    this->height = 0;
}
template<class TYPE>
void
FixedTable<TYPE>::Allocate(SizeT w, SizeT h)
{
    #if NEBULA_BOUNDSCHECKS
    n_assert(0 == this->elements);
    #endif
    if ((w > 0) && (h > 0))
    {
        this->elements = new TYPE[w * h];
    }
    this->width = w;
    this->height = h;
}
template<class TYPE>
void
FixedTable<TYPE>::Copy(const FixedTable<TYPE>& rhs)
{
    if (this != &rhs)
    {
        this->Allocate(rhs.width, rhs.height);
        IndexT y;
        for (y = 0; y < this->height; y++)
        {
            IndexT x;
            for (x = 0; x < this->width; x++)
            {
                int flatIndex = y * this->width + x;
                this->elements[flatIndex] = rhs.elements[flatIndex];
            }
        }
    }
}
template<class TYPE>
void
FixedTable<TYPE>::Clear(const TYPE& val)
{
    IndexT y;
    for (y = 0; y < this->height; y++)
    {
        IndexT x;
        for (x = 0; x < this->width; x++)
        {
            int flatIndex = y * this->width + x;
            this->elements[flatIndex] = val;
        }
    }
}
template<class TYPE>
FixedTable<TYPE>::FixedTable(SizeT w, SizeT h) :
    width(0),
    height(0),
    elements(0)
{
    this->Allocate(w, h);
}
template<class TYPE>
FixedTable<TYPE>::FixedTable(SizeT w, SizeT h, const TYPE& val) :
    width(0),
    height(0),
    elements(0)
{
    this->Allocate(w, h);
    this->Clear(val);
}
template<class TYPE>
FixedTable<TYPE>::FixedTable(const FixedTable<TYPE>& rhs)
    : width(0)
    , height(0)
    , elements(0)
{
    this->Copy(rhs);
}
template<class TYPE>
FixedTable<TYPE>::FixedTable(FixedTable<TYPE>&& rhs) noexcept
    : width(rhs.width)
    , height(rhs.height)
    , elements(rhs.height)
{
    rhs.elements = nullptr;
    rhs.width = 0;
    rhs.height = 0;
}
template<class TYPE>
FixedTable<TYPE>::~FixedTable()
{
    this->Delete();
}
template<class TYPE>
void
FixedTable<TYPE>::operator=(const FixedTable<TYPE>& rhs)
{
    this->Delete();
    this->Copy(rhs);
}
template<class TYPE>
void
FixedTable<TYPE>::operator=(FixedTable<TYPE>&& rhs) noexcept
{
    if (*this != rhs)
    {
        this->Delete();
        this->elements = rhs.elements;
        this->height = rhs.height;
        this->width = rhs.width;
        rhs.elements = nullptr;
        rhs.width = 0;
        rhs.height = 0;
    }
}
template<class TYPE>
bool
FixedTable<TYPE>::operator==(const FixedTable<TYPE>& rhs) const
{
    if ((this->width == rhs.width) && (this->height == rhs.height))
    {
        IndexT y;
        for (y = 0; y < this->height; y++)
        {
            IndexT x;
            for (x = 0; x < this->width; x++)
            {
                int flatIndex = y * this->width + x;
                if (this->elements[flatIndex] != rhs.elements[flatIndex])
                {
                    return false;
                }
            }
        }
        return true;
    }
    else
    {
        return false;
    }
}
template<class TYPE>
bool
FixedTable<TYPE>::operator!=(const FixedTable<TYPE>& rhs) const
{
    return !(*this == rhs);
}
template<class TYPE>
void
FixedTable<TYPE>::SetSize(SizeT w, SizeT h)
{
    this->Delete();
    this->Allocate(w, h);
}
template<class TYPE>
SizeT
FixedTable<TYPE>::Width() const
{
    return this->width;
}
template<class TYPE>
SizeT
FixedTable<TYPE>::Height() const
{
    return this->height;
}
template<class TYPE>
void
FixedTable<TYPE>::Set(IndexT x, IndexT y, const TYPE& val)
{
    #if NEBULA_BOUNDSCHECKS
    n_assert((x >= 0) && (x < this->width));
    n_assert(y < this->height);
    n_assert(0 != this->elements);
    #endif
    int flatIndex = y * this->width + x;
    this->elements[flatIndex] = val;
}
template<class TYPE>
TYPE&
FixedTable<TYPE>::At(IndexT x, IndexT y) const
{
    #if NEBULA_BOUNDSCHECKS
    n_assert((x >= 0) && (x < this->width));
    n_assert(y < this->height);
    n_assert(0 != this->elements);
    #endif
    int flatIndex = y * this->width + x;
    return this->elements[flatIndex];
}
}