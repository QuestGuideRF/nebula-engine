//------------------------------------------------------------------------------
//  blueprintmanager.cc
//  (C) 2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "blueprintmanager.h"
#include "io/jsonreader.h"
#include "io/ioserver.h"
#include "game/componentserialization.h"
#include "util/arraystack.h"
#include "game/gameserver.h"
#include "basegamefeature/components/basegamefeature.h"
#include "basegamefeature/components/position.h"
#include "basegamefeature/components/orientation.h"
#include "basegamefeature/components/scale.h"

namespace Game
{
__ImplementClass(Game::BlueprintManager, 'BpMa', Game::Manager);
__ImplementSingleton(BlueprintManager)

Util::String BlueprintManager::blueprintFolder("tbl:");
Util::String BlueprintManager::templatesFolder("tbl:templates");

//------------------------------------------------------------------------------
/**
*/
void
BlueprintManager::OnActivate()
{
    Manager::OnActivate();

    // сначала настройте «пустой» проект
    Blueprint bluePrint;
    bluePrint.name = "Empty";
    BlueprintId const emptyId = this->blueprints.Size();
    this->blueprints.Append(bluePrint);

    // Настройте все таблицы чертежей
    this->SetupBlueprints();

    // разобрать все шаблоны из папок.
    if (IO::IoServer::Instance()->DirectoryExists(this->templatesFolder))
    {
        this->LoadTemplateFolder(this->templatesFolder);
    }
}

//------------------------------------------------------------------------------
/**
*/
void
BlueprintManager::OnDeactivate()
{
    Manager::OnDeactivate();
}

//------------------------------------------------------------------------------
/**
*/
BlueprintManager::BlueprintManager()
{
    __ConstructSingleton

    Util::Array<Util::String> files = IO::IoServer::Instance()->ListFiles(this->blueprintFolder, "*.json", true);
    for (int i = 0; i < files.Size(); i++)
    {
        if (!this->ParseBlueprint(files[i]))
        {
            n_warning("Предупреждение: Managers::BlueprintManager: ошибка анализа %s!\n", files[i].AsCharPtr());
        }
    }

}

//------------------------------------------------------------------------------
/**
*/
BlueprintManager::~BlueprintManager()
{
    __DestructSingleton
}

//------------------------------------------------------------------------------
/**
    Этот метод анализирует данные файла:tables/blueprints.json в
    массив чертежей.
*/
bool
BlueprintManager::ParseBlueprint(Util::String const& blueprintsPath)
{
    if (IO::IoServer::Instance()->FileExists(blueprintsPath))
    {
        Ptr<IO::JsonReader> jsonReader = IO::JsonReader::Create();
        jsonReader->SetStream(IO::IoServer::Instance()->CreateStream(blueprintsPath));
        if (jsonReader->Open())
        {
            // убедитесь, что это файл BluePrints
            if (!jsonReader->SetToNode("blueprints"))
            {
                n_warning("Предупреждение: BlueprintManager::ParseBlueprints(): недопустимый файл чертежей!\n");
                return false;
            }
            if (jsonReader->SetToFirstChild()) do
            {
                Blueprint bluePrint;
                bluePrint.name = jsonReader->GetCurrentNodeName();

                if (jsonReader->SetToFirstChild("components"))
                {
                    if (jsonReader->IsArray())
                    {
                        Util::Array<Util::String> comps;
                        jsonReader->Get<>(comps);
                        for (Util::String const& name : comps)
                        {
                            bluePrint.components.Append({ name });
                        }
                    }
                    jsonReader->SetToParent();
                }

                this->blueprints.Append(bluePrint);

            } while (jsonReader->SetToNextChild());

            jsonReader->Close();
            return true;
        }
        else
        {
            n_error("Managers::BlueprintManager::ParseBlueprint(): не удалось открыть '%s'!", blueprintsPath.AsCharPtr());
            return false;
        }
    }
    return false;
}

//------------------------------------------------------------------------------
/**
*/
bool
BlueprintManager::LoadTemplateFolder(Util::String const& path)
{
    Util::Array<Util::String> files = IO::IoServer::Instance()->ListFiles(path, "*.json", true);

    // Разбор файлов
    for (int i = 0; i < files.Size(); i++)
    {
        if (!this->ParseTemplate(files[i]))
        {
            n_warning("Managers::BlueprintManager: Ошибка анализа %s!\n", files[i].AsCharPtr());
        }
    }

    // Рекурсировать все папки
    Util::Array<Util::String> dirs = IO::IoServer::Instance()->ListDirectories(path, "*", true);
    for (auto const& dir : dirs)
        this->LoadTemplateFolder(dir);

    return true;
}

//------------------------------------------------------------------------------
/**
*/
bool
BlueprintManager::ParseTemplate(Util::String const& templatePath)
{
    if (IO::IoServer::Instance()->FileExists(templatePath))
    {
        Ptr<IO::JsonReader> jsonReader = IO::JsonReader::Create();
        jsonReader->SetStream(IO::IoServer::Instance()->CreateStream(templatePath));
        if (jsonReader->Open())
        {
            jsonReader->SetToRoot();
            if (!jsonReader->HasNode("blueprint"))
            {
                jsonReader->Close();
                return false;
            }
            Util::StringAtom blueprintName = jsonReader->GetString("blueprint");
            if (this->blueprintMap.Contains(blueprintName))
            {
                // Создание экземпляра шаблона
                Ptr<MemDb::Database> templateDatabase = GameServer::Instance()->state.templateDatabase;
                BlueprintId blueprint = this->blueprintMap[blueprintName];
                MemDb::TableId templateTid = this->blueprints[blueprint.id].tableId;
                MemDb::RowId instance = templateDatabase->GetTable(templateTid).AddRow();
                n_assert2(instance.index < 0xFFFF, "Достигнуто максимальное количество шаблонов в чертеже! Вы выиграли!");

                // Создать имя шаблона
                Util::String fileName = templatePath.ExtractFileName();
                fileName.StripFileExtension();
                Util::String templateName;
                templateName.Append(blueprintName.Value());
                templateName += "/" + fileName;

                Util::StringAtom const nameAtom = Util::StringAtom(templateName);

                TemplateId templateId;
                if (this->templateIdPool.Allocate(templateId.id))
                {
                    this->templates.Append({});
                }

                Template& tmpl = this->templates[Ids::Index(templateId.id)];
                tmpl = Template();
                tmpl.bid = blueprint;
                tmpl.name = nameAtom;
                tmpl.row = instance;
                
                // Добавить на карту
                this->templateMap.Add(nameAtom, templateId);

                // Переопределить компоненты при необходимости
                if (jsonReader->SetToFirstChild("components"))
                {
                    MemDb::Table& table = templateDatabase->GetTable(templateTid);
                    Util::Array<ComponentId> const&  columns = table.GetAttributes();
                    for (ComponentId id : columns)
                    {
                        MemDb::Attribute * descriptor = MemDb::AttributeRegistry::GetAttribute(id);
                        if (descriptor != nullptr)
                        {
                            Util::StringAtom compName = descriptor->name;
                            if (jsonReader->HasAttr(compName.Value()))
                            {
                                MemDb::ColumnIndex column = table.GetAttributeIndex(id);
                                if (column == MemDb::ColumnIndex::Invalid()) continue;
                                void* componentValue = table.GetValuePointer(column, instance);
                                ComponentSerialization::Deserialize(jsonReader, id, componentValue);
                            }
                        }
                    }
                    jsonReader->SetToParent();
                }

                if (jsonReader->SetToFirstChild("variations"))
                {
                    jsonReader->SetToFirstChild();
                    do
                    {
                        Util::StringAtom componentName = jsonReader->GetCurrentNodeName();
                        // ЗАДАЧА: Получить значение!
                    } while (jsonReader->SetToNextChild());

                    // ЗАДАЧА: Создать варианты шаблона в таблице чертежей

                    jsonReader->SetToParent();
                }
            }
            jsonReader->Close();
            return true;
        }
        else
        {
            n_error("Managers::BlueprintManager::ParseTemplate(): не удалось открыть '%s'!", templatePath.AsCharPtr());
            return false;
        }
    }
    return false;
}

//------------------------------------------------------------------------------
/**
*/
void
BlueprintManager::SetBlueprintsFilename(const Util::String& name, const Util::String& folder)
{
    n_assert(name.IsValid());
    blueprintFolder = folder;
}

//------------------------------------------------------------------------------
/**
*/
BlueprintId const
BlueprintManager::GetBlueprintId(Util::StringAtom name)
{
    n_assert(Singleton != nullptr);
    return Singleton->blueprintMap[name];
}

//------------------------------------------------------------------------------
/**
*/
TemplateId const
BlueprintManager::GetTemplateId(Util::StringAtom name)
{
    IndexT index = Singleton->templateMap.FindIndex(name);
    if (index != InvalidIndex)
    {
        TemplateId tid = Singleton->templateMap.ValueAtIndex(name, index);
        return tid;
    }
    else
    {
        return TemplateId::Invalid();
    }
}

//------------------------------------------------------------------------------
/**
*/
void
BlueprintManager::SetupBlueprints()
{
    // create a instance of every component and call SetupDefaultAttributes()
    IndexT idxBluePrint;
    bool failed = false;
    for (idxBluePrint = 0; idxBluePrint < this->blueprints.Size(); idxBluePrint++)
    {
        Blueprint& blueprint = this->blueprints[idxBluePrint];

        if (this->blueprintMap.Contains(blueprint.name))
        {
            n_warning("Дублирующийся проект с именем «%s» найден в blueprints.json\n", blueprint.name.Value());
            continue;
        }
        EntityTableCreateInfo info;
        info.name = blueprint.name.AsString();

        const SizeT numBlueprintComponents = blueprint.components.Size();
        Util::StackArray<ComponentId, 32> columns;

        // добавить владельца и преобразовать, это делает это немного быстрее, чем позволить менеджеру объектов разобраться во всем...
        columns.Append(GetComponentId<Entity>());
        columns.Append(GetComponentId<Position>());
        columns.Append(GetComponentId<Orientation>());
        columns.Append(GetComponentId<Scale>());

        // отфильтровать недопустимые компоненты
        for (int i = 0; i < numBlueprintComponents; i++)
        {
            auto descriptor = MemDb::AttributeRegistry::GetAttributeId(blueprint.components[i].name);
            if (descriptor != ComponentId::Invalid())
            {
                // добавить в массив с динамически изменяемым размером
                columns.Append(descriptor);
            }
            else
            {
                n_warning("Внимание: нераспознанный компонент «%s» в схеме элементов «%s»\n", blueprint.components[i].name.AsString().AsCharPtr(), blueprint.name.Value());
            }
        }

        // переместить компоненты из массива с динамическим размером в фиксированный массив.
        // это немного странно, но тогда нам не нужно делать ничего особенного с недопустимыми компонентами...
        info.components.SetSize(columns.Size());
        for (int i = 0; i < columns.Size(); i++)
        {
            info.components[i] = columns[i];
        }

        if (!failed)
        {
            // Создайте таблицу шаблонов схемы элементов.
            MemDb::TableCreateInfo tableInfo;
            tableInfo.name = "blueprint:" + info.name;
            tableInfo.numAttributes = info.components.Size();
            tableInfo.attributeIds = info.components.Begin();
            Ptr<MemDb::Database> templateDatabase = GameServer::Instance()->state.templateDatabase;
            MemDb::TableId tid = templateDatabase->CreateTable(tableInfo);

            blueprint.tableId = tid;
            this->blueprintMap.Add(blueprint.name, idxBluePrint);

            // Create a template for this blueprints default values
            MemDb::RowId instance = templateDatabase->GetTable(tid).AddRow();
            TemplateId templateId;
            if (this->templateIdPool.Allocate(templateId.id))
                this->templates.Append({});
            Template& tmpl = this->templates[Ids::Index(templateId.id)];
            tmpl = Template();
            tmpl.bid = idxBluePrint;
            tmpl.name = blueprint.name;
            tmpl.row = instance;
            this->templateMap.Add(blueprint.name, templateId);
        }
    }

    if (failed)
    {
        n_error("Прерывание из-за неисправимой ошибки!\n");
    }
}

//------------------------------------------------------------------------------
/**
*/
Util::StringAtom const
BlueprintManager::GetTemplateName(TemplateId const templateId)
{
    n_assert(Singleton->templateIdPool.IsValid(templateId.id));
    return Singleton->templates[Ids::Index(templateId.id)].name;
}

//------------------------------------------------------------------------------
/**
*/
Util::Array<BlueprintManager::Template> const&
BlueprintManager::ListTemplates()
{
    return Singleton->templates;
}

//------------------------------------------------------------------------------
/**
*/
EntityMapping
BlueprintManager::Instantiate(World* const world, BlueprintId blueprint)
{
    GameServer::State& gsState = GameServer::Instance()->state;
    Ptr<MemDb::Database> const& tdb = gsState.templateDatabase;
    IndexT const categoryIndex = world->blueprintToTableMap.FindIndex(blueprint);

    if (categoryIndex != InvalidIndex)
    {
        MemDb::TableId const cid = world->blueprintToTableMap.ValueAtIndex(blueprint, categoryIndex);
        MemDb::RowId const instance = world->db->GetTable(cid).AddRow();
        return {cid, instance};
    }
    return {MemDb::InvalidTableId, MemDb::InvalidRow};
}

//------------------------------------------------------------------------------
/**
*/
EntityMapping
BlueprintManager::Instantiate(World* const world, TemplateId templateId)
{
    n_assert(Singleton->templateIdPool.IsValid(templateId.id));
    GameServer::State& gsState = GameServer::Instance()->state;
    Ptr<MemDb::Database> const& tdb = gsState.templateDatabase;
    Template& tmpl = Singleton->templates[Ids::Index(templateId.id)];
    IndexT const categoryIndex = world->blueprintToTableMap.FindIndex(tmpl.bid);

    if (categoryIndex != InvalidIndex)
    {
        MemDb::TableId const tid = world->blueprintToTableMap.ValueAtIndex(tmpl.bid, categoryIndex);
        MemDb::RowId const instance = MemDb::Table::DuplicateInstance(
            tdb->GetTable(Singleton->blueprints[tmpl.bid.id].tableId), tmpl.row, world->db->GetTable(tid)
        );
        return {tid, instance};
    }
    else
    {
        // Create the table, and then create the instance
        MemDb::TableId const tid = this->CreateCategory(world, tmpl.bid);
        MemDb::RowId const instance = MemDb::Table::DuplicateInstance(
            tdb->GetTable(Singleton->blueprints[tmpl.bid.id].tableId), tmpl.row, world->db->GetTable(tid)
        );
        return {tid, instance};
    }
}

//------------------------------------------------------------------------------
/**
    @todo это можно оптимизировать
*/
MemDb::TableId
BlueprintManager::CreateCategory(World* const world, BlueprintId bid)
{
    EntityTableCreateInfo info;
    info.name = blueprints[bid.id].name.Value();

    auto const& components = GameServer::Singleton->state.templateDatabase->GetTable(blueprints[bid.id].tableId).GetAttributes();
    info.components.Resize(components.Size());
    for (int i = 0; i < components.Size(); i++)
    {
        info.components[i] = components[i];
    }

    MemDb::TableId tid = world->CreateEntityTable(info);
    world->blueprintToTableMap.Add(bid, tid);
    return tid;
}

} // пространство имен Игра


