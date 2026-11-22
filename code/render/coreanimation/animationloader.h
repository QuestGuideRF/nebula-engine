#pragma once
#include "util/stringatom.h"
#include "resources/resourceloader.h"
#include "coreanimation/animclip.h"
#include "coreanimation/animkeybuffer.h"
#include "coregraphics/config.h"
#include "ids/idallocator.h"
namespace CoreAnimation
{
class AnimationLoader : public Resources::ResourceLoader
{
    __DeclareClass(AnimationLoader);
private:
    friend class AnimSampleBuffer;
    ResourceLoader::ResourceInitOutput InitializeResource(const ResourceLoadJob& job, const Ptr<IO::Stream>& stream) override;
    void Unload(const Resources::ResourceId id) override;
};
}