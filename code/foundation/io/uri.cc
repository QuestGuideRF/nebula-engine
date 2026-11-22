//------------------------------------------------------------------------------
//  uri.cc
//  (C) 2006 г. Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "io/uri.h"
#include "io/assignregistry.h"
#include "io/fswrapper.h"
#include "core/config.h"


//------------------------------------------------------------------------------
/**
    Литеральная строка формы конструктора, для использования "foobar"_uri автоматически создаст IO::URI.
*/
IO::URI
operator ""_uri(const char* c, std::size_t s)
{
    return IO::URI(c);
}

namespace IO
{
using namespace Util;

//------------------------------------------------------------------------------
/**
    Resolve assigns and split URI string into its components.

    @todo: this is too complicated...
*/
bool
URI::Split(const String& s)
{
    n_assert(s.IsValid());
    this->Clear();
    this->isEmpty = false;
    
    // разрешить присваивает
    String str;
    if (AssignRegistry::HasInstance())
    {
        str = AssignRegistry::Instance()->ResolveAssignsInString(s);
    }
    else
    {
        str = s;
    }

    // Схема является первым компонентом и заканчивается:
    IndexT schemeColonIndex = str.FindCharIndex(':');
    String potentialScheme;
    bool schemeIsDevice = false;
    if (InvalidIndex != schemeColonIndex)
    {
        potentialScheme = str.ExtractRange(0, schemeColonIndex);
        if (FSWrapper::IsDeviceName(potentialScheme))
        {
            // либо схемы не дано, либо "схема"
            // на самом деле это имя устройства
            // в обоих случаях мы возвращаемся к схеме по умолчанию «файл» и
            // просто установите всю строку как локальный путь, не будет никаких
            // другие компоненты
            schemeIsDevice = true;
        }
    }
    if ((InvalidIndex == schemeColonIndex) || schemeIsDevice)
    {
        this->SetScheme(DEFAULT_IO_SCHEME);
        this->SetLocalPath(str);
        return true;
    }

    // проверьте, предоставлена ​​ли действующая схема
    if (InvalidIndex != schemeColonIndex)
    {
        // дана действующая схема
        this->SetScheme(potentialScheme);

        // после схемы и перед хостом должна быть двойная косая черта
        if (!((str[schemeColonIndex + 1] == '/') && (str[schemeColonIndex + 2] == '/')))
        {
            return false;
        }
    }

    // извлечь компоненты UserInfo, Host и Port
    IndexT hostStartIndex = schemeColonIndex + 3;
    IndexT hostEndIndex = str.FindCharIndex('/', hostStartIndex);
    String userInfoHostPort;
    String path;
    if (InvalidIndex == hostEndIndex)
    {
        userInfoHostPort = str.ExtractToEnd(hostStartIndex);
    }
    else
    {
        userInfoHostPort = str.ExtractRange(hostStartIndex, hostEndIndex - hostStartIndex);
        path = str.ExtractToEnd(hostEndIndex + 1);
    }

    // извлечь номер порта, если он существует
    IndexT portIndex = userInfoHostPort.FindCharIndex(':');
    IndexT atIndex = userInfoHostPort.FindCharIndex('@');
    if (InvalidIndex != portIndex)
    {
        if (InvalidIndex != atIndex)
        {
            n_assert(portIndex > atIndex);
        }
        this->SetPort(userInfoHostPort.ExtractToEnd(portIndex + 1));
        userInfoHostPort.TerminateAtIndex(portIndex);
    }
    if (InvalidIndex != atIndex)
    {
        this->SetHost(userInfoHostPort.ExtractToEnd(atIndex + 1));
        userInfoHostPort.TerminateAtIndex(atIndex);
        this->SetUserInfo(userInfoHostPort);
    }
    else
    {
        this->SetHost(userInfoHostPort);
    }

    // разделить часть пути на компоненты
    if (path.IsValid())
    {
        IndexT fragmentIndex = path.FindCharIndex('#');
        IndexT queryIndex = path.FindCharIndex('?');
        if (InvalidIndex != queryIndex)
        {
            if (InvalidIndex != fragmentIndex)
            {
                n_assert(queryIndex > fragmentIndex);
            }
            this->SetQuery(path.ExtractToEnd(queryIndex + 1));
            path.TerminateAtIndex(queryIndex);
        }
        if (InvalidIndex != fragmentIndex)
        {
            this->SetFragment(path.ExtractToEnd(fragmentIndex + 1));
            path.TerminateAtIndex(fragmentIndex);
        }
        this->SetLocalPath(path);
    }
    return true;
}

//------------------------------------------------------------------------------
/**
    Это создает строку URI из ее компонентов.
*/
String
URI::Build() const
{
    n_assert(!this->IsEmpty());

    String str;
    str.Reserve(256);
    if (this->scheme.IsValid())
    {
        str.Append(this->scheme);
        str.Append("://");
    }
    if (this->userInfo.IsValid())
    {
        str.Append(this->userInfo);
        str.Append("@");
    }
    if (this->host.IsValid())
    {
        str.Append(this->host);
    }
    if (this->port.IsValid())
    {
        str.Append(":");
        str.Append(this->port);
    }
    if (this->localPath.IsValid())
    {
        str.Append("/");
        str.Append(this->localPath);
    }
    if (this->fragment.IsValid())
    {
        str.Append("#");
        str.Append(this->fragment);
    }
    if (this->query.IsValid())
    {
        str.Append("?");
        str.Append(this->query);
    }
    return str;
}

//------------------------------------------------------------------------------
/**
    This returns the "tail", which is the local path, the fragment and
    the query concatenated into one string.
*/
String
URI::GetTail() const
{   
    String str;
    str.Reserve(256);
    if (this->localPath.IsValid())
    {
        str.Append(this->localPath);
    }
    if (this->fragment.IsValid())
    {
        str.Append("#");
        str.Append(this->fragment);
    }
    if (this->query.IsValid())
    {
        str.Append("?");
        str.Append(this->query);
    }
    return str;
}

//------------------------------------------------------------------------------
/**
    Returns the host and local path in the form "//host/localpath".
    If no host has been set, only "/localpath" will be returned.
*/
String
URI::GetHostAndLocalPath() const
{
    String str;
    str.Reserve(this->host.Length() + this->localPath.Length() + 8);
    if (this->host.IsValid())
    {
        str.Append("//");
        str.Append(this->host);
        str.Append("/");
    }
    str.Append(this->localPath);
    return str;
}

//------------------------------------------------------------------------------
/**
    Добавляет элемент к локальному пути. Автоматически вставляет
    разделитель пути "/".
*/
void
URI::AppendLocalPath(const String& pathComponent)
{
    n_assert(pathComponent.IsValid());
    this->localPath.Append("/");
    this->localPath.Append(pathComponent);
}

//------------------------------------------------------------------------------
/**
    Это анализирует часть запроса URI (в форме
    param1=value&param2=value&param3=value ...) в словарь. Плохо отформатирован
    фрагменты запроса будут игнорироваться.
*/
Dictionary<String,String>
URI::ParseQuery() const
{
    Dictionary<String,String> result;
    Array<String> keyValuePairs = this->query.Tokenize("&");
    IndexT i;
    for (i = 0; i < keyValuePairs.Size(); i++)
    {
        Array<String> keyValueTokens = keyValuePairs[i].Tokenize("=");
        if (keyValueTokens.Size() == 2)
        {
            result.Add(keyValueTokens[0], keyValueTokens[1]);
        }
    }
    return result;
}

} // пространство имен ввода-вывода
