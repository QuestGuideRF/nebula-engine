#pragma once
#include "core/types.h"
#include "util/string.h"
#include "io/filetime.h"
#include "util/stringatom.h"
namespace ToolkitUtil
{
class AssetFile
{
public:
    enum State
    {
        Unknown,
        New,
        Deleted,
        Changed,
        Unchanged,
        Touched,
        Error,
    };
    AssetFile();
    void SetPath(const Util::StringAtom& path);
    const Util::StringAtom& GetPath() const;
    void SetTimeStamp(IO::FileTime fileTime);
    IO::FileTime GetTimeStamp() const;
    void SetChecksum(uint crc);
    uint GetChecksum() const;
    void SetState(State s);
    State GetState() const;
private:
    Util::StringAtom path;
    IO::FileTime timeStamp;
    uint checksum;
    State state;
};
}