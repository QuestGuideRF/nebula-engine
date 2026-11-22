#pragma once
#include "core/refcounted.h"
#include "toolkitutil/platform.h"
#include "io/uri.h"
#include "util/stringatom.h"
#include "resources/streaming/textureinfo.h"
#include "io/stream.h"
#include "resources/streaming/poolresourcemapper.h"
#define DEBUG_RES_DICT_BUILDER (1)
namespace ToolkitUtil
{
class ResourceDictionaryBuilder
{
public:
    ResourceDictionaryBuilder(void);
    ~ResourceDictionaryBuilder(void);
    void SetPlatform(Platform::Code platform);
    void SetTextureDirectory(const Util::String& texDir);
    bool BuildDictionary();
    void Unload();
private:
    bool AddTexturesToDictionary();
    bool SaveDictionary();
    bool ReadTextureData(Ptr<IO::Stream>& stream, Resources::TextureInfo& format);
    void CreateDefaultTexturePool(const IO::URI& fileName, uint maxBytes);
    static const SizeT MaxResIdLength = 124;
    Platform::Code platform;
    Util::String texDir;
    Util::Dictionary<Util::StringAtom, Resources::TextureInfo> dict;
};
inline void
ResourceDictionaryBuilder::SetPlatform(Platform::Code p)
{
    this->platform = p;
}
inline void
ResourceDictionaryBuilder::SetTextureDirectory(const Util::String& dir)
{
    this->texDir = dir;
}
}