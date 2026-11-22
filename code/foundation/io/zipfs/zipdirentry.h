#pragma once
#include "core/types.h"
#include "io/zipfs/zipfileentry.h"
namespace IO
{
class ZipDirEntry
{
public:
    ZipDirEntry();
    const Util::StringAtom& GetName() const;
    ZipFileEntry* FindFileEntry(const Util::StringAtom& name) const;
    ZipDirEntry* FindDirEntry(const Util::StringAtom& name) const;
    const Util::Array<ZipDirEntry>& GetDirEntries() const;
    const Util::Array<ZipFileEntry>& GetFileEntries() const;
private:
    friend class ZipArchive;
    void SetName(const Util::StringAtom& n);
    ZipFileEntry* AddFileEntry(const Util::StringAtom& name);
    ZipDirEntry* AddDirEntry(const Util::StringAtom& name);
    Util::StringAtom name;
    Util::Array<ZipFileEntry> fileEntries;
    Util::Array<ZipDirEntry> dirEntries;
    Util::Dictionary<Util::StringAtom, IndexT> fileIndexMap;
    Util::Dictionary<Util::StringAtom, IndexT> dirIndexMap;
};
inline void
ZipDirEntry::SetName(const Util::StringAtom& n)
{
    this->name = n;
}
inline const Util::StringAtom&
ZipDirEntry::GetName() const
{
    return this->name;
}
inline const Util::Array<ZipDirEntry>&
ZipDirEntry::GetDirEntries() const
{
    return this->dirEntries;
}
inline const Util::Array<ZipFileEntry>&
ZipDirEntry::GetFileEntries() const
{
    return this->fileEntries;
}
}