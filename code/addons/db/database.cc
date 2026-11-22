//------------------------------------------------------------------------------
//  database.cc
//  (C) 2006 г. Радон Лабс ГмбХ
//  (C) 2013–2016 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "foundation/stdneb.h"
#include "db/database.h"


namespace Attr
{
    DefineString(AttrName, 'ATNA', Attr::ReadWrite);
    DefineString(AttrType, 'ATTY', Attr::ReadWrite);
    DefineBool(AttrReadWrite, 'ATRW', Attr::ReadWrite);
    DefineBool(AttrDynamic, 'ATDY', Attr::ReadWrite);
}

namespace Db
{
__ImplementClass(Db::Database, 'DBAS', Core::RefCounted);

//------------------------------------------------------------------------------
/**
*/
Database::Database() :        
    isOpen(false),
    ignoreUnknownColumns(false),
    accessMode(ReadWriteExisting),
    memoryDatabase(false),
    exclusiveMode(false)
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
Database::~Database()
{
    n_assert(!this->IsOpen());
}

//------------------------------------------------------------------------------
/**
    Open the database. A subclass should connect to the database here
    and create a DataTable object for each table in the database.
*/
bool
Database::Open()
{
    n_assert(!this->IsOpen());
    this->isOpen = true;
    return true;
}

//------------------------------------------------------------------------------
/**
    Close the database. A subclass should flush all pending changes
    and disconnect from the database.
*/
void
Database::Close()
{
    n_assert(this->IsOpen());

    // очистить и отключить таблицы
    int tableIndex;
    int numTables = this->tables.Size();
    for (tableIndex = 0; tableIndex < numTables; tableIndex++)
    {
        this->tables[tableIndex]->Disconnect(false);
    }
    this->tables.Clear();
    this->isOpen = false;
}

//------------------------------------------------------------------------------
/**
    Начать транзакцию в базе данных. Переопределить этот метод в подклассе.
*/
void
Database::BeginTransaction()
{
    n_error("Вызывается Database::BeginTransaction()!");
}

//------------------------------------------------------------------------------
/**
    Завершите транзакцию в базе данных. Переопределить этот метод в подклассе.
*/
void
Database::EndTransaction()
{
    n_error("Вызывается Database::EndTransaction()!");
}   

//------------------------------------------------------------------------------
/**
    Add a table to the database. No table of the same name may exist
    yet in the database, otherwise an error will be thrown. If the table
    has values, the new database table will be initialized with those
    values. The table object will be connected to the database, which
    means any later changes to the table may be written to the database
    by calling the CommitChanges() method on the table.
 */
void
Database::AddTable(const Ptr<Table>& table)
{
    n_assert(this->IsOpen());
    n_assert(0 != table);
    n_assert(!this->HasTable(table->GetName()));
    this->tables.Append(table);
    table->Connect(this, Table::ForceCreate);
}

//------------------------------------------------------------------------------
/**
    Remove a data table from the database. A subclass must drop the table
    from the database when this method is called.
*/
void
Database::DeleteTable(const Util::String& tableName)
{
    IndexT tableIndex = this->FindTableIndex(tableName);
    n_assert(InvalidIndex != tableIndex);
    this->tables[tableIndex]->Disconnect(true);
    this->tables.EraseIndex(tableIndex);
}

//------------------------------------------------------------------------------
/**
    ПРИМЕЧАНИЕ. мы не можем использовать более эффективный контейнер для таблиц.
    потому что имя таблицы может измениться в любое время вне нашего контроля.
*/
IndexT
Database::FindTableIndex(const Util::String& tableName) const
{
    n_assert(tableName.IsValid());
    n_assert(this->IsOpen());
    IndexT i;
    for (i = 0; i < this->tables.Size(); i++)
    {
        if (this->tables[i]->GetName() == tableName)
        {
            return i;
        }
    }
    return InvalidIndex;
}

//------------------------------------------------------------------------------
/**
    ПРИМЕЧАНИЕ. мы не можем использовать более эффективный контейнер для таблиц.
    потому что имя таблицы может измениться в любое время вне нашего контроля.
*/
bool
Database::HasTable(const Util::String& tableName) const
{
    return (InvalidIndex != this->FindTableIndex(tableName));
}

//------------------------------------------------------------------------------
/**
    ПРИМЕЧАНИЕ. мы не можем использовать более эффективный контейнер для таблиц.
    потому что имя таблицы может измениться в любое время вне нашего контроля.
*/
SizeT
Database::GetNumTables() const
{
    n_assert(this->IsOpen());
    return this->tables.Size();
}

//------------------------------------------------------------------------------
/**
    ПРИМЕЧАНИЕ. мы не можем использовать более эффективный контейнер для таблиц.
    потому что имя таблицы может измениться в любое время вне нашего контроля.
*/
const Ptr<Table>&
Database::GetTableByIndex(IndexT i) const
{
    n_assert(this->IsOpen());
    return this->tables[i];
}

//------------------------------------------------------------------------------
/**
    ПРИМЕЧАНИЕ. мы не можем использовать более эффективный контейнер для таблиц.
    потому что имя таблицы может измениться в любое время вне нашего контроля.
*/
const Ptr<Table>&
Database::GetTableByName(const Util::String& tableName) const
{
    IndexT idx = this->FindTableIndex(tableName);
    if(idx == InvalidIndex)
    {               
        n_error("Недопустимое имя таблицы в базе данных: %s\ndвы забыли файл шаблона?\n",tableName.AsCharPtr());
    }
    
    return this->tables[idx];
}

//------------------------------------------------------------------------------
/**
    Присоедините к этой базе данных другую базу данных. Переопределить этот метод в 
    подкласс!
*/
bool
Database::AttachDatabase(const IO::URI& /*uri*/, const Util::String& /*dbName*/)
{
    n_error("Вызывается база данных::AttachDatabase!");
    return false;
}

//------------------------------------------------------------------------------
/**
    Отсоедините присоединенную базу данных от этой базы данных. Переопределить этот метод в 
    подкласс!
*/
void
Database::DetachDatabase(const Util::String& /*dbName*/)
{
    n_error("Вызывается Database::DetachDatabase()!");
}

//------------------------------------------------------------------------------
/**
    Скопируйте базу данных в файл. Переопределить этот метод в 
    подкласс!
*/
void
Database::CopyInMemoryDatabaseToFile(const IO::URI& fileUri)
{
    n_error("Вызывается Database::DetachDatabase()!");        
}

} // пространство имен БД
