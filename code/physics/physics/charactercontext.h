#pragma once
#include "physicsinterface.h"
#include "util/bitfield.h"
namespace Physics
{
class CharacterContext
{
public:
    static CharacterId CreateCharacter(Math::vec3 const& position, CharacterCreateInfo const& info, IndexT scene = 0);
    static void DestroyCharacter(CharacterId characterId);
    static Character& GetCharacter(CharacterId id);
    static void SetCharacterBoxSize(CharacterId characterId, Math::vec3 const& halfExtents);
    static void SetCharacterHeight(CharacterId characterId, float newHeight);
    static void SetCharacterPosition(CharacterId characterId, Math::vec3 const& newPosition);
    static Math::vec3 GetCharacterCenterPosition(CharacterId characterId);
    static Math::vec3 GetCharacterFootPosition(CharacterId characterId);
    static CharacterCollision MoveCharacter(CharacterId characterId, Math::vector const& displacement, float minDist, float timeSinceLastMove);
};
}