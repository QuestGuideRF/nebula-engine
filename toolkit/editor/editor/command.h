#pragma once
namespace Edit
{
struct CommandManager;
struct Command
{
    virtual ~Command() {};
    virtual const char* Name() = 0;
    virtual bool Execute() = 0;
    virtual bool Unexecute() = 0;
protected:
    friend CommandManager;
    bool executed = false;
};
}