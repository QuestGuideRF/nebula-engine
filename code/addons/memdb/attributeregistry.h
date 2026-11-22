#pragma once
#include "attribute.h"
#include "util/stringatom.h"
namespace MemDb
{
class AttributeRegistry
{
public:
    template<typename TYPE>
    static AttributeId Register(Util::StringAtom name, TYPE defaultValue, uint32_t flags = 0);
    template <typename TYPE>
    static AttributeId Register(Attribute* attribute);
    template <typename TYPE>
    static bool IsRegistered();
    static AttributeId Register(Util::StringAtom name, SizeT typeSize, void const* defaultValue, uint32_t flags = 0);
    static AttributeId GetAttributeId(Util::StringAtom name);
    static Attribute* GetAttribute(AttributeId descriptor);
    static SizeT TypeSize(AttributeId descriptor);
    static uint32_t Flags(AttributeId descriptor);
    static void const* const DefaultValue(AttributeId descriptor);
    static Util::FixedArray<Attribute*> const& GetAllAttributes();
private:
    static AttributeRegistry* Instance();
    static void Destroy();
    AttributeRegistry();
    ~AttributeRegistry();
    static AttributeRegistry* Singleton;
    Util::FixedArray<Attribute*> componentDescriptions;
    Util::Dictionary<Util::StringAtom, AttributeId> registry;
};
uint16_t GenerateNewAttributeId();
template <typename ATTRIBUTE>
AttributeId
GetAttributeId()
{
    static_assert(std::is_trivially_destructible<ATTRIBUTE>());
    static const uint16_t id = MemDb::GenerateNewAttributeId();
    return AttributeId(id);
}
template<typename TYPE>
inline bool
AttributeRegistry::IsRegistered()
{
    auto* reg = Instance();
    AttributeId id = MemDb::GetAttributeId<TYPE>();
    if (id < reg->componentDescriptions.Size())
    {
        return reg->componentDescriptions[id.id] != nullptr;
    }
    return false;
}
template<typename TYPE>
inline AttributeId
AttributeRegistry::Register(Util::StringAtom name, TYPE defaultValue, uint32_t flags)
{
    static_assert(!std::is_polymorphic<TYPE>(), "TYPE не должен быть полиморфным.");
    static_assert(!std::is_abstract<TYPE>(), "ТИП не должен быть абстрактным.");
    auto* reg = Instance();
    if (!reg->registry.Contains(name))
    {
        Attribute* desc = new Attribute(name, defaultValue, flags);
        AttributeId descriptor = MemDb::GetAttributeId<TYPE>();
        if (descriptor.id >= reg->componentDescriptions.Size())
        {
            SizeT prevSize = reg->componentDescriptions.Size();
            reg->componentDescriptions.Resize(descriptor.id + 1);
            SizeT num = reg->componentDescriptions.Size() - prevSize;
            reg->componentDescriptions.Fill(prevSize, num, nullptr);
        }
        reg->componentDescriptions[descriptor.id] = desc;
        reg->registry.Add(name, descriptor);
        return descriptor;
    }
    else
    {
        n_error("Попытка зарегистрировать компонент с именем %s: невозможно зарегистрировать два компонента с одинаковым именем!", name.Value());
    }
    return AttributeId::Invalid();
}
template <typename TYPE>
inline AttributeId
AttributeRegistry::Register(Attribute* desc)
{
    static_assert(!std::is_polymorphic<TYPE>(), "TYPE не должен быть полиморфным.");
    static_assert(!std::is_abstract<TYPE>(), "ТИП не должен быть абстрактным.");
    auto* reg = Instance();
    if (!reg->registry.Contains(desc->name))
    {
        AttributeId attrId = MemDb::GetAttributeId<TYPE>();
        if (attrId.id >= reg->componentDescriptions.Size())
        {
            SizeT prevSize = reg->componentDescriptions.Size();
            reg->componentDescriptions.Resize(attrId.id + 1);
            SizeT num = reg->componentDescriptions.Size() - prevSize;
            reg->componentDescriptions.Fill(
                prevSize, num, nullptr
            );
        }
        reg->componentDescriptions[attrId.id] = desc;
        reg->registry.Add(desc->name, attrId);
        return attrId;
    }
    else
    {
        n_error("Попытка зарегистрировать компонент с именем %s: невозможно зарегистрировать два компонента с одинаковым именем!", desc->name.Value());
    }
    return AttributeId::Invalid();
}
inline AttributeId
AttributeRegistry::Register(Util::StringAtom name, SizeT typeSize, void const* defaultValue, uint32_t flags)
{
    auto* reg = Instance();
    if (!reg->registry.Contains(name))
    {
        Attribute* desc = new Attribute(name, typeSize, defaultValue, flags);
        AttributeId descriptor = MemDb::GenerateNewAttributeId();
        if (descriptor.id >= reg->componentDescriptions.Size())
        {
            reg->componentDescriptions.Resize(descriptor.id + 1);
        }
        reg->componentDescriptions[descriptor.id] = desc;
        reg->registry.Add(name, descriptor);
        return descriptor;
    }
    else
    {
        n_error("Попытка зарегистрировать компонент с именем %s: невозможно зарегистрировать два компонента с одинаковым именем!", name.Value());
    }
    return AttributeId::Invalid();
}
inline AttributeId
AttributeRegistry::GetAttributeId(Util::StringAtom name)
{
    auto* reg = Instance();
    IndexT index = reg->registry.FindIndex(name);
    if (index != InvalidIndex)
    {
        return reg->registry.ValueAtIndex(index);
    }
    return AttributeId::Invalid();
}
inline Attribute*
AttributeRegistry::GetAttribute(AttributeId descriptor)
{
    auto* reg = Instance();
    if (descriptor.id >= 0 && descriptor.id < reg->componentDescriptions.Size())
    {
        n_assert2(
            reg->componentDescriptions[descriptor.id] != nullptr, "Пытаюсь получить описание незарегистрированного атрибута!"
        );
        return reg->componentDescriptions[descriptor.id];
    }
    return nullptr;
}
inline SizeT
AttributeRegistry::TypeSize(AttributeId descriptor)
{
    auto* reg = Instance();
    n_assert(descriptor.id >= 0 && descriptor.id < reg->componentDescriptions.Size());
    return reg->componentDescriptions[descriptor.id]->typeSize;
}
inline uint32_t
AttributeRegistry::Flags(AttributeId descriptor)
{
    auto* reg = Instance();
    n_assert(descriptor.id >= 0 && descriptor.id < reg->componentDescriptions.Size());
    return reg->componentDescriptions[descriptor.id]->externalFlags;
}
inline void const* const
AttributeRegistry::DefaultValue(AttributeId descriptor)
{
    auto* reg = Instance();
    n_assert(descriptor.id >= 0 && descriptor.id < reg->componentDescriptions.Size());
    return reg->componentDescriptions[descriptor.id]->defVal;
}
inline Util::FixedArray<Attribute*> const&
AttributeRegistry::GetAllAttributes()
{
    auto* reg = Instance();
    return reg->componentDescriptions;
}
}