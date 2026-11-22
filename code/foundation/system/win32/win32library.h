#pragma once
#include "util/string.h"
#include "io/uri.h"
#include "io/stream.h"
#include "system/base/librarybase.h"
namespace Win32
{
class Win32Library : public Base::Library
{
public:
    Win32Library();
    bool Load() override;
    void Close() override;
    void* GetExport(Util::String const& name) const override;
    bool IsLoaded() override;
private:
    HINSTANCE hInstance;
};
}