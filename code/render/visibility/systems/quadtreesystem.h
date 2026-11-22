#pragma once
#include "visibilitysystem.h"
#include "jobs/jobs.h"
namespace Visibility
{
class QuadtreeSystem : public VisibilitySystem
{
public:
private:
    friend class ObserverContext;
    void Setup(const QuadtreeSystemLoadInfo& info);
};
}