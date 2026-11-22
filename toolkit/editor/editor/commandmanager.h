#pragma once
#include "core/refcounted.h"
#include "core/singleton.h"
#include "core/ptr.h"
#include "command.h"
namespace Edit
{
struct CommandManager
{
    struct CommandStack
    {
        Util::String name;
        bool listAll = true;
        Util::StackArray<Command*, 1> commands;
    };
    typedef Util::List<CommandStack> CommandList;
    static void Create(uint scratchBufferSize);
    static void Discard();
    static bool CanUndo();
    static bool CanRedo();
    static SizeT GetUndoLevel();
    static void SetUndoLevel(SizeT newValue);
    static bool IsDirty();
    static CommandStack GetLastUndoCommand();
    static CommandStack GetLastRedoCommand();
    static CommandList const& GetUndoList();
    static CommandList const& GetRedoList();
    static bool Execute(Command* command);
    static void BeginMacro(const char* name = nullptr, bool fullHistory = true);
    static void EndMacro();
    static void Undo();
    static void Redo();
    static void Clear();
    static void SetClean();
    static void ClearUndoList();
    static void ClearRedoList();
    static ubyte* AllocScratch(uint size);
private:
    static void AddUndo(Command* command);
    static void AddRedo(Command* command);
};
}