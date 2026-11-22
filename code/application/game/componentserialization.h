#pragma once
#include "io/memorystream.h"
#include "io/binarywriter.h"
#include "io/binaryreader.h"
#include "io/jsonreader.h"
#include "io/jsonwriter.h"
#include "util/stringatom.h"
#include "util/dictionary.h"
#include "util/delegate.h"
#include "game/componentid.h"
#include <functional>
namespace Game
{
class ComponentSerialization
{
public:
    using DeserializeJsonFunc = std::function<void(Ptr<IO::JsonReader> const&, const char* name, void*)>;
    using SerializeJsonFunc   = std::function<void(Ptr<IO::JsonWriter> const&, const char* name, void*)>;
    enum OverridableType
    {
        ENTITY,
        RESOURCE,
        NUM_OVERRIDABLE_TYPES
    };
    static ComponentSerialization* Instance();
    static void Destroy();
    template<typename TYPE>
    static void Register(ComponentId component);
    static void Deserialize(Ptr<IO::JsonReader> const& reader, ComponentId component, void* ptr);
    static void Serialize(Ptr<IO::JsonWriter> const& writer, ComponentId component, void* ptr);
    static void Override(ComponentId component, DeserializeJsonFunc deserialize, SerializeJsonFunc serialize);
    static void OverrideType(OverridableType type, DeserializeJsonFunc deserialize, SerializeJsonFunc serialize);
    static DeserializeJsonFunc& GetTypeDeserializeFunc(OverridableType type);
    static SerializeJsonFunc& GetTypeSerializeFunc(OverridableType type);
private:
    ComponentSerialization();
    ~ComponentSerialization();
    static ComponentSerialization* Singleton;
    bool ValidateTypeSize(ComponentId component, uint32_t size);
    struct Serializer
    {
        DeserializeJsonFunc deserializeJson;
        SerializeJsonFunc serializeJson;
    };
    Util::Array<Serializer> serializers;
    Serializer typeOverrides[NUM_OVERRIDABLE_TYPES];
};
template<typename TYPE>
inline void
ComponentSerialization::Register(ComponentId component)
{
    const auto read = [](Ptr<IO::JsonReader> const& reader, const char* name, void* ptr)
    {
        TYPE& value = *(static_cast<TYPE*>(ptr));
        reader->Get(value, name);
    };
    const auto write = [](Ptr<IO::JsonWriter> const& writer, const char* name, void* ptr)
    {
        TYPE& value = *(static_cast<TYPE*>(ptr));
        writer->Add(value, name);
    };
    auto* reg = Instance();
    if (!reg->ValidateTypeSize(component, sizeof(TYPE)))
    {
        n_error("Попытка добавить сериализатор, который работает с компонентами другого размера по сравнению с тем, что зарегистрировано в MemDb::AttributeRegistry!");
        return;
    }
    Serializer s;
    s.deserializeJson = read;
    s.serializeJson = write;
    while (reg->serializers.Size() <= component.id)
    {
        reg->serializers.Grow();
        reg->serializers.Resize(reg->serializers.Capacity());
    }
    reg->serializers[component.id] = s;
}
}