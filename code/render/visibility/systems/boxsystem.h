#pragma once
#include "visibilitysystem.h"
#include "jobs/jobs.h"
namespace Visibility
{
class BoxSystem : public VisibilitySystem
{
public:
private:
    friend class ObserverContext;
    void Setup(const BoxSystemLoadInfo& info);
};
}