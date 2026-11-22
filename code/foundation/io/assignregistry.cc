//------------------------------------------------------------------------------
//  assignregistry.cc
//  (C) 2008 г. Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "io/assignregistry.h"
#include "io/fswrapper.h"
#include "core/coreserver.h"
#include "system/nebulasettings.h"

namespace IO
{
__ImplementClass(IO::AssignRegistry, 'ASRG', Core::RefCounted);
__ImplementInterfaceSingleton(IO::AssignRegistry);

using namespace Util;

//------------------------------------------------------------------------------
/**
*/
AssignRegistry::AssignRegistry() :
    isValid(false)
{
    __ConstructInterfaceSingleton;
}

//------------------------------------------------------------------------------
/**
*/
AssignRegistry::~AssignRegistry()
{
    if (this->IsValid())
    {
        this->Discard();
    }
    __DestructInterfaceSingleton;
}

//------------------------------------------------------------------------------
/**
*/
void
AssignRegistry::Setup()
{
    this->critSect.Enter();

    n_assert(!this->IsValid());
    this->isValid = true;
    this->SetupSystemAssigns();
    this->SetupProjectAssigns();
    
    this->critSect.Leave();
}

//------------------------------------------------------------------------------
/**
*/
void
AssignRegistry::Discard()
{
    this->critSect.Enter();
    
    n_assert(this->IsValid());
    this->assignTable.Clear();
    this->isValid = false;
    
    this->critSect.Leave();
}

//------------------------------------------------------------------------------
/**
*/
bool
AssignRegistry::IsValid() const
{
    return this->isValid;
}

//------------------------------------------------------------------------------
/**
*/
void
AssignRegistry::SetupSystemAssigns()
{
    this->critSect.Enter();

    n_assert(this->IsValid()); 
    
    String homeLocation = FSWrapper::GetHomeDirectory();
    if (homeLocation.IsValid())
    {
        this->SetAssign(Assign("home", homeLocation));
    }

    String binLocation = FSWrapper::GetBinDirectory();
    if (binLocation.IsValid())
    {
        this->SetAssign(Assign("bin", binLocation));
    }
    String tempLocation = FSWrapper::GetTempDirectory();
    if (tempLocation.IsValid())
    {
        this->SetAssign(Assign("temp", tempLocation));
    }
    String userLocation = FSWrapper::GetUserDirectory();
    if (userLocation.IsValid())
    {
        this->SetAssign(Assign("user", userLocation));
    }
    
    #if __WIN32__
    String appDataLocation = FSWrapper::GetAppDataDirectory();
    if (appDataLocation.IsValid())
    {
        this->SetAssign(Assign("appdata", appDataLocation));
    }
    String programsLocation = FSWrapper::GetProgramsDirectory();
    if (programsLocation.IsValid())
    {
        this->SetAssign(Assign("programs", programsLocation));
    }
    #endif

    this->critSect.Leave();
}    

//------------------------------------------------------------------------------
/**
*/
void
AssignRegistry::SetupProjectAssigns()
{
    this->critSect.Enter();
        
    n_assert(this->IsValid());

    String rootDir = Core::CoreServer::Instance()->GetRootDirectory().AsString();
    this->SetAssign(Assign("root", rootDir));
    this->SetAssign(Assign("export", "root:export"));
    
    // настройка содержимого назначает
    this->SetAssign(Assign("msh",   "export:meshes"));
    this->SetAssign(Assign("ani",   "export:anims"));
    this->SetAssign(Assign("ske",   "export:skeletons"));
#if PUBLIC_BUILD
    this->SetAssign(Assign("data",  "export:data"));        
#else
    this->SetAssign(Assign("data", "root:data"));
#endif
    this->SetAssign(Assign("video", "export:video"));
    this->SetAssign(Assign("db",    "export:db"));
    this->SetAssign(Assign("audio", "export:audio"));
    this->SetAssign(Assign("shd",   "export:shaders"));
    this->SetAssign(Assign("tex",   "export:textures"));
    this->SetAssign(Assign("frame", "export:frame"));
    this->SetAssign(Assign("mdl",   "export:models"));
    this->SetAssign(Assign("phys",  "export:physics"));
    this->SetAssign(Assign("audio", "export:audio"));    
    this->SetAssign(Assign("mat",   "export:materials"));
    this->SetAssign(Assign("sur",   "export:surfaces"));
    this->SetAssign(Assign("scr",   "data:scripts"));
    this->SetAssign(Assign("gui",   "data:gui"));
    this->SetAssign(Assign("tbl", "export:data/tables"));
    
    // назначить для навигационных сеток
    this->SetAssign(Assign("nav",   "data:navigation"));

    // setup special system assigns (for placeholders, etc...)
    this->SetAssign(Assign("sysmsh",    "msh:system"));
    this->SetAssign(Assign("systex",    "tex:system"));
    this->SetAssign(Assign("sysani",    "ani:system"));
    this->SetAssign(Assign("sysmdl",    "mdl:system"));
    this->SetAssign(Assign("syssur",    "sur:system"));
    this->SetAssign(Assign("sysphys",   "phys:system"));

    this->SetAssign(Assign("config", "data:config"));

    // инструменты назначает
    this->SetAssign(Assign("tool", Core::CoreServer::Instance()->GetToolDirectory().AsString()));    
    // назначение проекта из реестра
    if (System::NebulaSettings::Exists("gscept", "ToolkitShared", "workdir"))
    {
        this->SetAssign(Assign("proj", System::NebulaSettings::ReadString("gscept", "ToolkitShared", "workdir")));
    }
    else
    {
        this->SetAssign(Assign("proj", "root:"));
    }
    

    this->critSect.Leave();
}

//------------------------------------------------------------------------------
/**
*/
void
AssignRegistry::SetAssign(const Assign& assign)
{
    this->critSect.Enter();

    n_assert(this->IsValid());
    if (this->HasAssign(assign.GetName()))
    {
        this->ClearAssign(assign.GetName());
    }
    this->assignTable.Add(assign);
    
    this->critSect.Leave();
}

//------------------------------------------------------------------------------
/**
*/
bool
AssignRegistry::HasAssign(const String& assignName) const
{
    this->critSect.Enter();
    n_assert(assignName.IsValid());
    bool result = this->assignTable.Contains(assignName);
    this->critSect.Leave();
    return result;
}

//------------------------------------------------------------------------------
/**
*/
String
AssignRegistry::GetAssign(const String& assignName) const
{
    this->critSect.Enter();

    n_assert(assignName.IsValid());
    n_assert(this->HasAssign(assignName));
    String result = this->assignTable[assignName];
    
    this->critSect.Leave();
    return result;
}

//------------------------------------------------------------------------------
/**
*/
void
AssignRegistry::ClearAssign(const String& assignName)
{
    this->critSect.Enter();

    n_assert(assignName.IsValid());
    n_assert(this->HasAssign(assignName));
    this->assignTable.Erase(assignName);

    this->critSect.Leave();
}

//------------------------------------------------------------------------------
/**
*/
Array<Assign>
AssignRegistry::GetAllAssigns() const
{
    this->critSect.Enter();

    StackArray<KeyValuePair<String,String>, 1> content = this->assignTable.Content();
    Array<Assign> assigns(content.Size(), 0);
    IndexT i;
    for (i = 0; i < content.Size(); i++)
    {
        assigns.Append(Assign(content[i].Key(), content[i].Value()));
    }

    this->critSect.Leave();
    return assigns;
}

//------------------------------------------------------------------------------
/**
    Разрешает все назначения из URI, возвращая URI. Разрешено
    «стек» присваивает, что означает определение присваивания как указывающего на
    другое назначение.
*/
URI
AssignRegistry::ResolveAssigns(const URI& uri) const
{
    this->critSect.Enter();
    URI resolvedUri = this->ResolveAssignsInString(uri.AsString());
    this->critSect.Leave();
    return resolvedUri;
}

//------------------------------------------------------------------------------
/**
    Разрешает все назначения из URI. Разрешено
    «стек» присваивает, что означает определение присваивания как указывающего на
    еще одно назначение.
*/
String
AssignRegistry::ResolveAssignsInString(const String& uriString) const
{
    this->critSect.Enter();
    String result = uriString;
    
    // проверить назначения
    int colonIndex;
    while ((colonIndex = result.FindCharIndex(':', 0)) > 0)
    {
        // особый случай: игнорируйте односимвольные «назначения», потому что они 
        // действительно буквы диска DOS
        if (colonIndex > 1)
        {
            String assignString = result.ExtractRange(0, colonIndex);
            
            // игнорировать неизвестные назначения, поскольку они могут быть частью URI
            if (this->HasAssign(assignString))
            {
                String postAssignString = result.ExtractRange(colonIndex + 1, result.Length() - (colonIndex + 1));
                String replace = this->GetAssign(assignString);
                if (!replace.IsEmpty())
                {
                    if (replace[replace.Length()-1] != ':'
                        && (replace[replace.Length()-1] != '/'
                        || replace[replace.Length()-2] != ':'))
                    {
                        replace.Append("/");
                    }
                    replace.Append(postAssignString);
                }
                result = replace;
            }
            else break;
        }
        else break;
    }
    result.ConvertBackslashes();
    result.TrimRight("/");
    this->critSect.Leave();
    return result;
}

//------------------------------------------------------------------------------
/**
*/
void
AssignRegistry::PrintAll() const
{
    this->critSect.Enter();
    auto keys = this->assignTable.KeysAsArray();
    for (int i = 0; i < keys.Size(); i++)
    {
        Util::String assign = this->assignTable[keys[i]];
        Util::String path = IO::URI(this->assignTable[keys[i]]).LocalPath();
        n_printf("%-10s  =>   %-24s  =>  %s\n", (keys[i] + ":").AsCharPtr(), assign.AsCharPtr(), path.AsCharPtr());
    }
    this->critSect.Leave();
}

} // пространство имен ввода-вывода
