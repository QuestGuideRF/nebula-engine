#pragma once
#include "core/exithandler.h"
namespace Attr
{
class AttrExitHandler : public Core::ExitHandler
{
public:
    virtual void OnExit() const;
};
}