#pragma once
#include "toolkitutil/platform.h"
#include "toolkitutil/converters/n2converter.h"
#include "toolkitutil/texutil/textureconverter.h"
#include "toolkitutil/animutil/animconverter.h"
#include "toolkitutil/projectinfo.h"
namespace ToolkitUtil
{
class N2BatchConverter
{
public:
    N2BatchConverter();
    ~N2BatchConverter();
    void SetPlatform(Platform::Code platform);
    void SetForceFlag(bool b);
    void SetVerbose(bool b);
    void SetExternalTextureAttrTable(const TextureAttrTable* extTexAttrTable);
    static bool CheckRequiredProjectInfoAttrs(const ProjectInfo& projInfo);
    void Setup(Logger& logger, const ProjectInfo& projInfo);
    void Discard();
    bool IsValid() const;
    bool ConvertFile(const Util::String& category, const Util::String& srcFile);
    bool ConvertCategory(const Util::String& category);
    bool ConvertAll();
private:
    bool ConvertResources(const Util::Array<Util::String>& resIds);
    Util::String BuildSourceTexturePath(const Util::String& resId) const;
    Util::String BuildSourceAnimPath(const Util::String& resId) const;
    Platform::Code platform;
    bool force;
    bool verbose;
    bool isValid;
    Logger* logger;
    const TextureAttrTable* texAttrTable;
    N2Converter n2Converter;
    TextureConverter texConverter;
    AnimConverter animConverter;
};
inline bool
N2BatchConverter::IsValid() const
{
    return this->isValid;
}
inline void
N2BatchConverter::SetPlatform(Platform::Code p)
{
    this->platform = p;
}
inline void
N2BatchConverter::SetForceFlag(bool b)
{
    this->force = b;
}
inline void
N2BatchConverter::SetVerbose(bool b)
{
    this->verbose = b;
}
inline void
N2BatchConverter::SetExternalTextureAttrTable(const TextureAttrTable* extTexAttrTable)
{
    this->texAttrTable = extTexAttrTable;
}
}