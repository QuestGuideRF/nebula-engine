#pragma once
#include "toolkitutil/assetregistry.h"
namespace ToolkitUtil
{
class AssetUpdater
{
public:
    AssetUpdater();
    void SetLocalRootPath(const Util::String& path);
    const Util::String& GetLocalRootPath() const;
    void SetRemoteRootPath(const Util::String& path);
    const Util::String& GetRemoteRootPath() const;
    bool Update(Logger& logger, const AssetRegistry& diffSet);
private:
    Util::String localRootPath;
    Util::String remoteRootPath;
};
inline void
AssetUpdater::SetLocalRootPath(const Util::String& path)
{
    this->localRootPath = path;
}
inline const Util::String&
AssetUpdater::GetLocalRootPath() const
{
    return this->localRootPath;
}
inline void
AssetUpdater::SetRemoteRootPath(const Util::String& path)
{
    this->remoteRootPath = path;
}
inline const Util::String&
AssetUpdater::GetRemoteRootPath() const
{
    return this->remoteRootPath;
}
}