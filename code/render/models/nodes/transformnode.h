#pragma once
#include "modelnode.h"
#include "math/quat.h"
namespace Models
{
class TransformNode : public ModelNode
{
public:
    TransformNode();
    virtual ~TransformNode();
    void GetLODDistances(float& minDistance, float& maxDistance);
protected:
    friend class ModelLoader;
    friend class ModelContext;
    virtual bool Load(const Util::FourCC& fourcc, const Util::StringAtom& tag, const Ptr<IO::BinaryReader>& reader, bool immediate) override;
    Math::vec3 position;
    Math::quat rotate;
    Math::vec3 scale;
    Math::vec3 rotatePivot;
    Math::vec3 scalePivot;
    bool isInViewSpace;
    float minDistance;
    float maxDistance;
    bool useLodDistances;
    bool lockedToViewer;
};
}