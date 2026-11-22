#pragma once
#include "visibilitysystem.h"
#include "jobs/jobs.h"
namespace Visibility
{
class PortalSystem : public VisibilitySystem
{
public:
private:
    friend class ObserverContext;
    void Setup(const PortalSystemLoadInfo& info);
};
}