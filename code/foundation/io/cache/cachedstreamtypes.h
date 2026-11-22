#pragma once
#include "core/config.h"
#include "io/cache/cachedstream.h"
namespace IO
{
class CachedHttpStream : public CachedStream
{
    __DeclareClass(CachedHttpStream);
protected:
    virtual Core::Rtti const& GetParentRtti();
};
}