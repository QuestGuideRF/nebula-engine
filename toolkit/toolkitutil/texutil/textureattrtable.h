#pragma once
#include "core/types.h"
#include "toolkitutil/texutil/textureattrs.h"
#include "util/array.h"
#include "util/dictionary.h"
#include "util/stringatom.h"
namespace ToolkitUtil
{
class TextureAttrTable
{
public:
    TextureAttrTable();
    ~TextureAttrTable();
    bool Setup(const Util::String& path);
    bool Save(const Util::String& path);
    void Discard();
    bool IsValid() const;
    bool HasEntry(const Util::String& texName) const;
    const TextureAttrs& GetEntry(const Util::String& texName) const;
    void SetEntry(const Util::String& texName, const TextureAttrs& attrs);
    const TextureAttrs& GetDefaultEntry() const;
private:
    Util::Array<TextureAttrs> texAttrs;
    Util::Dictionary<Util::StringAtom, IndexT> indexMap;
    TextureAttrs defaultAttrs;
    bool valid;
};
inline bool
TextureAttrTable::IsValid() const
{
    return this->valid;
}
}