//------------------------------------------------------------------------------
//  sqlite3dataset.cc
//  (C) 2006 г. Радон Лабс ГмбХ
//  (C) 2013–2016 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "foundation/stdneb.h"
#include "db/sqlite3/sqlite3dataset.h"
#include "db/command.h"
#include "db/dbfactory.h"
#include "db/database.h"
#include "db/table.h"

namespace Db
{
__ImplementClass(Db::Sqlite3Dataset, 'S3DS', Db::Dataset);

using namespace Util;

// статические строки для предотвращения чрезмерного построения строковых объектов
const Util::String Sqlite3Dataset::SelectFrag("SELECT ");
const Util::String Sqlite3Dataset::FromFrag(" FROM ");
const Util::String Sqlite3Dataset::CommaFrag(",");
const Util::String Sqlite3Dataset::WhereFrag(" WHERE ");
const Util::String Sqlite3Dataset::StarFrag("*");
const Util::String Sqlite3Dataset::TickFrag("\"");

//------------------------------------------------------------------------------
/**
*/
Sqlite3Dataset::Sqlite3Dataset()
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
Sqlite3Dataset::~Sqlite3Dataset()
{
    if (this->IsConnected())
    {
        this->Disconnect();
    }
}

//------------------------------------------------------------------------------
/**
*/
bool
Sqlite3Dataset::Connect()
{
    if (Dataset::Connect())
    {
        // добавьте в таблицу любые новые столбцы из нашей таблицы значений
        const Util::Array<IndexT>& newColIndices = this->values->GetNewColumnIndices();
        IndexT i;
        for (i = 0; i < newColIndices.Size(); i++)
        {
            const Attr::AttrId& newColAttrId = this->values->GetColumnId(newColIndices[i]);
            if (!this->table->HasColumn(newColAttrId))
            {
                this->table->AddColumn(Db::Column(newColAttrId));
            }
        }
        n_assert(!this->queryCommand.isvalid());
        return true;
    }
    return false;
}

//------------------------------------------------------------------------------
/**
*/
void
Sqlite3Dataset::Disconnect()
{
    // освободить объекты команды
    this->queryCommand = nullptr;
    Dataset::Disconnect();
}

//------------------------------------------------------------------------------
/**
*/
void
Sqlite3Dataset::PerformQuery(bool appendResult)
{
    n_assert(!this->IsConnected());
    n_assert(this->values.isvalid());
    n_assert(this->table.isvalid());

    // подключиться к таблице базы данных
    this->Connect();

    // if not appending, clear all rows from the value table,
    // в противном случае новый запрос будет просто добавлен в таблицу значений.
    if (!appendResult)
    {
        this->values->Clear();
    }

    // if our table layout is invalid, we need at least update the database's
    // макет таблицы, чтобы следующий запрос скомпилировался!
    if (this->table->HasUncommittedColumns())
    {
        this->table->CommitUncommittedColumns();
    }

    // проверьте, можем ли мы повторно использовать существующий объект команды
    if (!this->queryCommand.isvalid())
    {
        this->queryCommand = DbFactory::Instance()->CreateCommand();
    }
    if (!this->queryCommand->IsValid() || this->filter->IsDirty())
    {
        // перекомпилировать команду
        String sql;
        sql.Reserve(4096);
        sql = SelectFrag;
        sql.Append(this->GetSqlSelectColumns());
        sql.Append(FromFrag);
        sql.Append(this->table->GetName());

        // добавить необязательный оператор WHERE
        if (this->filter.isvalid() && (!this->filter->IsEmpty()))
        {
            sql.Append(WhereFrag);
            sql.Append(this->filter->AsSqlWhere());
        }
        this->filter->ClearDirtyFlag();

        // перекомпилировать команду запроса...
        bool compiled = this->queryCommand->Compile(this->table->GetDatabase(), sql, this->values);
        if (!compiled)
        {
            n_error("Sqlite3Dataset::PerformQuery: ошибка компиляции оператора SQL:\n%s\nС ошибкой:\n%s\n", 
                sql.AsCharPtr(), this->queryCommand->GetError().AsCharPtr());
            return;
        }
    }

    // привязать значения для команды
    if (this->filter.isvalid() && (!this->filter->IsEmpty()))
    {
        this->filter->BindValuesToCommand(this->queryCommand, 0);
    }

    // ок, запрос скомпилирован и действителен, теперь выполните его
    bool executed = this->queryCommand->Execute();
    if (!executed)
    {
        n_error("Sqlite3Dataset::PerformQuery: ошибка выполнения оператора SQL:\n%s\nС ошибкой:\n%s\n", 
            this->queryCommand->GetSqlCommand().AsCharPtr(), this->queryCommand->GetError().AsCharPtr());
        return;
    }
    
    // отключиться от нашего стола
    this->Disconnect();
}

//------------------------------------------------------------------------------
/**
    Commits any changes in the value table into the database.
*/
void
Sqlite3Dataset::CommitChanges(bool newRowsAsUpdate)
{
    n_assert(!this->IsConnected());
    this->Connect();
    this->table->CommitChanges(newRowsAsUpdate);
    this->Disconnect();
}

//------------------------------------------------------------------------------
/**
    Немедленно удаляет строки из базы данных.
*/
void
Sqlite3Dataset::CommitDeletedRows()
{
    n_assert(!this->IsConnected());
    this->Connect();
    this->table->CommitDeletedRows();
    this->Disconnect();
}

//------------------------------------------------------------------------------
/**
    Helper method which returns a SQL fragment string containing the columns
    to select. In the simplest case this method just returns a "*". Otherwise
    the format "table.column" will be returned.
*/
String
Sqlite3Dataset::GetSqlSelectColumns() const
{
    n_assert(this->table.isvalid());
    String str;
    str.Reserve(4096);

    // if no columns are defined, then return everything in the table
    if (this->values->GetNumColumns() == 0) 
    {
        str = StarFrag;
    }
    else
    {
        IndexT colIndex;
        SizeT numColumns = this->values->GetNumColumns();
        for (colIndex = 0; colIndex < numColumns; colIndex++)
        {
            str.Append(TickFrag);
            str.Append(this->values->GetColumnName(colIndex));
            str.Append(TickFrag);
            if (colIndex < (numColumns - 1))
            {
                str.Append(CommaFrag);
            }
        }
    }
    return str;
}

} // пространство имен БД
