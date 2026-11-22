//------------------------------------------------------------------------------
//  sqlite3table.cc
//  (C) 2006 г. Радон Лабс ГмбХ
//  (C) 2013–2016 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "foundation/stdneb.h"
#include "db/sqlite3/sqlite3table.h"
#include "db/sqlite3/sqlite3database.h"
#include "db/command.h"
#include "db/sqlite3/sqlite3factory.h"
#include "attr/valuetype.h"
#include "attr/attrid.h"

namespace Db
{
__ImplementClass(Db::Sqlite3Table, 'S3TB', Db::Table);

// статические строки для предотвращения чрезмерного построения строковых объектов
const Util::String Sqlite3Table::InsertIntoFrag("INSERT INTO ");
const Util::String Sqlite3Table::OpenBracketFrag(" (");
const Util::String Sqlite3Table::CommaFrag(",");
const Util::String Sqlite3Table::ValuesFrag(") ЦЕННОСТИ (");
const Util::String Sqlite3Table::WildcardFrag("?");
const Util::String Sqlite3Table::CloseBracketFrag(")");
const Util::String Sqlite3Table::UpdateFrag("UPDATE ");
const Util::String Sqlite3Table::SetFrag(" SET ");
const Util::String Sqlite3Table::AssignWildcardFrag("=?");
const Util::String Sqlite3Table::TickFrag("\"");
const Util::String Sqlite3Table::IntegerFrag(" INTEGER");
const Util::String Sqlite3Table::RealFrag(" REAL");
const Util::String Sqlite3Table::TextFrag(" TEXT");
const Util::String Sqlite3Table::BlobFrag(" BLOB");
const Util::String Sqlite3Table::WhereFrag(" WHERE ");
const Util::String Sqlite3Table::DeleteFromFrag("DELETE FROM ");

using namespace Util;

//------------------------------------------------------------------------------
/**
*/
Sqlite3Table::Sqlite3Table()
{
    // empty
}

//------------------------------------------------------------------------------
/**
    NOTE: destroying a Table object does not automatically commit changes!
    Changes will be lost if CommitChanges() isn't called manually. This is
    because the Table could have been dropped from the db, in this case
    a CommitChanges() would add unnessecary overhead.
    
*/
Sqlite3Table::~Sqlite3Table()
{
    if (this->IsConnected())
    {
        // зафиксировать изменения и отключиться от базы данных
        this->Disconnect(false);
    }
}

//------------------------------------------------------------------------------
/**
    Set the name of the table. If the table is connected to a database
    this will immediately rename the database table as well.
*/
void
Sqlite3Table::SetName(const Util::String& newName)
{
    if (this->IsConnected())
    {
        // создайте команду SQL, которая выполняет переименование
        String sql;
        sql.Format("ALTER TABLE '%s' RENAME TO '%s'", this->name.AsCharPtr(), newName.AsCharPtr());
        Ptr<Command> cmd = DbFactory::Instance()->CreateCommand();
        cmd->CompileAndExecute(this->database, sql);
    }
    Table::SetName(newName);
}

//------------------------------------------------------------------------------
/**
    Private helper method which checks if an associated database
    table exists.
*/
bool
Sqlite3Table::TableExists()
{
    n_assert(this->database.isvalid());
    n_assert(this->name.IsValid());

    String sql;
    sql.Format("SELECT name FROM 'sqlite_master' WHERE type='table' AND name='%s'", this->name.AsCharPtr());
    Ptr<Command> cmd = Db::Sqlite3Factory::Instance()->CreateCommand();
    Ptr<ValueTable> result = Sqlite3Factory::Instance()->CreateValueTable();
    cmd->CompileAndExecute(this->database, sql, result);
    return (result->GetNumRows() > 0);
}

//------------------------------------------------------------------------------
/**
    Частный вспомогательный метод, который удаляет связанную таблицу базы данных.
*/
void
Sqlite3Table::DropTable()
{
    n_assert(this->database.isvalid());
    n_assert(this->name.IsValid());

    String sql;
    sql.Format("DROP TABLE '%s'", this->name.AsCharPtr());
    Ptr<Command> cmd = Sqlite3Factory::Instance()->CreateCommand();
    cmd->CompileAndExecute(this->database, sql);
}

//------------------------------------------------------------------------------
/**
    Helper method which builds an SQL column definition string fragment
    from a column object.
*/
String
Sqlite3Table::BuildColumnDef(const Column& column)
{
    String def(TickFrag);
    def.Append(column.GetName());
    def.Append(TickFrag);
    switch (column.GetValueType())
    {
        case Attr::IntType:
        case Attr::BoolType:
            def.Append(IntegerFrag);
            break;
        
        case Attr::FloatType:
            def.Append(RealFrag);
            break;

        case Attr::StringType:
            def.Append(TextFrag);
            break;

        case Attr::Vec4Type:
        case Attr::Mat4Type:
        case Attr::BlobType:
        case Attr::GuidType:
            def.Append(BlobFrag);
            break;
        case Attr::VoidType:
            n_error("Sqlite3Table::BuildColumnDef() Тип Void!");
            break;
        default: n_error("Sqlite3Table::BuildColumnDef() unhandled enum");break;
    }
    if (column.GetType() == Column::Primary)
    {
        def.Append(" PRIMARY KEY ON CONFLICT REPLACE");
    }
    return def;
}

//------------------------------------------------------------------------------
/**
    Частный вспомогательный метод, который создает связанную таблицу базы данных.
*/
void
Sqlite3Table::CreateTable()
{
    n_assert(this->database.isvalid());
    n_assert(this->name.IsValid());

    // построить строку SQL, которая создает таблицу
    String sql;
    sql.Format("CREATE TABLE '%s' ( ", this->name.AsCharPtr());
    IndexT colIndex;
    SizeT numColumns = this->GetNumColumns();
    for (colIndex = 0; colIndex < numColumns; colIndex++)
    {
        Column& column = this->columns[colIndex];
        column.SetCommitted(true);
        sql.Append(this->BuildColumnDef(column));
        if (colIndex < (numColumns - 1))
        {
            sql.Append(CommaFrag);
        }
    }
    sql.Append(CloseBracketFrag);

    // выполнить оператор SQL
    Ptr<Command> cmd = DbFactory::Instance()->CreateCommand();
    cmd->CompileAndExecute(this->database, sql);

    // создавать индексы
    for (colIndex = 0; colIndex < numColumns; colIndex++)
    {
        const Column& column = this->GetColumn(colIndex);
        if (column.GetType() == Column::Indexed)
        {
            sql.Format("CREATE INDEX %s_%s ON %s ( %s )", 
                this->GetName().AsCharPtr(),
                column.GetName().AsCharPtr(), 
                this->GetName().AsCharPtr(), 
                column.GetName().AsCharPtr());
            cmd->CompileAndExecute(this->database, sql);
        }
    }
}

//------------------------------------------------------------------------------
/**
    This synchronizes this table object with the database table on
    a column-by-column basis. If this table has columns which don't exist in
    the database, they will be added to the database table (however, this actually
    happens any time later during CommitChanges(). If this
    object is missing any columns from the db, they will be created on the table
    object. This method cannot be used to delete any columns from the database
    (in fact, deleting columns is also impossible in Sqlite).
*/
void
Sqlite3Table::ReadTableLayout(bool ignoreUnknownColumns)
{
    n_assert(this->database.isvalid());
    n_assert(this->name.IsValid());

    // получить информацию о таблице из sqlite
    String sql;
    sql.Format("PRAGMA table_info(%s)", this->name.AsCharPtr());
    Ptr<Command> cmd = DbFactory::Instance()->CreateCommand();
    Ptr<ValueTable> tableInfoResult = Sqlite3Factory::Instance()->CreateValueTable();
    cmd->CompileAndExecute(this->database, sql, tableInfoResult);

    // каждая строка результата описывает столбец, мы 
    // хотя в принципе просто название интересует...
    IndexT i;
    SizeT num = tableInfoResult->GetNumRows();
    for (i = 0; i < num; i++)
    {
        // прочитать атрибуты столбца из sqlite
        const String& columnName = tableInfoResult->GetString(Attr::name,  i);
        
        // проверьте, действительно ли столбец существует как атрибут
        bool validColumn = Attr::AttrId::IsValidName(columnName);
        if ((false==validColumn) && (false==ignoreUnknownColumns))
        {
            // неизвестный столбец, выдает ошибку
            n_error("Sqlite3Table::ReadTableLayout(): недопустимый столбец «%s» в таблице «%s»!", 
                columnName.AsCharPtr(), this->name.AsCharPtr());
        }
        if (validColumn)
        {
            // check if column already exists in this table object
            Attr::AttrId attrId(columnName);
            if (!this->HasColumn(attrId))
            {
                // еще не существует, создайте новый столбец и прикрепите
                Column newColumn;
                newColumn.SetAttrId(columnName);
                if (tableInfoResult->GetBool(Attr::pk, i))
                {
                    newColumn.SetType(Column::Primary);
                }
                newColumn.SetCommitted(true);
                this->AddColumn(newColumn);
            }
        }
    }

    // запросить метаданные для индексированных столбцов
    sql.Format("PRAGMA index_list(%s)", this->GetName().AsCharPtr());
    Ptr<ValueTable> indexListResult = Sqlite3Factory::Instance()->CreateValueTable();
    cmd->CompileAndExecute(this->database, sql, indexListResult);
    for (i = 0; i < indexListResult->GetNumRows(); i++)
    {
        const String& indexName = indexListResult->GetString(Attr::name, i);
        if (!String::MatchPattern(indexName, "sqlite_*"))
        {
            Ptr<Command> indexInfoCmd = DbFactory::Instance()->CreateCommand();
            Ptr<ValueTable> indexInfoResult = Sqlite3Factory::Instance()->CreateValueTable();
            sql.Format("PRAGMA index_info(%s)", indexName.AsCharPtr());
            indexInfoCmd->CompileAndExecute(this->database, sql, indexInfoResult);
            if (1 == indexInfoResult->GetNumRows())
            {
                // мы рассматриваем только индексы с одним столбцом
                const String& columnName = indexInfoResult->GetString(Attr::name, 0);
                if (Attr::AttrId::IsValidName(columnName))
                {
                    Attr::AttrId attrId(columnName);
                    if (this->HasColumn(attrId))
                    {
                        Column& column = const_cast<Column&>(this->GetColumn(attrId));
                        if (column.GetType() != Column::Primary)
                        {
                            column.SetType(Column::Indexed);
                        }
                    }
                }
            }
        }
    }
}

//------------------------------------------------------------------------------
/**
    This connects the table object with a database, from that moment on,
    all changes to the table object will be synchronized with the database.
    Changes will be batched until CommitChanges() is called. 

    The following stuff happens at Connect() depending on the ConnectMode:
    - ConnectMode=ForceCreate: a new empty table will be created in the 
      database, if a table of the same name exists, it will be deleted first
    - ConnectMode=AssumeExists: it is assumed that the table exists, the
      columns in the database will be checked against any columns in the
      table object and the Committed flag will be set on existing columns
    - ConnectMode=Default: it is checked first if the table exists, if 
      not it will be acted like ForceCreate, otherwise it will be acted
      like AssumeExists
*/
void
Sqlite3Table::Connect(const Ptr<Database>& db, ConnectMode connectMode, bool ignoreUnknownColumns)
{
    n_assert(!this->IsConnected());
    n_assert(this->name.IsValid());

    // базовый класс первого вызова
    Table::Connect(db, connectMode, ignoreUnknownColumns);

    // теперь действуйте в зависимости от режима подключения
    if (ForceCreate == connectMode)
    {
        // ForceCreate: if table exists, delete existing and create new
        n_assert(!this->TableExists());
        this->CreateTable();
    }
    else if (AssumeExists == connectMode)
    {
        // AssumeExists: синхронизировать столбцы таблицы
        this->ReadTableLayout(ignoreUnknownColumns);
    }
    else
    {
        // Default: if table exists, synchronize it, if not, create new
        if (this->TableExists())
        {
            this->ReadTableLayout(ignoreUnknownColumns);
        }
        else
        {
            this->CreateTable();
        }
    }

    // создавать объекты команд
    this->insertCommand = DbFactory::Instance()->CreateCommand();
    this->updateCommand = DbFactory::Instance()->CreateCommand();
    this->deleteCommand = DbFactory::Instance()->CreateCommand();
}

//------------------------------------------------------------------------------
/**
    This disconnects the table from the database. If the dropTable argument
    is true (default is false), the actual database table will be deleted
    as well. Otherwise, a CommitChanges is invoked to write any local
    changes back into the database, and just the C++ table object is
    disconnected.
*/
void
Sqlite3Table::Disconnect(bool dropTable)
{
    if (dropTable)
    {
        this->DropTable();
    }
    this->insertCommand = nullptr;
    this->updateCommand = nullptr;
    this->deleteCommand = nullptr;
    Table::Disconnect(dropTable);
}

//------------------------------------------------------------------------------
/**
*/
void
Sqlite3Table::BindValueTable(const Ptr<ValueTable>& valTable)
{
    if (this->IsConnected())
    {
        this->insertCommand->Clear();
        this->updateCommand->Clear();
        this->deleteCommand->Clear();
    }
    Table::BindValueTable(valTable);
}

//------------------------------------------------------------------------------
/**
*/
void
Sqlite3Table::UnbindValueTable()
{
    Table::UnbindValueTable();
    if (this->IsConnected())
    {
        this->insertCommand->Clear();
        this->updateCommand->Clear();
        this->deleteCommand->Clear();
    }
}

//------------------------------------------------------------------------------
/**
    This writes any uncommited changes to the table layout back into the
    database. NOTE: it is not possible to make an existing column primary, or to 
    create an index for an existing column, this must happen when the table
    is actually created! This method also does not deletion of columns
    (SQLite doesn't support column removal, so what...).
*/
void
Sqlite3Table::CommitChanges(bool resetModifiedState, bool useTransaction)
{
    n_assert(this->database.isvalid());
    n_assert(this->name.IsValid());

    // есть ли вообще изменения?
    if (!(this->HasUncommittedColumns() || (this->valueTable.isvalid() && this->valueTable->IsModified())))
    {
        return;
    }

    // update table layout and recompile command objects if necessary
    if (this->HasUncommittedColumns())
    {
        this->CommitUncommittedColumns();
    }

    // зафиксировать любые изменения в значениях таблиц
    if (this->valueTable.isvalid())
    {
        if (!this->insertCommand->IsValid())
        {
            // перекомпилируйте команду вставки только в том случае, если действительно есть что вставить
            if (this->valueTable->HasNewRows())
            {
                this->CompileInsertCommand();
            }
        }

        // обновление и удаление работает только для таблиц с основным столбцом
        if (this->HasPrimaryColumn())
        {
            if (!this->updateCommand->IsValid())
            {
                // перекомпилируйте команду обновления только в том случае, если есть что обновить
                if (this->valueTable->HasModifiedRows())
                {
                    this->CompileUpdateCommand();
                }
            }
            if (!this->deleteCommand->IsValid())
            {
                // перекомпилируйте команду удаления только в том случае, если есть что удалить
                if (this->valueTable->HasDeletedRows())
                {
                    this->CompileDeleteCommand();
                }
            }
        }

        // запускать обновления внутри транзакции?
        if (useTransaction)
        {
            this->database->BeginTransaction();
        }

        // вставить новые данные (если нам не скажут 
        // обновить новые строки с помощью команды UPDATE)
        if (this->valueTable->HasNewRows())
        {
            this->ExecuteInsertCommand();
        }
        
        // обновление и удаление работает только для таблиц с основным столбцом
        if (this->HasPrimaryColumn())
        {
            if (this->valueTable->HasModifiedRows())
            {
                this->ExecuteUpdateCommand();
            }
            if (this->valueTable->HasDeletedRows())
            {
                this->ExecuteDeleteCommand();
            }
        }

        // совершить транзакцию
        if (useTransaction)
        {
            this->database->EndTransaction();
        }

        // Сбросьте измененное состояние нашей таблицы значений, если того требует
        // вызывающий абонент, если наша таблица значений является общей, это в большинстве случаев не
        // хорошая идея, так как другие могут зависеть от правильного 
        // изменилось состояние. Таким образом, мы оставляем это на усмотрение вызывающего абонента.
        if (resetModifiedState)
        {
            this->valueTable->ResetModifiedState();
        }
    }
}

//------------------------------------------------------------------------------
/**
    Это проверяет наличие незафиксированного столбца, изменяет таблицу базы данных.
    соответственно структуру.
*/
void
Sqlite3Table::CommitUncommittedColumns()
{
    n_assert(this->HasUncommittedColumns());

    Ptr<Command> cmd = DbFactory::Instance()->CreateCommand();

    // for each uncommitted column...
    String sql;
    IndexT i;
    for (i = 0; i < this->columns.Size(); i++)
    {
        Column& column = this->columns[i];
        if (!column.IsCommitted())
        {
            column.SetCommitted(true);

            // note: it is illegal to add primary or unique columns after table creation
            n_assert(column.GetType() != Column::Primary)
            sql.Format("ALTER TABLE '%s' ADD COLUMN ", this->name.AsCharPtr());
            sql.Append(this->BuildColumnDef(column));
            bool columnAdded = cmd->CompileAndExecute(this->database, sql);
            n_assert(columnAdded);

            // нужно создать индекс для нового столбца?
            if (column.GetType() == Column::Indexed)
            {
                sql.Format("CREATE INDEX %s_%s ON %s ( %s )", 
                    this->GetName().AsCharPtr(),
                    column.GetName().AsCharPtr(), 
                    this->GetName().AsCharPtr(), 
                    column.GetName().AsCharPtr());
                bool indexCreated = cmd->CompileAndExecute(this->database, sql);
                n_assert(indexCreated);
            }
        }
    }
}

//------------------------------------------------------------------------------
/**
    Immediately delete rows from the database.
*/
void
Sqlite3Table::CommitDeletedRows()
{
    n_assert(this->database.isvalid());
    n_assert(this->name.IsValid());
    if (this->valueTable.isvalid())
    {
        if (this->HasPrimaryColumn())
        {
            if (!this->deleteCommand->IsValid())
            {
                // перекомпилируйте команду удаления только в том случае, если есть что удалить
                if (this->valueTable->HasDeletedRows())
                {
                    this->CompileDeleteCommand();
                }
            }
        }
        if (this->HasPrimaryColumn())
        {
            if (this->valueTable->HasDeletedRows())
            {
                this->ExecuteDeleteCommand();
            }
        }
        this->valueTable->ClearDeletedRowsFlags();
    }
}

//------------------------------------------------------------------------------
/**
    Recompiles the insert command which will write a complete new
    row back into the database. The current implementation will
    only update main table rows!
*/
void
Sqlite3Table::CompileInsertCommand()
{
    n_assert(this->database.isvalid());
    n_assert(this->insertCommand.isvalid());
    n_assert(this->valueTable.isvalid());

    String sql;
    sql.Reserve(4096);
    sql.Append(InsertIntoFrag);
    sql.Append(this->GetName());
    sql.Append(OpenBracketFrag);

    // for an insert command we need to write ALL columns
    // не только столбцы чтения/записи
    IndexT colIndex;
    const SizeT numColumns = this->valueTable->GetNumColumns();
    for (colIndex = 0; colIndex < numColumns; colIndex++)
    {
        sql.Append(TickFrag);
        sql.Append(this->valueTable->GetColumnName(colIndex));
        sql.Append(TickFrag);
        if (colIndex < (numColumns - 1))
        {
            sql.Append(CommaFrag);
        }
    }
    sql.Append(ValuesFrag);
    for (colIndex = 0; colIndex < numColumns; colIndex++)
    {
        // примечание: здесь мы пишем заполнители!
        sql.Append(WildcardFrag);
        if (colIndex < (numColumns - 1))
        {
            sql.Append(CommaFrag);
        }
    }
    sql.Append(CloseBracketFrag);

    // скомпилировать команду
    bool compiled = this->insertCommand->Compile(this->database, sql, valueTable);
    if (!compiled)
    {
        n_error("Sqlite3Table::CompileInsertCommand: ошибка компиляции оператора SQL:\n%s\nС ошибкой:\n%s\n", 
            this->insertCommand->GetSqlCommand().AsCharPtr(), this->insertCommand->GetError().AsCharPtr());
    }
}

//------------------------------------------------------------------------------
/**
    Recompiles the update command which will write modified rows back 
    into the database.
*/
void
Sqlite3Table::CompileUpdateCommand()
{
    n_assert(this->HasPrimaryColumn());
    n_assert(this->database.isvalid());
    n_assert(this->updateCommand.isvalid());
    n_assert(this->valueTable.isvalid());
    n_assert(this->valueTable->HasColumn(this->GetPrimaryColumn().GetAttrId()));
    if (this->valueTable->GetNumColumns() == 1)
    {
        // обновление не имеет смысла для таблицы, в которой есть только основной столбец.
        return;
    }

    // в противном случае создайте строку обновления SQL
    String sql;
    sql.Reserve(4096);
    sql.Append(UpdateFrag);
    sql.Append(this->GetName());
    sql.Append(SetFrag);

    // записать пары ключ/подстановочный знак в оператор SQL
    const Attr::AttrId& primaryAttrId = this->GetPrimaryColumn().GetAttrId();
    IndexT colIndex;
    const SizeT numColumns = this->valueTable->GetNumColumns();
    for (colIndex = 0; colIndex < numColumns; colIndex++)
    {
        if (this->valueTable->GetColumnId(colIndex) != primaryAttrId)
        {
            const Util::String& columnName = this->valueTable->GetColumnName(colIndex);
            sql.Append(TickFrag);
            sql.Append(columnName);
            sql.Append(TickFrag);
            sql.Append(AssignWildcardFrag);
            if (colIndex < (numColumns - 1))
            {
                sql.Append(CommaFrag);
            }
        }   
    }
    // вырезать последний символ, если это запятая.
    if (sql[sql.Length()-1] == *CommaFrag.AsCharPtr())
    {
        sql.TerminateAtIndex(sql.Length() -1 );
    }
    sql.Append(WhereFrag);
    sql.Append(TickFrag);
    sql.Append(this->GetPrimaryColumn().GetName());
    sql.Append(TickFrag);
    sql.Append(AssignWildcardFrag);

    // ок, теперь скомпилируем команду
    bool compiled = this->updateCommand->Compile(this->database, sql);
    if (!compiled)
    {
        n_error("Sqlite3Dataset::CompileUpdateCommand: ошибка компиляции оператора SQL:\n%s\nС ошибкой:\n%s\n", 
            sql.AsCharPtr(), this->updateCommand->GetError().AsCharPtr());
    }
}

//------------------------------------------------------------------------------
/**
    Recompiles the delete command which will delete any deleted value table
    rows from the database.
*/
void
Sqlite3Table::CompileDeleteCommand()
{
    n_assert(this->HasPrimaryColumn());
    n_assert(this->database.isvalid());
    n_assert(this->deleteCommand.isvalid());

    // построить строку SQL
    String sql;
    sql.Reserve(1024);
    sql.Append(DeleteFromFrag);
    sql.Append(this->GetName());
    sql.Append(WhereFrag);
    sql.Append(TickFrag);
    sql.Append(this->GetPrimaryColumn().GetName());
    sql.Append(TickFrag);
    sql.Append(AssignWildcardFrag);

    // скомпилировать команду
    bool compiled = this->deleteCommand->Compile(this->database, sql);
    if (!compiled)
    {
        n_error("Sqlite3Dataset::CompileDeleteCommand: ошибка компиляции оператора SQL:\n%s\nС ошибкой:\n%s\n", 
            sql.AsCharPtr(), this->deleteCommand->GetError().AsCharPtr());
    }
}

//------------------------------------------------------------------------------
/**
    Helper method which binds a value from a value table to a command.
*/
void
Sqlite3Table::BindValueToCommand(const Ptr<Command>& cmd, IndexT wildCardIndex, IndexT valueTableColIndex, IndexT valueTableRowIndex)
{
    n_assert(this->valueTable.isvalid());
    switch (this->valueTable->GetColumnValueType(valueTableColIndex))
    {
        case Attr::IntType: 
            cmd->BindInt(wildCardIndex, this->valueTable->GetInt(valueTableColIndex, valueTableRowIndex));
            break;

        case Attr::FloatType:
            cmd->BindFloat(wildCardIndex, this->valueTable->GetFloat(valueTableColIndex, valueTableRowIndex));
            break;

        case Attr::BoolType:
            cmd->BindBool(wildCardIndex, this->valueTable->GetBool(valueTableColIndex, valueTableRowIndex));
            break;

        case Attr::Vec4Type:
            cmd->BindVec4(wildCardIndex, this->valueTable->GetVec4(valueTableColIndex, valueTableRowIndex));
            break;

        case Attr::StringType:
            cmd->BindString(wildCardIndex, this->valueTable->GetString(valueTableColIndex, valueTableRowIndex));
            break;

        case Attr::Mat4Type:
            cmd->BindMat4(wildCardIndex, this->valueTable->GetMat4(valueTableColIndex, valueTableRowIndex));
            break;

        case Attr::BlobType:
            cmd->BindBlob(wildCardIndex, this->valueTable->GetBlob(valueTableColIndex, valueTableRowIndex));
            break;

        case Attr::GuidType:
            cmd->BindGuid(wildCardIndex, this->valueTable->GetGuid(valueTableColIndex, valueTableRowIndex));
            break;

        default:
            n_error("Sqlite3Table::Sqlite3Table(): неверный тип значения атрибута!");
            break;
    }
}

//------------------------------------------------------------------------------
/**
    При этом выполняется предварительно скомпилированная команда вставки для каждой новой строки. Игнорирует
    удаленные строки.
*/
void
Sqlite3Table::ExecuteInsertCommand()
{
    n_assert(this->insertCommand.isvalid() && this->insertCommand->GetSqlCommand().IsValid());
    n_assert(this->valueTable);

    // for each new row...
    const Util::Array<IndexT> newRowIndices = this->valueTable->GetNewRowIndices();
    IndexT i;
    SizeT num = newRowIndices.Size();
    for (i = 0; i < num; i++)
    {
        // привязать значения к команде 
        IndexT valueTableRowIndex = newRowIndices[i];
        if (!this->valueTable->IsRowDeleted(valueTableRowIndex))
        {
            IndexT valueTableColIndex;
            SizeT numValueTableColumns = this->valueTable->GetNumColumns();
            for (valueTableColIndex = 0; valueTableColIndex < numValueTableColumns; valueTableColIndex++)
            {
                IndexT wildCardIndex = valueTableColIndex;
                this->BindValueToCommand(this->insertCommand, wildCardIndex, valueTableColIndex, valueTableRowIndex);
            }

            // выполнить команду
            bool executed = this->insertCommand->Execute();
            if (!executed)
            {
                n_error("Sqlite3Table::ExecuteInsertCommand(): error in row '%d' for command '%s' with '%s'", 
                    valueTableRowIndex, this->insertCommand->GetSqlCommand().AsCharPtr(), 
                    this->insertCommand->GetError().AsCharPtr());
            }
        }
    }
}

//------------------------------------------------------------------------------
/**
    This executes the pre-compiled update command for each modified row. 
    Ignores deleted rows.

    - 26-Jun-07   floh  fixed obscure bug where modified haven't been updated                        
*/
void
Sqlite3Table::ExecuteUpdateCommand()
{
    n_assert(this->HasPrimaryColumn());
    n_assert(this->valueTable.isvalid());
    n_assert(this->valueTable->HasColumn(this->GetPrimaryColumn().GetAttrId()));
    if (this->valueTable->GetNumColumns() == 1)
    {
        // обновление не имеет смысла для таблицы, в которой есть только основной столбец.
        return;
    }
    n_assert(this->updateCommand.isvalid() && this->updateCommand->IsValid());

    // for each modified row (but not new or deleted rows)... bind the row values to the command
    const Util::Array<IndexT> modRowIndices = this->valueTable->GetModifiedRowsExcludeNewAndDeletedRows();

    // get column index of primary column in value table
    IndexT valueTablePrimaryColIndex = this->valueTable->GetColumnIndex(this->GetPrimaryColumn().GetAttrId());
    n_assert(InvalidIndex != valueTablePrimaryColIndex);

    IndexT i;
    for (i = 0; i < modRowIndices.Size(); i++)
    {
        IndexT valueTableRowIndex = modRowIndices[i];
        IndexT wildCardIndex = 0;
        IndexT valueTableColIndex;
        const SizeT valueTableNumColumns = this->valueTable->GetNumColumns();
        for (valueTableColIndex = 0; valueTableColIndex < valueTableNumColumns; valueTableColIndex++)
        {
            if (valueTableColIndex != valueTablePrimaryColIndex)
            {
                this->BindValueToCommand(this->updateCommand, wildCardIndex++, valueTableColIndex, valueTableRowIndex);
            }
        }

        // bind primary key (for WHERE statement)
        this->BindValueToCommand(this->updateCommand, wildCardIndex, valueTablePrimaryColIndex, valueTableRowIndex);

        // выполнить команду
        bool executed = this->updateCommand->Execute();
        if (!executed)
        {
            n_error("Sqlite3Database::ExecuteUpdateCommand(): error in row '%d' for command '%s'", 
                valueTableRowIndex, this->updateCommand->GetSqlCommand().AsCharPtr());
        }
    }
}

//------------------------------------------------------------------------------
/**
    При этом выполняется предварительно скомпилированная команда удаления для каждой удаленной строки.
*/
void
Sqlite3Table::ExecuteDeleteCommand()
{
    n_assert(this->deleteCommand.isvalid() && this->deleteCommand->IsValid());
    n_assert(this->HasPrimaryColumn());
    n_assert(this->valueTable->HasColumn(this->GetPrimaryColumn().GetAttrId()));

    // get column index of primary column in value table
    IndexT valueTablePrimaryColIndex = this->valueTable->GetColumnIndex(this->GetPrimaryColumn().GetAttrId());
    n_assert(InvalidIndex != valueTablePrimaryColIndex);

    // for each deleted row...
    IndexT i;
    SizeT num = this->valueTable->GetDeletedRowIndices().Size();
    for (i = 0; i < num; i++)
    {
        IndexT valueTableRowIndex = this->valueTable->GetDeletedRowIndices()[i];

        // bind primary key value to command (for the WHERE statement)
        this->BindValueToCommand(this->deleteCommand, 0, valueTablePrimaryColIndex, valueTableRowIndex);
        
        // выполнить команду
        bool executed = this->deleteCommand->Execute();
        if (!executed)
        {
            n_error("Sqlite3Database::ExecuteDeleteCommand(): error in row '%d' for command '%s': %s", 
                valueTableRowIndex, 
                this->deleteCommand->GetSqlCommand().AsCharPtr(), 
                this->deleteCommand->GetError().AsCharPtr());
        }
    }
}

//------------------------------------------------------------------------------
/**
    This method creates a multicolumn index on the table. The table must
    be connected, otherwise an assertion will be thrown!
*/
void
Sqlite3Table::CreateMultiColumnIndex(const Util::Array<Attr::AttrId>& columnIds)
{
    n_assert(this->IsConnected());
    String sql;
    sql.Reserve(4096);
    sql.Append("CREATE INDEX ");
    sql.Append(this->GetName());
    sql.Append("_");
    IndexT i;
    for (i = 0; i < columnIds.Size(); i++)
    {
        n_assert(this->HasColumn(columnIds[i]));
        sql.Append(columnIds[i].GetName());
        if (i < (columnIds.Size() - 1))
        {
            sql.Append("_");
        }
    }
    sql.Append(" ON ");
    sql.Append(this->GetName());
    sql.Append(" ( ");
    for (i = 0; i < columnIds.Size(); i++)
    {
        sql.Append("'");
        sql.Append(columnIds[i].GetName());
        sql.Append("'");
        if (i < (columnIds.Size() - 1))
        {
            sql.Append(",");
        }
    }
    sql.Append(" ) ");
    Ptr<Command> cmd = DbFactory::Instance()->CreateCommand();
    cmd->CompileAndExecute(this->database, sql);
}

} // пространство имен БД
