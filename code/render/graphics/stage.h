#pragma once
#include "core/refcounted.h"
#include "graphicsentity.h"
namespace Graphics
{
class Stage : public Core::RefCounted
{
    __DeclareClass(Stage);
public:
    void AttachEntity(const GraphicsEntityId entity);
    void DetachEntity(const GraphicsEntityId entity);
private:
    friend class GraphicsServer;
    Stage();
    virtual ~Stage();
    Util::Array<GraphicsEntityId> entities;
};
}