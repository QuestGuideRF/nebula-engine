#pragma once
#include "math/mat4.h"
#include "jobs/jobs.h"
#include "math/bbox.h"
#include "resources/resourceid.h"
#include "graphics/graphicsentity.h"
#include "models/modelcontext.h"
#include "threading/event.h"
#include "jobs2/jobs2.h"
namespace Visibility
{
struct BoxSystemLoadInfo
{
    Resources::ResourceName path;
};
struct PortalSystemLoadInfo
{
    Resources::ResourceName path;
};
struct OctreeSystemLoadInfo
{
    bool worldExpanding;
    uint cellsX, cellsY, cellsZ;
    uint width, height, depth;
    Math::vec4 pos;
};
struct QuadtreeSystemLoadInfo
{
    bool worldExpanding;
    uint cellsX, cellsY;
    uint width, height;
    Math::vec4 pos;
};
struct BruteforceSystemLoadInfo
{
};
class VisibilitySystem
{
public:
    VisibilitySystem();
    virtual void PrepareObservers(const Math::mat4* transforms, bool* orthoFlags, Util::Array<Math::ClipStatus::Type>* results, const SizeT count);
    virtual void PrepareEntities(const Math::bbox* transforms, const uint32_t* ranges, const Graphics::GraphicsEntityId* entities, const uint32_t* entityFlags, const SizeT count);
    virtual void Run(const Threading::AtomicCounter* previousSystemCompletionCounters, const Util::FixedArray<const Threading::AtomicCounter*, true>& extraCounters);
    const Threading::AtomicCounter GetCompletionCounter(IndexT i) const;
    const Threading::AtomicCounter* GetCompletionCounters() const;
protected:
    Math::vec3 center;
    Math::bbox boundingbox;
    struct Observer
    {
        const Math::mat4* transforms;
        const bool* isOrtho;
        Util::Array<Math::ClipStatus::Type>* results;
        SizeT count;
        Util::Array<Threading::AtomicCounter> completionCounters;
    } obs;
    struct Entity
    {
        const Math::bbox* boxes;
        const Graphics::GraphicsEntityId* entities;
        const uint32_t* ids;
        const uint32_t* entityFlags;
        SizeT count;
    } ent;
};
}