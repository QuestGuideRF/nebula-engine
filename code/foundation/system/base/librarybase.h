#pragma once
#include "util/string.h"
#include "io/uri.h"
#include "io/stream.h"
namespace Base
{
class Library
{
public:
    Library();
    void SetPath(const IO::URI& uri);
    virtual bool Load() = 0;
    virtual void Close() = 0;
    virtual void* GetExport(Util::String const& name) const = 0;
    virtual bool IsLoaded() = 0;
protected:
    bool isLoaded;
    IO::URI path;
};
inline Library::Library()
    : isLoaded(false)
{
}
inline void
Library::SetPath(const IO::URI& uri)
{
    this->path = uri;
}
}