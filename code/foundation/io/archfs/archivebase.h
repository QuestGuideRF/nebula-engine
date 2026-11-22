#pragma once
#include "core/refcounted.h"
#include "io/uri.h"
namespace IO
{
class ArchiveBase : public Core::RefCounted
{
    __DeclareClass(ArchiveBase);
public:
    ArchiveBase();
    virtual ~ArchiveBase();
    bool Setup(const URI& archiveURI, const Util::String& rootPath);
    void Discard();
    bool IsValid() const;
    const URI& GetURI() const;
    Util::Array<Util::String> ListFiles(const Util::String& dirPathInArchive, const Util::String& pattern) const;
    Util::Array<Util::String> ListDirectories(const Util::String& dirPathInArchive, const Util::String& pattern) const;
    URI ConvertToArchiveURI(const URI& fileURI) const;
    Util::String ConvertToPathInArchive(const Util::String& absPath) const;
protected:
    bool isValid;
    URI uri;
};
inline bool
ArchiveBase::IsValid() const
{
    return this->isValid;
}
inline const URI&
ArchiveBase::GetURI() const
{
    return this->uri;
}
}