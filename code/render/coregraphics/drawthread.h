#pragma once
#include "threading/thread.h"
#include "threading/event.h"
namespace CoreGraphics
{
class DrawThread : public Threading::Thread
{
public:
    DrawThread();
    enum CommandType
    {
        BeginCommand,
        ResetCommand,
        EndCommand,
        GraphicsPipeline,
        ComputePipeline,
        InputAssemblyVertex,
        InputAssemblyIndex,
        Draw,
        IndirectDraw,
        IndirectIndexedDraw,
        Dispatch,
        BindDescriptors,
        PushRange,
        Viewport,
        ViewportArray,
        ScissorRect,
        ScissorRectArray,
        StencilRefs,
        StencilReadMask,
        StencilWriteMask,
        UpdateBuffer,
        SetEvent,
        ResetEvent,
        WaitForEvent,
        Barrier,
        Sync,
        Timestamp,
        BeginQuery,
        EndQuery,
        BeginMarker,
        EndMarker,
        InsertMarker
    };
    struct Command
    {
        IndexT size;
        CommandType type;
    };
    struct SyncCommand
    {
        static const CommandType Type = Sync;
        Threading::Event* event;
    };
    struct CommandBuffer
    {
        byte* buffer;
        SizeT size;
        SizeT capacity;
        CommandBuffer()
            : buffer(nullptr)
            , size(0)
            , capacity(1024)
        {};
        void Append(const byte* buf, SizeT numBytes)
        {
            if (this->buffer == nullptr)
            {
                this->capacity = Math::max(numBytes, this->capacity);
                this->buffer = new byte[this->capacity];
            }
            else if (this->size + numBytes > this->capacity)
            {
                const SizeT grow = Math::max(numBytes, this->capacity >> 1);
                this->capacity += grow;
                byte* newBuf = new byte[this->capacity];
                memcpy(newBuf, this->buffer, this->size);
                delete[] this->buffer;
                this->buffer = newBuf;
            }
            memcpy(this->buffer + this->size, buf, numBytes);
            this->size += numBytes;
        }
        const SizeT Size() const
        {
            return this->size;
        }
        void Clear()
        {
            delete[] this->buffer;
            this->buffer = nullptr;
            this->capacity = 0;
            this->size = 0;
        }
        void Reset()
        {
            this->size = 0;
        }
    };
    void EmitWakeupSignal() override;
    template<typename T> void Push(const T& command);
    void Flush();
    void Signal(Threading::Event* event);
protected:
    Threading::CriticalSection lock;
    Threading::Event signalEvent;
    Util::Array<Command> commands;
    CommandBuffer commandBuffer;
    Threading::Event* event;
};
extern DrawThread* CreateDrawThread();
template<typename T>
inline void
DrawThread::Push(const T& command)
{
    this->lock.Enter();
    Command entry;
    entry.size = sizeof(T);
    entry.type = T::Type;
    this->commands.Append(entry);
    this->commandBuffer.Append(reinterpret_cast<const byte*>(&command), sizeof(T));
    this->lock.Leave();
}
}