#pragma once
#include "physicsinterface.h"
#include "physics/callbacks.h"
#include "ids/idgenerationpool.h"
#include "util/set.h"
#define MAX_SHAPE_OVERLAPS 256
#define PHYSICS_RATE 0.016
namespace Physics
{
class PhysxState : public physx::PxSimulationEventCallback
{
public:
    physx::PxFoundation * foundation;
    physx::PxPhysics * physics;
    physx::PxPvd *pvd;
    physx::PxPvdTransport *transport;
    Util::Delegate<void(ActorId*, SizeT)> onSleepCallback;
    Util::Delegate<void(ActorId*, SizeT)> onWakeCallback;
    Physics::Allocator allocator;
    Physics::ErrorCallback errorCallback;
    Physics::Material FallbackMaterial;
    Util::Set<Ids::Id32> awakeActors;
    Util::Dictionary<Util::StringAtom, IndexT> materialNameTable;
    Util::StackArray<Physics::Scene, 8> activeScenes;
    Util::StackArray<Physics::Material, 16> materials;
    Util::StackArray<IndexT, 8> activeSceneIds;
    Util::StackArray<IndexT, 8> deadSceneIds;
    physx::PxOverlapHit overlapBuffer[MAX_SHAPE_OVERLAPS];
    PhysxState();
    void Setup();
    void Shutdown();
    void Update(Timing::Time delta);
    void BeginSimulating(Timing::Time delta, IndexT scene);
    void EndSimulating(IndexT scene);
    void FlushSimulation(IndexT scene);
    physx::PxRigidActor* CreateActor(ActorType type, Math::mat4 const & transform);
    physx::PxRigidActor* CreateActor(ActorType type, Math::vec3 const& location, Math::quat const& orientation);
    void DiscardActor(ActorId id);
    void ConnectPVD();
    void DisconnectPVD();
    void onConstraintBreak(physx::PxConstraintInfo* constraints, physx::PxU32 count) {}
    void onWake(physx::PxActor** actors, physx::PxU32 count);
    void onSleep(physx::PxActor** actors, physx::PxU32 count);
    void onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs);
    void onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count) {}
    void onAdvance(const physx::PxRigidBody*const* bodyBuffer, const physx::PxTransform* poseBuffer, const physx::PxU32 count) {}
};
extern PhysxState state;
}