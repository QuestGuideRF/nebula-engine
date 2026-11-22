//------------------------------------------------------------------------------
//  levelparser.cc
//  (C) 2024 г. Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "foundation/stdneb.h"
#include "levelparser.h"
#include "game/world.h"

using namespace Util;
using namespace Math;

namespace BaseGameFeature
{

__ImplementClass(BaseGameFeature::LevelParser, 'LVPR', Core::RefCounted);

//------------------------------------------------------------------------------
/**
*/
LevelParser::LevelParser()
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
LevelParser::~LevelParser()
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
void
LevelParser::SetWorld(Game::World* world)
{
    this->world = world;
}

//------------------------------------------------------------------------------
/**
*/
Util::Array<Game::Entity>
LevelParser::LoadJsonLevel(const Ptr<IO::JsonReader> & reader)
{
    auto& g2e = this->guidToEntity;
    Game::ComponentSerialization::OverrideType( // TODO: это должен быть временный объект, который уничтожается в конце области действия, удаляя переопределение.
        Game::ComponentSerialization::ENTITY,
        [&g2e](Ptr<IO::JsonReader> const& reader, const char* name, void* data)
        {
            Util::Guid guid;
            reader->Get<Util::Guid>(guid, name);
            IndexT index = g2e.FindIndex(guid);
            // Убедитесь, что объект уже настроен.
            n_assert(index != InvalidIndex);
            *(Game::Entity*)data = g2e.ValueAtIndex(guid, index);
        },
        nullptr
    );

    this->BeginLoad();

    reader->SetToRoot();
    Util::Array<Game::Entity> entities;

    if (reader->SetToNode("/level"))
    {
        int levelversion = reader->GetInt("version");
        if (levelversion != 100)
        {
            n_warning("Отменить поддержку уровня версии!");
            return {};
        }
        else
        {
            // Теперь прочитайте все сущности, существующие в этой сцене.
            reader->SetToFirstChild("entities");

            this->invalidAttrs.Clear();

            // загрузить объекты, руководство по настройке->карта объектов
            reader->SetToFirstChild();
            this->guidToEntity.BeginBulkAdd();
            do
            {
                if (reader->HasAttr("sub_scene"))
                {
                    // 1. Загрузите подсцену, состоящую из нескольких объектов.
                    // 2. Сгруппируйте их в редакторе.
                    //    Возможно, это можно сделать с помощью своего рода иерархического «Преобразования».
                    //    компонент (изменения в этом преобразовании объектов распространяются на
                    //    это дети, и у детей должен быть «родитель»
                    //    компонент, состоящий из локальной позиции, rot, масштаба и родительского идентификатора)
                    // 3. Привяжите их к ресурсу сцены, чтобы если ресурс
                    //    обновлены, сущности также
                }
                else // обычная сущность, просто загружайся нормально
                {
                    Game::Entity entity = this->LoadEntity(reader);
                    entities.Append(entity);
                }

            } while (reader->SetToNextChild());
            this->guidToEntity.EndBulkAdd();

            // Загрузите компоненты. Мы делаем это отдельно, поскольку сущности и их гид
            // необходимо установить, чтобы иметь возможность вносить исправления из GUID в объект в полях компонента.
            reader->SetToFirstChild();
            IndexT entityIndex = 0;
            do
            {
                if (reader->HasAttr("sub_scene"))
                {
                    // Обязательно загрузите все данные объектов sub_scene.
                }
                else // обычная сущность, просто загружайся нормально
                {
                    Game::Entity entity = entities[entityIndex++];
                    this->LoadComponents(reader, entity);
                }

            } while (reader->SetToNextChild());

            if (!this->invalidAttrs.IsEmpty())
            {
                Util::String levelName;
                if (reader->HasStream())
                {
                    Util::String fileName = reader->GetStream()->GetURI().LocalPath().ExtractFileName();
                    fileName.StripFileExtension();
                    levelName = fileName;
                }
                // выдать сообщение об ошибке, сообщающее, какие компоненты отсутствуют
                Util::String errorMessage;
                errorMessage.Format(
                    "\n\nНа уровне '%s' обнаружены недопустимые компоненты (они были удалены с момента последнего сохранения уровня?):\n\n",
                    levelName.AsCharPtr()
                );

                for (IndexT i = 0; i < invalidAttrs.Size(); i++)
                {
                    errorMessage.Append("\t" + invalidAttrs[i] + "\n");
                }

                n_warning(errorMessage.AsCharPtr());
            }

            world->ExecuteAddComponentCommands();

            for (IndexT i = 0; i < entities.Size(); i++)
            {
                Game::Entity entity = entities[i];
                this->CommitEntity(entity);
            }

            this->CommitLevel();
        }
    }

    Game::ComponentSerialization::
        OverrideType(
            Game::ComponentSerialization::ENTITY,
            nullptr,
            nullptr
        );

    return entities;
}

//------------------------------------------------------------------------------
/**
*/
Game::Entity
LevelParser::LoadEntity(const Ptr<IO::JsonReader> & reader)
{
    Util::Guid guid = Util::Guid::FromString(reader->GetCurrentNodeName());
    
    Util::String entityName = reader->GetOptString("name", "unnamed_entity");

    Game::Entity entity = world->CreateEntity(true);
    
    this->AddEntity(entity, guid);
    this->SetName(entity, entityName);

    this->guidToEntity.Add(guid, entity);

    return entity;
}

//------------------------------------------------------------------------------
/**
*/
void
LevelParser::LoadComponents(const Ptr<IO::JsonReader>& reader, Game::Entity entity)
{
    Util::String entityName = reader->GetOptString("name", "unnamed_entity");

    if (reader->SetToFirstChild("components"))
    {
        uint numChildren = reader->CurrentSize();
        for (uint childIndex = 0; childIndex < numChildren; childIndex++)
        {
            Util::String const componentName = reader->GetChildNodeName(childIndex);
            Game::ComponentId componentId = Game::GetComponentId(componentName);
            if (componentId == Game::ComponentId::Invalid())
            {
                invalidAttrs.Append(entityName + " ->\t" + componentName);
                continue;
            }

            void* componentData = world->AddComponent(entity, componentId);
            Game::ComponentSerialization::Deserialize(reader, componentId, componentData);
        }

        reader->SetToParent();
    }
}

} // пространство имен BaseGameFeature
