#pragma once
#include "memdb/attributeregistry.h"
#include "game/componentid.h"
#include "game/entity.h"
#include "game/componentserialization.h"
#include "game/componentinspection.h"
#include "util/bitfield.h"
namespace Game
{
class World;
ComponentId GetComponentId(Util::StringAtom name);
template <typename COMPONENT>
ComponentId GetComponentId();
enum ComponentFlags : uint32_t
{
    COMPONENTFLAG_NONE = 0,
    COMPONENTFLAG_DECAY = 1 << 0
};
struct ComponentDecayBuffer
{
    uint32_t size = 0;
    uint32_t capacity = 0;
    void* buffer = nullptr;
};
template <typename COMPONENT_TYPE>
struct ComponentRegisterInfo
{
    using OnInitFunc = void (*)(Game::World*, Game::Entity, COMPONENT_TYPE*);
    bool decay = false;
    OnInitFunc OnInit = nullptr;
};
class ComponentInterface : public MemDb::Attribute
{
public:
    template <typename T>
    explicit ComponentInterface(Util::StringAtom name, T const& defaultValue, uint32_t flags)
        : Attribute(name, defaultValue, flags)
    {
        this->componentName = T::Traits::name;
        this->fullyQualifiedName = T::Traits::fully_qualified_name;
        this->numFields = T::Traits::num_fields;
        this->fieldNames = (const char**)T::Traits::field_names;
        this->fieldTypenames = (const char**)T::Traits::field_typenames;
        this->fieldByteOffsets = (const size_t*)T::Traits::field_byte_offsets;
    }
    using ComponentInitFunc = void (*)(Game::World*, Game::Entity, void*);
    ComponentInitFunc Init = nullptr;
    const char* GetName() const { return componentName; }
    const char* GetFullyQualifiedName() const { return fullyQualifiedName; }
    const char** GetFieldNames() const { return fieldNames; };
    const char** GetFieldTypenames() const { return fieldTypenames; };
    const size_t* GetFieldByteOffsets() const { return fieldByteOffsets; };
    size_t const GetNumFields() const { return numFields; };
private:
    const char* componentName = nullptr;
    const char* fullyQualifiedName = nullptr;
    const char** fieldNames = nullptr;
    const char** fieldTypenames = nullptr;
    const size_t* fieldByteOffsets = nullptr;
    size_t numFields = 0;
};
template <typename COMPONENT>
inline ComponentId
GetComponentId()
{
#if !PUBLIC_BUILD
    if (!MemDb::AttributeRegistry::IsRegistered<COMPONENT>())
    {
        n_error(
            "Компонент «%s» не зарегистрирован! Перед этим обязательно вызовите Game::RegisterComponent<T>() для всех типов компонентов. "
            "используя их.",
            COMPONENT::Traits::name
        );
    }
#endif
    return MemDb::GetAttributeId<COMPONENT>();
}
inline ComponentId
GetComponentId(Util::StringAtom name)
{
    return MemDb::AttributeRegistry::GetAttributeId(name);
}
}