#pragma once
#include "core/types.h"
#include "util/string.h"
namespace Posix
{
class PosixSettings
{
public:
    static bool Exists(const Util::String & vendor, const Util::String& key, const Util::String& name);
    static bool WriteString(const Util::String & vendor, const Util::String& key, const Util::String& name, const Util::String& value);
    static Util::String ReadString(const Util::String & vendor, const Util::String& key, const Util::String& name);
    static bool Delete(const Util::String & vendor, const Util::String& key);
private:
    static Util::String GetUserDir();
};
}