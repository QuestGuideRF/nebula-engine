#pragma once
#include "coregraphics/config.h"
namespace Materials
{
struct MaterialVariant
{
    struct TextureHandleTuple;
    MaterialVariant()
        : mem(nullptr)
        , needsDeref(false)
        , size(0)
    {};
    MaterialVariant(const MaterialVariant& rhs)
        : mem(rhs.mem)
        , needsDeref(rhs.needsDeref)
        , size(rhs.size)
    {
    }
    void operator=(const MaterialVariant& rhs)
    {
        this->mem = rhs.mem;
        this->needsDeref = rhs.needsDeref;
        this->size = rhs.size;
    }
    MaterialVariant(MaterialVariant&& rhs)
        : mem(rhs.mem)
        , needsDeref(rhs.needsDeref)
        , size(rhs.size)
    {
        rhs.mem = nullptr;
        rhs.needsDeref = false;
        rhs.size = 0;
    }
    void operator=(MaterialVariant&& rhs)
    {
        this->mem = rhs.mem;
        this->needsDeref = rhs.needsDeref;
        this->size = rhs.size;
        rhs.mem = nullptr;
        rhs.needsDeref = false;
        rhs.size = 0;
    }
    MaterialVariant(std::nullptr_t)
        : mem(nullptr)
        , needsDeref(false)
        , size(0)
    {}
    bool needsDeref;
    void* mem;
    SizeT size;
    const void* Get() const
    {
        return this->needsDeref ? this->mem : reinterpret_cast<const void*>(&this->mem);
    }
    template <typename T> const T& ConstGet() const
    {
        return this->needsDeref ? *reinterpret_cast<T*>(this->mem) : reinterpret_cast<const T&>(this->mem);
    }
    template <typename T> T& Get()
    {
        return this->needsDeref ? *reinterpret_cast<T*>(this->mem) : reinterpret_cast<T&>(this->mem);
    }
    template <typename T> void Set(const T& data, void* mem)
    {
        this->size = sizeof(T);
        this->mem = mem;
        this->needsDeref = this->size > sizeof(void*);
        memcpy(this->needsDeref ? this->mem : reinterpret_cast<void*>(&this->mem), &data, this->size);
    }
    template <typename T> void Set(void* mem)
    {
        this->size = sizeof(T);
        this->mem = mem;
        this->needsDeref = true;
    }
};
}