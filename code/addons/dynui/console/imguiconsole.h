#pragma once
#include "core/refcounted.h"
#include "scripting/scriptserver.h"
#include "util/ringbuffer.h"
#include "io/textwriter.h"
namespace Dynui
{
class ImguiConsole : public Core::RefCounted
{
    __DeclareClass(ImguiConsole);
    __DeclareInterfaceSingleton(ImguiConsole);
public:
    enum LogMessageType
    {
        N_MESSAGE = 0,
        N_INPUT = 1,
        N_WARNING = 2,
        N_ERROR = 3,
        N_EXCEPTION = 4,
        N_SYSTEM = 5
    };
    struct LogEntry
    {
        LogMessageType type;
        Util::String msg;
    };
    ImguiConsole();
    virtual ~ImguiConsole();
    void Setup();
    void Discard();
    void Render();
    void RenderContent();
    void AppendToLog(const LogEntry& msg);
    bool EvaluateCVar(Util::Array<Util::String> const& splits);
    Util::Array<Util::String> previousCommands;
    int previousCommandIndex;
    enum CommandMode
    {
        Python,
        CVar
    } cmdMode;
private:
    const char * LogEntryTypeAsCharPtr(const LogMessageType & type) const;
    bool scrollToBottom;
    void Execute(const Util::String& command);
    char command[65535];
    Util::RingBuffer<LogEntry> consoleBuffer;
    IndexT selectedSuggestion;
    Ptr<Scripting::ScriptServer> scriptServer;
    bool visible;
    Ptr<IO::TextWriter> persistentHistory;
};
}