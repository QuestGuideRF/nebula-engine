//------------------------------------------------------------------------------
//  skinhelper.cc
//  (C) 2012-2016 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "foundation/stdneb.h"
#include "skinhelper.h"

namespace ToolkitUtil
{
__ImplementClass(ToolkitUtil::SkinHelper, 'SKHE', Core::RefCounted);

//------------------------------------------------------------------------------
/**
*/
SkinHelper::SkinHelper()
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
SkinHelper::~SkinHelper()
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
void
SkinHelper::Setup(const Ptr<IO::XmlReader>& reader)
{
    if (reader->SetToFirstChild()) do 
    {
        if (reader->GetCurrentNodeName() != "Skin")
        {
            continue;
        }
        Util::String skinName = reader->GetString("name");

        this->skins.Append(skinName);
    } 
    while (reader->SetToNextChild());
}
} // пространство имен ToolkitUtil