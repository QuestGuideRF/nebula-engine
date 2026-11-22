//------------------------------------------------------------------------------
//  entityloader.cc
//  @copyright (C) 2021 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "foundation/stdneb.h"
#include "entityloader.h"
#include "io/jsonreader.h"
#include "io/jsonwriter.h"
#include "io/ioserver.h"
#include "editor.h"
#include "cmds.h"
#include "commandmanager.h"
#include "game/componentserialization.h"
#include "basegamefeature/managers/blueprintmanager.h"
#include "editor/components/editorcomponents.h"

namespace Editor
{

__ImplementClass(Editor::EntityLoader, 'EELo', BaseGameFeature::LevelParser);

//------------------------------------------------------------------------------
/**
*/
void
WriteEntityGuid(Ptr<IO::JsonWriter> const& writer, const char* name, void* value)
{
    Game::Entity const entity = *(Game::Entity*)value;
    if (entity == Game::Entity::Invalid() || !Editor::state.editorWorld->IsValid(entity))
    {
        return; // не нужно писать неверные данные
    }
    
    Editor::Editable const& edit = Editor::state.editables[entity.index];
    writer->Add(edit.guid.AsString(), name);
}

//------------------------------------------------------------------------------
/**
*/
bool
SaveEntities(const char* filePath)
{
    IO::URI const file = filePath;
    Ptr<IO::JsonWriter> writer = IO::JsonWriter::Create();
    writer->SetStream(IO::IoServer::Instance()->CreateStream(file));

    // TODO: Возможно, переместите это в класс SceneSerializer, который можно будет использовать и вне редактора.

    // TODO: задается только один раз, как сериализация, так и десериализация
    Game::ComponentSerialization::OverrideType(
        Game::ComponentSerialization::ENTITY,
        nullptr,
        &WriteEntityGuid
    );

    if (writer->Open())
    {
        writer->BeginObject("level");
        
        writer->Add(100, "version");

        writer->BeginObject("entities");

        Game::Filter filter = Game::FilterBuilder().Including<Game::Entity>().Build();
        Game::Dataset data = state.editorWorld->Query(filter);
        Game::ComponentId entityPID = Game::GetComponentId<Game::Entity>();

        for (int v = 0; v < data.numViews; v++)
        {
            Game::Dataset::View const& view = data.views[v];
            Editor::Entity const* const entities = (Editor::Entity*)view.buffers[0];

            for (IndexT i = 0; i < view.numInstances; ++i)
            {
                Editor::Entity const& editorEntity = entities[i];
                Editable& edit = state.editables[editorEntity.index];
                Game::EntityMapping const mapping = Editor::state.editorWorld->GetEntityMapping(editorEntity);
                writer->BeginObject(edit.guid.AsString().AsCharPtr());
                writer->Add(edit.name, "name");
                if (edit.templateId != Game::TemplateId::Invalid())
                {
                    writer->Add(Game::BlueprintManager::GetTemplateName(edit.templateId).Value(), "template");
                }
                MemDb::Table const& table = Editor::state.editorWorld->GetDatabase()->GetTable(view.tableId);
                IndexT col = 0;
                if (table.GetAttributes().Size() > 1)
                {
                    writer->BeginObject("components");
                    for (auto component : table.GetAttributes())
                    {
                        uint32_t const flags = MemDb::AttributeRegistry::Flags(component);
                        if (component != entityPID)
                        {
                            SizeT const typeSize = MemDb::AttributeRegistry::TypeSize(component);
                            if (typeSize > 0)
                            {
                                void* buffer = Editor::state.editorWorld->GetInstanceBuffer(
                                    view.tableId, mapping.instance.partition, component
                                );
                                n_assert(buffer != nullptr);
                                Game::ComponentSerialization::Serialize(
                                    writer,
                                    component,
                                    ((byte*)buffer) + MemDb::AttributeRegistry::TypeSize(component) * mapping.instance.index
                                );
                            }
                            else
                            {
                                writer->Add("null", MemDb::AttributeRegistry::GetAttribute(component)->name.Value());
                            }
                        }

                        col++;
                    }
                    writer->End(); // конечные компоненты
                }
                writer->End(); // конечный объект (GUID)
            }
        }

        Game::DestroyFilter(filter);

        writer->End(); // конечные сущности
        writer->End(); // конечный уровень
        return true;
    }

    return false;
}

//------------------------------------------------------------------------------
/**
*/
EntityLoader::EntityLoader()
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
EntityLoader::~EntityLoader()
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
void
EntityLoader::BeginLoad()
{
    Edit::CommandManager::BeginMacro("Загрузить объекты", false);
}

//------------------------------------------------------------------------------
/**
*/
void
EntityLoader::AddEntity(Game::Entity entity, Util::Guid const& guid)
{
    if (Editor::state.editables.Size() >= entity.index)
        Editor::state.editables.Append({});

    // TODO: Нам нужно добавить объекты с направляющими, прежде чем мы фактически столкнемся с
    //       реальная сущность иногда, если на сущность ссылаются до того, как она будет
    //       instantiated.

    Editable& editable = Editor::state.editables[entity.index];
    
    n_assert(editable.gameEntity == Game::Entity::Invalid());
    
    editable.guid = guid;

    editable.version++;
}

//------------------------------------------------------------------------------
/**
*/
void
EntityLoader::SetName(Game::Entity entity, const Util::String& name)
{
    Editable& editable = Editor::state.editables[entity.index];
    editable.name = name;
}

//------------------------------------------------------------------------------
/**
*/
void
EntityLoader::CommitEntity(Editor::Entity editorEntity)
{
    Editable& editable = Editor::state.editables[editorEntity.index];
    Game::World* gameWorld = Game::GetWorld(WORLD_DEFAULT);
    
    Game::Entity gameEntity = gameWorld->AllocateEntityId();
    
    editable.gameEntity = gameEntity;
    editable.version++;

    // Найдите правильную таблицу для этого объекта редактора.
    Game::World* editorWorld = Editor::state.editorWorld;
    auto const mapping = editorWorld->GetEntityMapping(editorEntity);
    MemDb::Table const& editorTable = editorWorld->GetDatabase()->GetTable(mapping.table);
    MemDb::TableSignature const& signature = editorTable.GetSignature();

    MemDb::TableId gameTableId = gameWorld->GetDatabase()->FindTable(signature);
    if (gameTableId == MemDb::TableId::Invalid())
    {
        // Create table if not exists
        Game::EntityTableCreateInfo info;
        info.components = editorTable.GetAttributes();
        info.name = editorTable.name.Value();
        gameTableId = gameWorld->CreateEntityTable(info);
    }

    Util::Blob entityData = editorTable.SerializeInstance(mapping.instance);
    gameWorld->AllocateInstance(gameEntity, gameTableId, &entityData);

    Editor::EditorEntity* editorEntityComponent = gameWorld->AddComponent<Editor::EditorEntity>(gameEntity);
    editorEntityComponent->id = (uint64_t)editorEntity;
}

//------------------------------------------------------------------------------
/**
*/
void
EntityLoader::CommitLevel()
{
    // TODO: добавьте команду для уровня загрузки, чтобы мы могли ее отменить.
    Edit::CommandManager::EndMacro();
}

} // Редактор пространства имен
