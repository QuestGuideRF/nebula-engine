//------------------------------------------------------------------------------
//  attrid.cc
//  (C) 2006 г. Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "foundation/stdneb.h"
#include "attrid.h"

namespace Attr
{
//------------------------------------------------------------------------------
/**
    Возвращает массив со всеми существующими идентификаторами атрибутов. Это медленный
    метод.
*/
Util::FixedArray<AttrId>
AttrId::GetAllAttrIds()
{
    n_assert(0 != AttributeDefinitionBase::NameRegistry);

    // получить содержимое реестра атрибутов
    Util::StackArray<Util::KeyValuePair<Util::String, const Attr::AttributeDefinitionBase*>, 1> attrDefs = AttributeDefinitionBase::NameRegistry->Content();
    Util::FixedArray<AttrId> result(attrDefs.Size());
    IndexT i;
    SizeT num = attrDefs.Size();
    for (i = 0; i < num; i++)
    {
        AttrId attrId(*(attrDefs[i].Value()));
        result[i] = attrId;
    }
    return result;
}

} // Атрибут пространства имен
