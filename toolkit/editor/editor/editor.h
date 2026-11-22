#pragma once
#include "util/string.h"
#include "util/guid.h"
#include "game/category.h"
#include "game/gameserver.h"
namespace Editor
{
constexpr uint32_t WORLD_EDITOR = uint32_t('EWLD');
constexpr uint32_t TIMESOURCE_EDITOR = uint32_t('TsEd');
typedef Game::Entity Entity;
struct Editable
{
    Util::Guid guid;
    Util::String name;
    Game::TemplateId templateId;
    Game::Entity gameEntity = Game::Entity::Invalid();
    uint64_t version = 0;
};
struct State
{
    Game::World* editorWorld;
    Util::Array<Editable> editables;
};
void Create();
void Start();
void Destroy();
void PlayGame();
void PauseGame();
void StopGame();
extern State state;
}