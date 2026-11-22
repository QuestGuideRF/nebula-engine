//------------------------------------------------------------------------------
//  projectinfo.cc
//  (C) 2008 г. Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "foundation/stdneb.h"
#include "projectinfo.h"
#include "io/assignregistry.h"
#include "io/ioserver.h"
#include "io/stream.h"
#include "io/jsonreader.h"
#include "system/nebulasettings.h"
#include "system/environment.h"
#include "app/application.h"
#include "util/commandlineargs.h"


namespace ToolkitUtil
{
using namespace Util;
using namespace IO;
using namespace System;

//------------------------------------------------------------------------------
/**
*/
ProjectInfo::ProjectInfo() 
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
ProjectInfo::~ProjectInfo()
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
bool
ProjectInfo::HasAttr(const Util::String& attrName) const
{
    return this->attrs.Contains(attrName);
}
//------------------------------------------------------------------------------
/**
*/
bool
ProjectInfo::HasListAttr(const Util::String& attrName) const
{
    return this->listAttrs.Contains(attrName);
}

//------------------------------------------------------------------------------
/**
*/
const Util::String&
ProjectInfo::GetAttr(const Util::String& attrName) const
{
    n_assert(attrName.IsValid());

    if (!this->HasAttr(attrName))
    {
        n_error("ProjectInfo: атрибут «%s» не существует в Projectinfo!",
            attrName.AsCharPtr());
    }
    return this->attrs[attrName];
}

//------------------------------------------------------------------------------
/**
*/
const Util::Dictionary<Util::String, Util::String>&
ProjectInfo::GetListAttr(const Util::String& attrName) const
{
    n_assert(attrName.IsValid());

    if (!this->HasListAttr(attrName))
    {
        n_error("ProjectInfo: атрибут «%s» не существует в Projectinfo!",
            attrName.AsCharPtr());
    }
    return this->listAttrs[attrName];
}

//------------------------------------------------------------------------------
/**
*/
bool
ProjectInfo::IsValid() const
{
    return (this->attrs.Size() > 0);
}

//------------------------------------------------------------------------------
/**
*/
ProjectInfo::Result
ProjectInfo::Setup()
{
    n_assert(!this->IsValid());

    String projPath;

    // проверьте переопределения cmdline для проекта
    const Util::CommandLineArgs & args = App::Application::Instance()->GetCmdLineArgs();

    if (args.HasArg("-workdir"))
    {
        projPath = args.GetString("-workdir");
    }
    else
    {
        // grab from registry instead (default)
        projPath = this->QueryProjectPathFromRegistry();
    }           
    if (projPath.IsEmpty())
    {
        return NoProjPathInRegistry;
    }
    AssignRegistry::Instance()->SetAssign(Assign("proj", projPath));

    // то же самое для набора инструментов: сначала переопределения, затем реестр
    String toolkitPath;
    if (args.HasArg("-toolkit"))
    {
        toolkitPath = args.GetString("-toolkit");
    }
    else
    {
        toolkitPath = this->QueryToolkitPathFromRegistry();
    }    
    n_assert(toolkitPath.IsValid());    
    AssignRegistry::Instance()->SetAssign(Assign("toolkit", toolkitPath));

    // анализ XML-файла с информацией о проекте
    Util::String projectPath = "proj:projectinfo.json";
    if (args.HasArg("-project"))
    {
        projectPath = args.GetString("-project");
        if (IoServer::Instance()->FileExists(projectPath))
        {
            projPath = projectPath.ExtractToLastSlash();
            AssignRegistry::Instance()->SetAssign(Assign("proj", projPath));
        }
    }

    Result res = ProjectInfo::Success;
    if (IoServer::Instance()->FileExists(projectPath))
    {
        res = this->ParseProjectInfoFile(URI(projectPath));
        this->AddAttrAssigns();
    }
    else
    {
        return NoProjectInfoFile;
    }
    return res;
}

//------------------------------------------------------------------------------
/**
*/
void
ProjectInfo::AddAttrAssigns()
{
    for (auto const& attr : this->attrs)
    {
        AssignRegistry::Instance()->SetAssign(Assign(attr.Key(), attr.Value()));
    }
}
//------------------------------------------------------------------------------
/**
*/
void
ProjectInfo::Discard()
{
    this->attrs.Clear();
    this->listAttrs.Clear();
}

//------------------------------------------------------------------------------
/**
    Разберите файл projectinfo.xml, который ожидается в корне проекта. 
    каталог.
*/
ProjectInfo::Result
ProjectInfo::ParseProjectInfoFile(const IO::URI & path)
{
    n_assert(!this->IsValid());

    Ptr<Stream> stream = IoServer::Instance()->CreateStream(path);
    Ptr<JsonReader> jsonReader = JsonReader::Create();
    jsonReader->SetStream(stream);
    if (jsonReader->Open())
    {

        if (jsonReader->SetToFirstChild()) do
        {
            if (!jsonReader->HasChildren())
            {
                this->attrs.Add(jsonReader->GetCurrentNodeName(), jsonReader->GetString());
            }
            else
            {
                String currentKey = jsonReader->GetCurrentNodeName();
                Dictionary<String, String> values;
                jsonReader->SetToFirstChild();
                do
                {
                    values.Add(jsonReader->GetString("Name"), jsonReader->GetString("Value"));
                } 
                while (jsonReader->SetToNextChild());
                this->listAttrs.Add(currentKey, values);
            }
        } 
        while (jsonReader->SetToNextChild());
        return Success;
    }
    else
    {
        return CouldNotOpenProjectInfoFile;
    }
}

//------------------------------------------------------------------------------
/**
    Query the project path from the registry, the registry key is set
    by the "Nebula2 Toolkit For Maya". If no key is found, the method
    will return "home:".
*/
String
ProjectInfo::QueryProjectPathFromRegistry()
{
    String projDirectory;
    if (NebulaSettings::Exists("gscept","ToolkitShared", "workdir"))
    {
        projDirectory = NebulaSettings::ReadString("gscept","ToolkitShared", "workdir");
    }
    else
    {
        projDirectory = "home:";
    }
    return projDirectory;
}

//------------------------------------------------------------------------------
/**
    Query the toolkit path from the registry, the registry key is set
    by the "Nebula2 Toolkit For Maya". If no key is found, the method
    will return "home:".
*/
String
ProjectInfo::QueryToolkitPathFromRegistry()
{
    String toolkitDirectory;

    if (NebulaSettings::Exists("gscept","ToolkitShared", "path"))
    {
        toolkitDirectory = NebulaSettings::ReadString("gscept","ToolkitShared", "path");
    }
    else
    {
        toolkitDirectory = "home:";
    }
    return toolkitDirectory;
}

//------------------------------------------------------------------------------
/**
*/
String
ProjectInfo::GetErrorString(Result res) const
{
    switch (res)
    {
        case ProjectInfo::NoProjPathInRegistry:
            return String("ОШИБКА: в реестре не найдено записей набора инструментов Nebula!");
        case ProjectInfo::CouldNotOpenProjectInfoFile:
            return String("ОШИБКА: не удалось открыть файл projectinfo.xml в каталоге проекта!");
        case ProjectInfo::ProjectFileContentInvalid:
            return String("ОШИБКА: содержимое файла projectinfo.xml неверно!");
        case ProjectInfo::NoProjectInfoFile:
            return String("ОШИБКА: В текущем проекте не найден файл projectinfo.xml!");
        default:
            return String("ОШИБКА: неизвестная ошибка объекта ProjectInfo!\n");
    }
}

//------------------------------------------------------------------------------
/**
    Интерпретирует значение атрибута как путь, который может содержать реестр.
    или псевдонимы среды в следующей форме:

    $(reg:HKEY_LOCAL_MACHINE/bla/blob)/bla/bla
    $(env:HOME_DIR)/бла/бла
*/
String
ProjectInfo::GetPathAttr(const String& attrName) const
{
    String attrVal = this->GetAttr(attrName);
    if ((attrVal.Length() > 0) && (attrVal[0] == '$'))
    {
        // необходимо разрешить переменную env или reg
        IndexT varStartIndex = attrVal.FindCharIndex('(');
        IndexT varEndIndex   = attrVal.FindCharIndex(')');
        n_assert(varStartIndex == 1);
        n_assert(varEndIndex > varStartIndex);
        String varString = attrVal.ExtractRange(varStartIndex + 1, (varEndIndex - varStartIndex) - 1);

#if __WIN32__
        // переменная реестра или среды?
        if (String::MatchPattern(varString, "рег:*"))
        {
            // рассматривать как ключ реестра
            String regString = varString.ExtractToEnd(4);
            Array<String> regTokens = regString.Tokenize("/");
            n_assert(regTokens.Size() > 2);
            Win32::Win32Registry::RootKey rootKey = Win32::Win32Registry::AsRootKey(regTokens[0]);
            String regKeyName = regTokens.Back();
            regTokens.EraseIndex(0);
            regTokens.EraseIndex(regTokens.Size() - 1);
            String regPath = String::Concatenate(regTokens, "\\");

            // прочитать фактическое значение реестра
            String regValue = Win32::Win32Registry::ReadString(rootKey, regPath, regKeyName);

            // создать фактическое имя файла
            String retValue = regValue;
            retValue.Append(attrVal.ExtractToEnd(varEndIndex + 1));
            return retValue;
        }
        else if (String::MatchPattern(varString, "окружение:*"))
        {
            // переменная среды
            String envString = varString.ExtractToEnd(4);
            if (Win32::Win32Environment::Exists(envString))
            {
                String envValue = Win32::Win32Environment::Read(envString);
                String retValue = envValue;
                retValue.Append(attrVal.ExtractToEnd(varEndIndex + 1));
                return retValue;
            }
        }
#endif
    }
    
    // провал: ошибка формата, возвращаемая строка как есть
    return attrVal;
}

//------------------------------------------------------------------------------
/**    
*/
ProjectInfo
ProjectInfo::SetupFromFile(const String& file)
{
    ProjectInfo pi;
    URI uri(file);
    if(IO::IoServer::Instance()->FileExists(uri))
    {
        pi.ParseProjectInfoFile(uri);
    }
    return pi;
}

} // пространство имен ToolkitUtil