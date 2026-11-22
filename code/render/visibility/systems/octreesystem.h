#pragma once
#include "visibilitysystem.h"
#include "jobs/jobs.h"
namespace Visibility
{
class OctreeSystem : public VisibilitySystem
{
public:
private:
    friend class ObserverContext;
    void Setup(const OctreeSystemLoadInfo& info);
};
}