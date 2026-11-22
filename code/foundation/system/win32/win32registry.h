#pragma once
#include "core/types.h"
#include "util/string.h"
namespace Win32
{
class Win32Registry
{
public:
    enum RootKey
    {
        ClassesRoot,
        CurrentUser,
        LocalMachine,
        Users,
    };
    static bool Exists(const Util::String& vendor, const Util::String& key, const Util::String& name);
    static bool WriteString(const Util::String& vendor, const Util::String& key, const Util::String& name, const Util::String& value);
    static Util::String ReadString(const Util::String& vendor, const Util::String& key, const Util::String& name);
    static bool Delete(const Util::String& vendor, const Util::String& key);
    static bool Exists(RootKey rootKey, const Util::String& key, const Util::String& name);
    static bool WriteString(RootKey rootKey, const Util::String& key, const Util::String& name, const Util::String& value);
    static Util::String ReadString(RootKey rootKey, const Util::String& key, const Util::String& name);
    static int ReadInt(RootKey rootKey, const Util::String& key, const Util::String& name);
    static bool Delete(RootKey rootKey, const Util::String& key);
    static RootKey AsRootKey(const Util::String& str);
private:
    static HKEY RootKeyToWinKeyHandle(RootKey rootKey);
};
}