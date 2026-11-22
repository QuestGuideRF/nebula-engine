//------------------------------------------------------------------------------
//  writer.cc
//  (C) 2006 г. Радон Лабс ГмбХ
//  (C) 2013–2016 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "foundation/stdneb.h"
#include "db/writer.h"
#include "db/dbserver.h"
#include "db/database.h"
#include "db/dbfactory.h"

namespace Db
{
__ImplementClass(Db::Writer, 'DBWR', Core::RefCounted);

//------------------------------------------------------------------------------
/**
*/
Writer::Writer() :
    isOpen(false),
    inBeginRow(false),
    flushTable(false),
    rowIndex(InvalidIndex)
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
Writer::~Writer()
{
    if (this->IsOpen())
    {
        this->Close();
    }
}

//------------------------------------------------------------------------------
/**
    Это открывает писателя.
*/
bool
Writer::Open()
{
    n_assert(!this->isOpen);
    n_assert(!this->inBeginRow);

    // создать новую таблицу значений
    this->valueTable = DbFactory::Instance()->CreateValueTable();
    IndexT i;
    for (i = 0; i < this->columns.Size(); i++)
    {
        this->valueTable->AddColumn(this->columns[i].GetAttrId(), false);
    }
    this->isOpen = true;
    return true;
}

//------------------------------------------------------------------------------
/**
    This opens the writer from an existing value table.
    Note: you still need to add primary and/or indexed columns,
    all others will be taken from the value table.
*/
bool
Writer::OpenFromValueTable(const Ptr<ValueTable>& srcValues)
{
    n_assert(!this->isOpen);
    n_assert(!this->inBeginRow);
    
    this->valueTable = DbFactory::Instance()->CreateValueTable();

    // скопировать таблицу значений src в таблицу столбцов записи
    IndexT i;
    for (i = 0; i < srcValues->GetNumColumns(); i++)
    {
        if (!this->columnMap.Contains(srcValues->GetColumnId(i)))
        {
            this->AddColumn(Column(srcValues->GetColumnId(i)));
        }
    }

    // обновить столбцы таблицы значений
    for (i = 0; i < this->columns.Size(); i++)
    {
        this->valueTable->AddColumn(this->columns[i].GetAttrId(), false);
    }

    // наконец, скопируйте строки исходных значений в таблицу назначения
    IndexT srcRowIndex = 0;
    for (srcRowIndex = 0; srcRowIndex < srcValues->GetNumRows(); srcRowIndex++)
    {
        this->valueTable->CopyExtRow(srcValues, srcRowIndex);
    }

    this->isOpen = true;
    return true;
}

//------------------------------------------------------------------------------
/**
    Закройте автора. Здесь будет происходить обновление базы данных.
*/
void
Writer::Close()
{
    n_assert(this->isOpen);
    n_assert(!this->inBeginRow);
    n_assert(this->database.isvalid());

    // удалить таблицу, если потребуется
    if (this->flushTable)
    {
        if (this->database->HasTable(this->tableName))
        {
            this->database->DeleteTable(this->tableName);
        }
    }

    // проверьте, должна ли быть создана новая таблица в базе данных
    bool tableWasCreated = false;
    Ptr<Table> table;
    if (this->flushTable || (!this->database->HasTable(this->tableName)))
    {
        table = DbFactory::Instance()->CreateTable();
        table->SetName(tableName);
        tableWasCreated = true;
    }
    else
    {
        table = this->database->GetTableByName(this->tableName);
    }

    // обновить столбцы таблицы
    IndexT colIndex;
    for (colIndex = 0; colIndex < this->columns.Size(); colIndex++)
    {
        if (!table->HasColumn(this->columns[colIndex].GetAttrId()))
        {
            table->AddColumn(this->columns[colIndex]);
        }
    }

    // добавить таблицу в базу данных, если она была создана
    if (tableWasCreated)
    {
        this->database->AddTable(table);
    }

    // if the number of columns in our value table is smaller then
    // количество столбцов таблицы, то это обновляет всего лишь 
    // часть таблицы, в этом случае новые строки должны быть
    // рассматриваются как измененные строки, поэтому вместо операции INSERT SQL
    // необходимо использовать операцию UPDATE
    if (this->valueTable->GetNumColumns() < table->GetNumColumns())
    {
        this->valueTable->ClearNewRowFlags();
    }

    // привязать таблицу значений к таблице и зафиксировать изменения в базе данных
    table->BindValueTable(this->valueTable);
    table->CommitChanges();
    table->UnbindValueTable();

    // наконец освободите таблицу значений и вернитесь
    this->valueTable = nullptr;
    this->database = nullptr;
    this->isOpen = false;
}

//------------------------------------------------------------------------------
/**
    Начните запись новой строки в базу данных.
*/
void
Writer::BeginRow()
{
    n_assert(this->isOpen);
    n_assert(!this->inBeginRow);
    n_assert(this->valueTable.isvalid());
    this->inBeginRow = true;
    this->rowIndex = this->valueTable->AddRow();
}

//------------------------------------------------------------------------------
/**
    Завершите запись текущей строки. Это обновит столбцы
    массив с любыми новыми идентификаторами атрибутов в текущей строке.
*/
void
Writer::EndRow()
{
    n_assert(this->isOpen);
    n_assert(this->inBeginRow);
    this->inBeginRow = false;
    this->rowIndex = InvalidIndex;
}

}; // пространство имен БД
