#pragma once
#include "core/types.h"
#include "toolkitutil/assetfile.h"
#include "toolkit-common/logger.h"
#include "util/dictionary.h"
#include "timing/timer.h"
#include "io/uri.h"
namespace ToolkitUtil
{
class AssetRegistry
{
public:
    AssetRegistry();
    void SetRootPath(const Util::String& path);
    const Util::String& GetRootPath() const;
    void SetRegistryFile(const Util::String& path);
    const Util::String& GetRegistryFile() const;
    void SetAssetDirectories(const Util::Array<Util::String>& dirs);
    const Util::Array<Util::String>& GetAssetDirectories() const;
    bool UpdateLocal(Logger& logger);
    bool UpdateRemote(Logger& logger);
    bool SaveRegistry(Logger& logger);
    bool BuildDifference(Logger& logger, AssetRegistry& localRegistry, const AssetRegistry& remoteRegistry);
    SizeT GetNumAssets() const;
    const AssetFile& GetAssetByIndex(IndexT i) const;
private:
    bool LoadRegistryFile(Logger& logger, const IO::URI& uri);
    bool SaveRegistryFile(Logger& logger, const IO::URI& uri);
    bool UpdateRegistry(Logger& logger);
    bool RecurseUpdateRegistry(Logger& logger, const Util::String& dir);
    void AddAssetFile(Logger& logger, const Util::StringAtom& filePath);
    void UpdateAssetFile(Logger& logger, const Util::StringAtom& filePath);
    void RemoveObsoleteFilesFromRegistry(Logger& logger);
    Util::String rootPath;
    Util::String registryPath;
    Util::Array<Util::String> directories;
    Util::Dictionary<Util::StringAtom, AssetFile> registry;
    SizeT numScannedFiles;
    SizeT numAddedFiles;
    SizeT numChangedFiles;
    SizeT numIdenticalFiles;
    SizeT numDeletedFiles;
    SizeT numTouchedFiles;
    SizeT numErrorFiles;
    Timing::Timer updateLocalTimer;
};
inline SizeT
AssetRegistry::GetNumAssets() const
{
    return this->registry.Size();
}
inline const AssetFile&
AssetRegistry::GetAssetByIndex(IndexT i) const
{
    return this->registry.ValueAtIndex(i);
}
inline void
AssetRegistry::SetRootPath(const Util::String& path)
{
    this->rootPath = path;
}
inline const Util::String&
AssetRegistry::GetRootPath() const
{
    return this->rootPath;
}
inline void
AssetRegistry::SetRegistryFile(const Util::String& path)
{
    this->registryPath = path;
}
inline const Util::String&
AssetRegistry::GetRegistryFile() const
{
    return this->registryPath;
}
inline void
AssetRegistry::SetAssetDirectories(const Util::Array<Util::String>& dirs)
{
    this->directories = dirs;
}
inline const Util::Array<Util::String>&
AssetRegistry::GetAssetDirectories() const
{
    return this->directories;
}
}