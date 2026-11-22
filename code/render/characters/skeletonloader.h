#pragma once
#include "resources/resourceloader.h"
#include "util/fixedarray.h"
#include "util/hashtable.h"
#include "skeleton.h"
namespace Characters
{
class SkeletonLoader : public Resources::ResourceLoader
{
    __DeclareClass(SkeletonLoader);
private:
    ResourceLoader::ResourceInitOutput InitializeResource(const ResourceLoadJob& job, const Ptr<IO::Stream>& stream) override;
    void Unload(const Resources::ResourceId id) override;
};
}