#pragma once
#include "game/processor.h"
#include "threading/assertingmutex.h"
namespace Game
{
class FramePipeline;
struct ProcessorJobInput
{
    Game::Dataset::View* view;
    Processor* processor;
};
struct ProcessorJobContext
{
    Game::World* world;
    ProcessorJobInput* inputs;
};
class FrameEvent
{
public:
    FrameEvent() = default;
    ~FrameEvent();
    class Batch;
    void Run(World* world);
    void AddProcessor(Processor* processor);
    void RemoveProcessor(Processor* processor);
    void Prefilter(World* world, bool force = false);
    void CacheTable(MemDb::TableId, MemDb::TableSignature const&);
    Util::StringAtom name;
    int order = 100;
    Util::Array<Batch const*> const GetBatches() const;
private:
    friend FramePipeline;
    FramePipeline* pipeline;
    Util::Array<Batch*> batches;
};
class FrameEvent::Batch
{
public:
    Batch() = default;
    ~Batch();
    void Execute(World* world);
    bool TryInsert(Processor* processor);
    void Prefilter(World* world, bool force = false);
    void CacheTable(MemDb::TableId, MemDb::TableSignature const&);
    int order = 100;
    bool async = false;
    Util::Array<Processor const*> GetProcessors() const;
private:
    void ExecuteAsync(World* world);
    void ExecuteSequential(World* world);
    Util::Array<Processor*> processors;
};
class FramePipeline
{
public:
    FramePipeline() = delete;
    FramePipeline(World* world);
    ~FramePipeline();
    FrameEvent* RegisterFrameEvent(int order, Util::StringAtom name);
    FrameEvent* GetFrameEvent(Util::StringAtom name);
    void Begin();
    void End();
    void Reset();
    void RunThru(Util::StringAtom name);
    void RunRemaining();
    bool IsRunningAsync();
    void Prefilter(bool force = false);
    void CacheTable(MemDb::TableId, MemDb::TableSignature const&);
    Util::Array<FrameEvent const*> const GetFrameEvents() const;
private:
    friend FrameEvent;
    World* world;
    bool isRunning = false;
    volatile bool inAsync = false;
    IndexT currentIndex = 0;
    Util::Array<FrameEvent*> frameEvents;
};
}