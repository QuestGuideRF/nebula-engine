//------------------------------------------------------------------------------
//  schemeregistry.cc
//  (C) 2008 г. Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "io/schemeregistry.h"
#include "io/stream.h"
#include "io/filestream.h"
#include "http/httpnzstream.h"
#include "safefilestream.h"
#include "io/cache/cachedstreamtypes.h"
#include "io/embeddedmemorystream.h"

namespace IO
{
__ImplementClass(IO::SchemeRegistry, 'SCRG', Core::RefCounted);
__ImplementInterfaceSingleton(IO::SchemeRegistry);

using namespace Util;

//------------------------------------------------------------------------------
/**
*/
SchemeRegistry::SchemeRegistry() :
    isValid(false)
{
    __ConstructInterfaceSingleton;
}

//------------------------------------------------------------------------------
/**
*/
SchemeRegistry::~SchemeRegistry()
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
SchemeRegistry::Setup()
{
    this->critSect.Enter();

    n_assert(!this->IsValid());
    this->isValid = true;
    this->SetupStandardSchemes();
    
    this->critSect.Leave();
}

//------------------------------------------------------------------------------
/**
*/
void
SchemeRegistry::Discard()
{
    this->critSect.Enter();

    n_assert(this->IsValid());
    this->isValid = false;
    this->schemeRegistry.Clear();

    this->critSect.Leave();
}

//------------------------------------------------------------------------------
/**
*/
bool
SchemeRegistry::IsValid() const
{
    return this->isValid;
}

//------------------------------------------------------------------------------
/**
    Связывает схему URI с классом потока. Если тот же URI
    схема уже зарегистрирована, старая ассоциация будет перезаписана.
*/
void
SchemeRegistry::RegisterUriScheme(const String& uriScheme, const Core::Rtti& classRtti)
{
    this->critSect.Enter();

    n_assert(classRtti.IsDerivedFrom(Stream::RTTI));
    n_assert(uriScheme.IsValid());
    if (this->schemeRegistry.Contains(uriScheme))
    {
        this->UnregisterUriScheme(uriScheme);
    }
    this->schemeRegistry.Add(uriScheme, &classRtti);

    this->critSect.Leave();
}

//------------------------------------------------------------------------------
/**
*/
void
SchemeRegistry::UnregisterUriScheme(const String& uriScheme)
{
    this->critSect.Enter();

    n_assert(uriScheme.IsValid());
    n_assert(this->schemeRegistry.Contains(uriScheme));
    this->schemeRegistry.Erase(uriScheme);

    this->critSect.Leave();
}

//------------------------------------------------------------------------------
/**
*/
bool
SchemeRegistry::IsUriSchemeRegistered(const String& uriScheme) const
{
    this->critSect.Enter();
    n_assert(uriScheme.IsValid());
    bool result = this->schemeRegistry.Contains(uriScheme);
    this->critSect.Leave();
    return result;
}

//------------------------------------------------------------------------------
/**
*/
const Core::Rtti&
SchemeRegistry::GetStreamClassByUriScheme(const String& uriScheme) const
{
    this->critSect.Enter();
    n_assert(uriScheme.IsValid());
    const Core::Rtti& result = *(this->schemeRegistry[uriScheme]);
    this->critSect.Leave();
    return result;
}

//------------------------------------------------------------------------------
/**
*/
Array<String> 
SchemeRegistry::GetAllRegisteredUriSchemes() const
{
    this->critSect.Enter();
    Array<String> result = this->schemeRegistry.KeysAsArray();
    this->critSect.Leave();
    return result;
}

//------------------------------------------------------------------------------
/**
    Регистрирует стандартные схемы URI.
    ПРИМЕЧАНИЕ: частный метод, поэтому защита критической секции не требуется!
*/
void
SchemeRegistry::SetupStandardSchemes()
{
    this->RegisterUriScheme("file", FileStream::RTTI);
    this->RegisterUriScheme("embed", EmbeddedMemoryStream::RTTI);
    this->RegisterUriScheme("safefile", SafeFileStream::RTTI);
    this->RegisterUriScheme("http", IO::CachedHttpStream::RTTI);
#ifdef USE_CURL
    this->RegisterUriScheme("https", IO::CachedHttpStream::RTTI);
#endif
    this->RegisterUriScheme("httpnz", Http::HttpNzStream::RTTI);
}

} // пространство имен ввода-вывода
