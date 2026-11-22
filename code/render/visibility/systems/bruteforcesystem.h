#pragma once
#include "visibilitysystem.h"
#include "jobs/jobs.h"
namespace Visibility
{
class BruteforceSystem : public VisibilitySystem
{
private:
    friend class ObserverContext;
    void Setup(const BruteforceSystemLoadInfo& info);
    void Run(const Threading::AtomicCounter* previousSystemCompletionCounters, const Util::FixedArray<const Threading::AtomicCounter*, true>& extraCounters) override;
};
}