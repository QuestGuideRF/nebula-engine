#pragma once
#ifndef DB_VALUETABLE_H
#define DB_VALUETABLE_H
#include "core/refcounted.h"
#include "attr/attributetable.h"
namespace Db
{
class ValueTable : public Attr::AttributeTable
{
    __DeclareClass(ValueTable);
public:
    ValueTable();
    virtual ~ValueTable();
};
}
#endif