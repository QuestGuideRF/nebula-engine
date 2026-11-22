#pragma once
#include "core/refcounted.h"
#include "core/singleton.h"
#include "game/manager.h"
#include "util/stringatom.h"
#include "game/api.h"
#include "ids/idgenerationpool.h"
namespace Game
{
class World;
class BlueprintManager : public Game::Manager
{
    __DeclareClass(BlueprintManager)
    __DeclareSingleton(BlueprintManager)
public:
    struct Template
    {
        BlueprintId bid;
        MemDb::RowId row;
        Util::StringAtom name;
    };
    BlueprintManager();
    ~BlueprintManager();
    void OnActivate() override;
    void OnDeactivate() override;
    static void SetBlueprintsFilename(const Util::String& name, const Util::String& folder);
    static BlueprintId const GetBlueprintId(Util::StringAtom name);
    static TemplateId const GetTemplateId(Util::StringAtom name);
    static Util::StringAtom const GetTemplateName(TemplateId const templateId);
    static Util::Array<Template> const& ListTemplates();
    EntityMapping Instantiate(World* const world, BlueprintId blueprint);
    EntityMapping Instantiate(World* const world, TemplateId templateId);
private:
    bool ParseBlueprint(Util::String const& blueprintsPath);
    bool LoadTemplateFolder(Util::String const& path);
    bool ParseTemplate(Util::String const& templatePath);
    void SetupBlueprints();
    MemDb::TableId CreateCategory(World* const world, BlueprintId bid);
    struct ComponentEntry
    {
        Util::StringAtom name;
    };
    struct Blueprint
    {
        MemDb::TableId tableId;
        Util::StringAtom name;
        Util::Array<ComponentEntry> components;
    };
    Util::Array<Template> templates;
    Util::Array<Blueprint> blueprints;
    Ids::IdGenerationPool templateIdPool;
    Util::HashTable<Util::StringAtom, TemplateId> templateMap;
    Util::HashTable<Util::StringAtom, BlueprintId> blueprintMap;
    static Util::String blueprintFolder;
    static Util::String templatesFolder;
};
}