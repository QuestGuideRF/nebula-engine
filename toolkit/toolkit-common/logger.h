#pragma once
#include "util/string.h"
#include "util/array.h"
#include "util/stack.h"
namespace ToolkitUtil
{
class Logger
{
public:
    Logger();
    virtual ~Logger();
    void SetVerbose(bool b);
    bool GetVerbose() const;
    virtual void Error(const char* msg, ...);
    virtual void Warning(const char* msg, ...);
    virtual void Print(const char* msg, ...);
    void Indent();
    void Unindent();
protected:
    bool verbose;
    Util::String indent;
    Util::Array<Util::String> messages;
};
inline void
Logger::SetVerbose(bool b)
{
    this->verbose = b;
}
inline bool
Logger::GetVerbose() const
{
    return this->verbose;
}
inline void
Logger::Indent()
{
    this->indent += "    ";
}
inline void
Logger::Unindent()
{
    this->indent = this->indent.ExtractRange(0, this->indent.Length() - 4);
}
}