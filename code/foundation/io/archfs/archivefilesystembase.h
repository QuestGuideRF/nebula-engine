#pragma once
#include "core/refcounted.h"
#include "core/singleton.h"
#include "util/dictionary.h"
#include "io/uri.h"
namespace IO
{
class Archive;
class ArchiveFileSystemBase : public Core::RefCounted
{
    __DeclareClass(ArchiveFileSystemBase);
    __DeclareInterfaceSingleton(ArchiveFileSystemBase);
public:
    ArchiveFileSystemBase();
    virtual ~ArchiveFileSystemBase();
    void Setup();
    void Discard();
    bool IsValid() const;
    virtual Ptr<Archive> Mount(const URI& uri);
    virtual Ptr<Archive> MountEmbedded(const URI& uri, const Util::String& rootPath);
    virtual void Unmount(const URI& uri);
    virtual void Unmount(const Ptr<Archive>& archive);
    bool IsMounted(const URI& uri) const;
    bool HasArchives() const;
    Util::Array<Ptr<Archive> > GetMountedArchives() const;
    Ptr<Archive> FindArchive(const URI& uri) const;
    virtual Ptr<Archive> FindArchiveWithFile(const URI& fileUri) const;
    virtual Ptr<Archive> FindArchiveWithDir(const URI& dirUri) const;
    URI ConvertFileToArchiveURIIfExists(const URI& uri) const;
    URI ConvertDirToArchiveURIIfExists(const URI& uri) const;
protected:
    Threading::CriticalSection critSect;
    Util::Dictionary<Util::String, Ptr<Archive> > archives;
    bool isValid;
};
inline bool
ArchiveFileSystemBase::IsValid() const
{
    return this->isValid;
}
}