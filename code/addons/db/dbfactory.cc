//------------------------------------------------------------------------------
//  factory.cc
//  (C) 2006 г. Радон Лабс ГмбХ
//  (C) 2013–2016 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "foundation/stdneb.h"
#include "db/dbfactory.h"
#include "db/database.h"
#include "db/command.h"
#include "db/table.h"
#include "db/dataset.h"
#include "db/reader.h"
#include "db/writer.h"
#include "db/relation.h"
#include "db/valuetable.h"

namespace Db
{
__ImplementClass(Db::DbFactory, 'DBFC', Core::RefCounted);
__ImplementSingleton(Db::DbFactory);

//------------------------------------------------------------------------------
/**
*/
DbFactory::DbFactory()
{
    __ConstructSingleton;
}

//------------------------------------------------------------------------------
/**
*/
DbFactory::~DbFactory()
{
    __DestructSingleton;
}

//------------------------------------------------------------------------------
/**
*/
Ptr<Database>
DbFactory::CreateDatabase() const
{
    n_error("Вызывается Db::DbFactory::CreateDatabase()!");
    return nullptr;
}

//------------------------------------------------------------------------------
/**
*/
Ptr<Command>
DbFactory::CreateCommand() const
{
    n_error("Вызывается Db::DbFactory::CreateCommand()!");
    return nullptr;
}

//------------------------------------------------------------------------------
/**
*/
Ptr<Table>
DbFactory::CreateTable() const
{
    n_error("Вызывается Db::DbFactory::CreateTable()!");
    return nullptr;
}

//------------------------------------------------------------------------------
/**
*/
Ptr<Dataset>
DbFactory::CreateDataset() const
{
    n_error("Вызывается Db::DbFactory::CreateDataset()!");
    return nullptr;
}

//------------------------------------------------------------------------------
/**
*/
Ptr<FilterSet>
DbFactory::CreateFilterSet() const
{
    n_error("Вызывается Db::DbFactory::CreateFilterSet()!");
    return nullptr;
}

//------------------------------------------------------------------------------
/**
*/
Ptr<Relation>
DbFactory::CreateRelation() const
{
    return Relation::Create();
}

//------------------------------------------------------------------------------
/**
*/
Ptr<ValueTable>
DbFactory::CreateValueTable() const
{
    return ValueTable::Create();
}

//------------------------------------------------------------------------------
/**
*/
Ptr<Reader>
DbFactory::CreateReader() const
{
    return Reader::Create();
}

//------------------------------------------------------------------------------
/**
*/
Ptr<Writer>
DbFactory::CreateWriter() const
{
    return Writer::Create();
}

} // пространство имен БД
