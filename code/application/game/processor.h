#pragma once
#include "util/stringatom.h"
#include "filter.h"
#include "processorid.h"
namespace Game
{
class World;
class ProcessorBuilder;
class Processor
{
public:
    Util::String name;
    int order = 100;
    bool async = false;
    Filter filter;
    std::function<void(World*, Dataset::View const&)> callback;
    Util::Array<MemDb::TableId> cache;
    bool cacheValid = false;
    bool active = true;
#ifdef WITH_NEBULA_EDITOR
    bool runInEditor = false;
#endif
private:
    friend ProcessorBuilder;
    template <typename... TYPES, std::size_t... Is>
    static void
    UpdateExpander(World* world, std::function<void(World*, TYPES...)> const& func, Game::Dataset::View const& view, const IndexT instance, uint8_t const bufferStartOffset, std::index_sequence<Is...>)
    {
        func(
            world,
            *((typename std::remove_const<typename std::remove_reference<TYPES>::type>::type*)view.buffers[Is] + instance)...
        );
    }
    template <typename... COMPONENTS>
    static std::function<void(World*, Dataset::View const&)>
    ForEach(std::function<void(World*, COMPONENTS...)> func, uint8_t bufferStartOffset)
    {
        return [func, bufferStartOffset](World* world, Game::Dataset::View const& view)
        {
            uint16_t i = 0;
            while (i < view.numInstances)
            {
                if (!view.validInstances.SectionIsNull(i / 64))
                {
                    uint16_t const end = Math::min<uint16_t>(i + uint16_t(64), view.numInstances);
                    for (uint32_t instance = i; instance < end; ++instance)
                    {
                        if (view.validInstances.IsSet(instance))
                        {
                            UpdateExpander<COMPONENTS...>(
                                world,
                                func,
                                view,
                                instance,
                                bufferStartOffset,
                                std::make_index_sequence<sizeof...(COMPONENTS)>()
                            );
                        }
                    }
                }
                i += 64;
            }
        };
    }
    template <typename... COMPONENTS>
    static std::function<void(World*, Dataset::View const&)>
    ForEachModified(std::function<void(World*, COMPONENTS...)> func, uint8_t bufferStartOffset)
    {
        return [func, bufferStartOffset](World* world, Game::Dataset::View const& view)
        {
            uint16_t i = 0;
            while (i < view.numInstances)
            {
                uint64_t section = i / 64;
                if (!view.validInstances.SectionIsNull(section) && !view.modifiedInstances.SectionIsNull(section))
                {
                    uint16_t const end = Math::min<uint16_t>(i + uint16_t(64), view.numInstances);
                    for (uint32_t instance = i; instance < end; ++instance)
                    {
                        if (view.validInstances.IsSet(instance) && view.modifiedInstances.IsSet(instance))
                        {
                            UpdateExpander<COMPONENTS...>(
                                world,
                                func,
                                view,
                                instance,
                                bufferStartOffset,
                                std::make_index_sequence<sizeof...(COMPONENTS)>()
                            );
                        }
                    }
                }
                i += 64;
            }
        };
    }
};
class ProcessorBuilder
{
public:
    ProcessorBuilder() = delete;
    ProcessorBuilder(Game::World* world, Util::StringAtom processorName);
    template<typename LAMBDA>
    ProcessorBuilder& Func(LAMBDA);
    template<typename ...COMPONENTS>
    ProcessorBuilder& Func(std::function<void(World*, COMPONENTS...)> func);
    template<typename ... COMPONENTS>
    ProcessorBuilder& Including();
    template<typename ... COMPONENTS>
    ProcessorBuilder& Excluding();
    ProcessorBuilder& Excluding(std::initializer_list<ComponentId>);
    ProcessorBuilder& On(Util::StringAtom eventName);
    ProcessorBuilder& Async();
    ProcessorBuilder& OnlyModified();
    ProcessorBuilder& Order(int order);
    ProcessorBuilder& RunInEditor();
    Processor* Build();
private:
    World* world;
    Util::StringAtom name;
    Util::StringAtom onEvent;
    std::function<void(World*, Dataset::View const&)> func = nullptr;
    std::function<void(World*, Dataset::View const&)> funcModified = nullptr;
    FilterBuilder filterBuilder;
    bool async = false;
    bool onlyModified = false;
    int order = 100;
#ifdef WITH_NEBULA_EDITOR
    bool runInEditor = false;
#endif
};
template<typename LAMBDA>
ProcessorBuilder& ProcessorBuilder::Func(LAMBDA lambda)
{
    static_assert(std::is_invocable<LAMBDA()>());
    return this->Func(std::function(lambda));
}
template<typename ...COMPONENTS>
inline ProcessorBuilder&
ProcessorBuilder::Func(std::function<void(World*, COMPONENTS...)> func)
{
    uint8_t const bufferStartOffset = this->filterBuilder.GetNumInclusive();
    this->filterBuilder.Including<COMPONENTS...>();
    this->func = Processor::ForEach(func, bufferStartOffset);
    this->funcModified = Processor::ForEachModified(func, bufferStartOffset);
    return *this;
}
template<typename ...COMPONENTS>
inline ProcessorBuilder& ProcessorBuilder::Including()
{
    this->filterBuilder.Including<COMPONENTS...>();
    return *this;
}
template<typename ...COMPONENTS>
inline ProcessorBuilder& ProcessorBuilder::Excluding()
{
    this->filterBuilder.Excluding<COMPONENTS...>();
    return *this;
}
}