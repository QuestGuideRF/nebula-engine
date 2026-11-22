#pragma once
namespace Visibility
{
enum ObserverMask : uint8_t
{
    Graphics    = (1 << 0),
    Observers   = (1 << 1),
    All = (1 << 2),
    NumObserverMasks = 2
};
__ImplementEnumBitOperators(ObserverMask);
enum ObserverType : uint8_t
{
    Perspective         = 0,
    Orthographic        = 1,
    BoundingBox         = 2,
    Omnipercipient      = 3,
    NumObserverTypes    = 4
};
__ImplementEnumBitOperators(ObserverType);
enum VisibilityEntityType : uint8_t
{
    Model,
    Particle,
    Camera,
    Light,
    LightProbe
};
}