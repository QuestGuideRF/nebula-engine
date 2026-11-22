//------------------------------------------------------------------------------
//  shaderidentifier.cc
//  (C) 2015–2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "coregraphics/shaderidentifier.h"
#include "shaderserver.h"

namespace CoreGraphics
{

using namespace Util;

//------------------------------------------------------------------------------
/**
    Частный конструктор, только ShaderServer может создавать центральный 
    Реестр ShaderIdentifier.
*/
ShaderIdentifier::ShaderIdentifier()
{
    this->nameToCode.Reserve(MaxNumShaderIdentifiers);
    this->codeToName.Reserve(MaxNumShaderIdentifiers);
}

//------------------------------------------------------------------------------
/**
*/
ShaderIdentifier::Code
ShaderIdentifier::FromName(const Name& name)
{
    ShaderIdentifier& registry = ShaderServer::Instance()->shaderIdentifierRegistry;
    IndexT index = registry.nameToCode.FindIndex(name);
    if (InvalidIndex != index)
    {
        return registry.nameToCode.ValueAtIndex(index);
    }
    else
    {
        // имя еще не зарегистрировано
        registry.codeToName.Append(name);
        Code code = registry.codeToName.Size() - 1;
        registry.nameToCode.Add(name, code);
        return code;
    }
}

//------------------------------------------------------------------------------
/**
*/
ShaderIdentifier::Name
ShaderIdentifier::ToName(Code c)
{
    ShaderIdentifier& registry = ShaderServer::Instance()->shaderIdentifierRegistry;
    return registry.codeToName[c];
}

} // Модели пространства имен
