#pragma once
#include "core/types.h"
#include "posteffect/posteffectentity.h"
namespace ToolkitUtil
{
class PostEffectParser
{
public:
    static bool Load(const Util::String & path, PostEffect::PostEffectEntity::ParamSet & parms);
    static void Save(const Util::String & path, const PostEffect::PostEffectEntity::ParamSet & parms);
};
}