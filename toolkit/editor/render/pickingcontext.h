#pragma once
#include "graphics/graphicscontext.h"
#include "game/entity.h"
namespace Editor
{
class PickingContext : public Graphics::GraphicsContext
{
    __DeclareContext()
public:
	PickingContext();
	~PickingContext();
    void SetupEntity(Graphics::GraphicsEntityId id, Game::Entity editorEntity);
private:
    typedef Ids::IdAllocator<
        Game::Entity
    > PickingIdAllocator;
    static PickingIdAllocator allocator;
};
}