#pragma once
#include "core/refcounted.h"
#include "util/array.h"
#include "util/string.h"
namespace IO
{
class ConsoleHandler : public Core::RefCounted
{
    __DeclareClass(ConsoleHandler);
public:
    ConsoleHandler();
    virtual ~ConsoleHandler();
    virtual void Open();
    virtual void Close();
    bool IsOpen() const;
    virtual void Update();
    virtual void Print(const Util::String& s);
    virtual void Error(const Util::String& s);
    virtual void Warning(const Util::String& s);
    virtual void Confirm(const Util::String& s);
    virtual void DebugOut(const Util::String& s);
    virtual bool HasInput();
    virtual Util::String GetInput();
private:
    bool isOpen;
};
inline
bool
ConsoleHandler::IsOpen() const
{
    return this->isOpen;
}
}