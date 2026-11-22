//------------------------------------------------------------------------------
//  sqlite3database.cc
//  (C) 2006 г. Радон Лабс ГмбХ
//  (C) 2013–2016 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "foundation/stdneb.h"
#include "db/dataset.h"
#include "db/sqlite3/sqlite3database.h"
#include "db/sqlite3/sqlite3command.h"
#include "db/sqlite3/sqlite3table.h"
#include "db/sqlite3/sqlite3factory.h"
#include "io/ioserver.h"
#include "attr/attributedefinitionbase.h"

namespace Attr
{
    // некоторые внутренние имена SQLite
    DefineString(name, 'name', ReadOnly);
    DefineString(type, 'type', ReadOnly);
    DefineInt(cid, 'cid_', ReadOnly);
    DefineInt(notnull, 'ntnl', ReadOnly);
    DefineBlob(dflt_value, 'dflv', ReadOnly);
    DefineBool(pk, 'pk__', ReadOnly);
    DefineInt(seq, 'seq_', ReadOnly);
    DefineBool(unique, 'uniq', ReadOnly);
    DefineInt(seqno, 'sqno', ReadOnly);
};

namespace Db
{
__ImplementClass(Db::Sqlite3Database, 'S3DB', Db::Database);

using namespace IO;
using namespace Util;

//------------------------------------------------------------------------------
/**
*/
Sqlite3Database::Sqlite3Database() :
    cacheNumPages(2000),
    tempStore(Memory),
    syncMode(false),
    busyTimeout(100),
    sqliteHandle(0)
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
Sqlite3Database::~Sqlite3Database()
{
    n_assert(!this->IsOpen());
}

//------------------------------------------------------------------------------
/**
    Открывает базу данных Sqlite3 (либо открывает существующую, либо создает 
    новая база данных). Настраивает базу данных через прагмы, читает существующие
    столы.
*/
bool
Sqlite3Database::Open()
{
    n_assert(!this->IsOpen());
    n_assert(0 == this->sqliteHandle);
    n_assert(!this->uri.IsEmpty());
    if (Database::Open())
    {
        IoServer* ioServer = IoServer::Instance();

        // проверьте, существует ли файл базы данных»
        if (!ioServer->FileExists(this->uri))
        {
            // if we are in read only mode, fail
            if ((ReadOnly == this->accessMode) || (ReadWriteExisting == this->accessMode))
            {
                String err;
                err.Format("Sqlite3Database::Open(): файл БД «%s» не существует!\n", this->uri.AsString().AsCharPtr());
                this->SetError(err);
                return false;
            }
            else
            {
                // if database is created, make sure directory path is valid
                bool dirSuccess = ioServer->CreateDirectory(this->uri.AsString().ExtractDirName());
                if (!dirSuccess)
                {
                    String err;
                    err.Format("Sqlite3Database::Open(): Failed to create path for DB file '%s'!\n", this->uri.AsString().AsCharPtr());
                    this->SetError(err);
                    return false;
                }
            }
        }
                
        // изменить путь для SQLite и открыть базу данных SQL
        String nativePath = this->uri.LocalPath();
        int err = SQLITE_OK;
        if (!this->memoryDatabase)
        {            
            err = sqlite3_open(nativePath.AsCharPtr(), &this->sqliteHandle);
        }
        else
        {
            err = sqlite3_open(":memory:", &this->sqliteHandle);
        }
        if (err != SQLITE_OK)
        {
            this->SetError(sqlite3_errmsg(this->sqliteHandle));
            this->sqliteHandle = 0;
            this->isOpen = false;
            return false;
        }

        // настроить базу данных
        err = sqlite3_busy_timeout(this->sqliteHandle, this->busyTimeout);
        if (err != SQLITE_OK)
        {
            this->SetError(sqlite3_errmsg(this->sqliteHandle));
            sqlite3_close(this->sqliteHandle);
            this->sqliteHandle = 0;
            this->isOpen = false;
            return false;
        }
            
        // включить расширенные сообщения с кодами ошибок
        err = sqlite3_extended_result_codes(this->sqliteHandle, 1);
        if (err != SQLITE_OK)
        {
            this->SetError(sqlite3_errmsg(this->sqliteHandle));
            sqlite3_close(this->sqliteHandle);
            this->sqliteHandle = 0;
            this->isOpen = false;
            return false;
        }

        // включите эксклюзивный режим, чтобы другие программы не могли 
        // доступ к базе данных, пока она открыта
        String sql;
        Ptr<Command> cmd = DbFactory::Instance()->CreateCommand();

        if (this->exclusiveMode)
        {
            sql = "PRAGMA locking_mode=ЭКСКЛЮЗИВНЫЙ";
            cmd->CompileAndExecute(this, sql);
        }

        sql.Format("PRAGMAcache_size=%d", this->cacheNumPages);

        cmd->CompileAndExecute(this, sql);

        if (this->syncMode) sql = "PRAGMA синхронный = ВКЛ.";
        else sql = "PRAGMA синхронный = ВЫКЛ.";
        cmd->CompileAndExecute(this, sql);

        if (Memory == this->tempStore) sql = "PRAGMA temp_store=ПАМЯТЬ";
        else sql = "PRAGMA temp_store=ФАЙЛ";
        cmd->CompileAndExecute(this, sql);

        // компилировать команды начала и завершения транзакции
        this->beginTransactionCmd = DbFactory::Instance()->CreateCommand();
        this->beginTransactionCmd->Compile(this, "НАЧАЛО ОТЛОЖЕНО");
        this->endTransactionCmd = DbFactory::Instance()->CreateCommand();
        this->endTransactionCmd->Compile(this, "COMMIT");
        
        // for in-memory database we have to copy all tables from db on disk
        if (this->memoryDatabase)
        {
            // присоедините файл базы данных диска и скопируйте все таблицы в базу данных памяти               
            this->AttachDatabase(nativePath, "fileDB");

            // запросить существующие таблицы
            cmd = DbFactory::Instance()->CreateCommand();
            Ptr<ValueTable> result = Sqlite3Factory::Instance()->CreateValueTable();
            cmd->CompileAndExecute(this, "SELECT name FROM fileDB.sqlite_master WHERE type='table'", result);
            
            char* errMsg;
            IndexT i;
            for (i = 0; i < result->GetNumRows(); i++)
            {
                String sqlStatement("CREATE TABLE ");
                String newName(result->GetString(Attr::name, i));                
                sqlStatement.Append(newName);
                sqlStatement.Append(" КАК ВЫБРАТЬ * ИЗ fileDB.");
                sqlStatement.Append(result->GetString(Attr::name, i));
                sqlite3_exec(this->sqliteHandle, sqlStatement.AsCharPtr(), NULL, NULL, &errMsg);        
            }
            this->DetachDatabase("fileDB");
        }
        
        // прочитать существующие таблицы базы данных        
        this->ReadTableLayouts();

        return true;
    }
    return false;
}

//------------------------------------------------------------------------------
/**
    Закройте базу данных.
*/
void
Sqlite3Database::Close()
{
    n_assert(this->IsOpen());
    n_assert(0 != this->sqliteHandle);

    // сначала вызовите родительский класс, это отключит все таблицы
    Database::Close();

    // отпустите команды транзакции
    this->beginTransactionCmd = nullptr;
    this->endTransactionCmd = nullptr;

    // затем закройте базу данных Sqlite
    int err = sqlite3_close(this->sqliteHandle);
    if (err != SQLITE_OK)
    {
        this->SetError(sqlite3_errmsg(this->sqliteHandle));
        n_error("Sqlite3Database::Close(): Failed to close db '%s' with error '%s'\n", 
            this->uri.AsString().AsCharPtr(), this->error.AsCharPtr());
    }
    this->sqliteHandle = 0;
}

//------------------------------------------------------------------------------
/**
    Начать транзакцию базы данных.
*/
void
Sqlite3Database::BeginTransaction()
{
    if (!this->beginTransactionCmd->IsValid())
    {
        this->beginTransactionCmd->Compile(this, "BEGIN");
    }
    bool beginTransactionExecuted = this->beginTransactionCmd->Execute();
    n_assert(beginTransactionExecuted);
}

//------------------------------------------------------------------------------
/**
    Завершить текущую транзакцию базы данных.
*/
void
Sqlite3Database::EndTransaction()
{
    if (!this->endTransactionCmd->IsValid())
    {
        this->endTransactionCmd->Compile(this, "COMMIT");
    }
    bool endTransactionExecuted = this->endTransactionCmd->Execute();
    n_assert(endTransactionExecuted);
}

//------------------------------------------------------------------------------
/**
    Registers additional attributes defined in the database's special
    _Attributes table. This enables an application to function even
    without knowing all attributes beforehand.

    FIXME: we could also check here if the attributes in the database
    are type-compatibel with the already existing attributes(?).
*/
void
Sqlite3Database::RegisterAttributes(Ptr<Table>& attrTable)
{
    n_assert(0 != attrTable);

    // create a dataset and read table contents
    Ptr<Dataset> dataset = attrTable->CreateDataset();
    dataset->AddAllTableColumns();
    dataset->PerformQuery();
    Ptr<ValueTable> values = dataset->Values();
    IndexT rowIndex;
    SizeT numRows = values->GetNumRows();
    for (rowIndex = 0; rowIndex < numRows; rowIndex++)
    {
        const Util::String& attrName = values->GetString(Attr::AttrName, rowIndex);        
        if (!Attr::AttrId::IsValidName(attrName))
        {
            Util::String valueString = values->GetString(Attr::AttrType, rowIndex);
        #if NEBULA3_DATABASE_LEGACY_VECTORS
            // поддержка экспорта nebula2
            if (valueString == "vector3" || valueString == "vector4")
            {
                valueString = "float4";
            }
        #endif
            Attr::ValueType valueType = Attr::Attribute::StringToValueType(valueString);
            bool readWrite = values->GetBool(Attr::AttrReadWrite, rowIndex);
            Attr::AccessMode accessMode = readWrite ? Attr::ReadWrite : Attr::ReadOnly;
            Attr::AttributeDefinitionBase::RegisterDynamicAttribute(attrName, valueString, Util::FourCC(), valueType, accessMode);
        }
    }
}

//------------------------------------------------------------------------------
/**
    Reads in the existing database table structure, and attaches them as
    Table objects. 
*/
void
Sqlite3Database::ReadTableLayouts()
{
    n_assert(0 == this->GetNumTables());

    // запросить существующие таблицы
    Ptr<Command> cmd = DbFactory::Instance()->CreateCommand();
    Ptr<ValueTable> result = Sqlite3Factory::Instance()->CreateValueTable();
    cmd->CompileAndExecute(this, "SELECT name FROM sqlite_master WHERE type='table'", result);

    // first check if there is an _Attributes table, if it exists,
    // зарегистрировать все атрибуты в этой таблице
    Util::Array<IndexT> attrTableRowIndices = result->FindRowIndicesByAttr(Attr::Attribute(Attr::name, "_Attributes"), true);
    if (!attrTableRowIndices.IsEmpty())
    {
        // подключаем таблицу атрибутов как обычно...
        Ptr<Sqlite3Table> attrsTable = Sqlite3Factory::Instance()->CreateTable().cast<Sqlite3Table>();
        attrsTable->SetName(result->GetString(Attr::name, attrTableRowIndices[0]));
        this->tables.Append(attrsTable.cast<Table>());
        attrsTable->Connect(this, Table::AssumeExists, this->ignoreUnknownColumns);

        // зарегистрировать все атрибуты в таблице...
        Ptr<Table> tempTablePtr = attrsTable.cast<Table>();
        this->RegisterAttributes(tempTablePtr);
    }

    // соединить все остальные таблицы
    IndexT rowIndex;
    SizeT numRows = result->GetNumRows();
    for (rowIndex = 0; rowIndex < numRows; rowIndex++)
    {
        // create a table object and connect it to the database,
        // таблица будет читать макет таблицы, когда она подключена
        if (attrTableRowIndices.IsEmpty() || rowIndex != attrTableRowIndices[0])
        {
            Ptr<Sqlite3Table> table = Sqlite3Factory::Instance()->CreateTable().cast<Sqlite3Table>();
            table->SetName(result->GetString(Attr::name, rowIndex));
            this->tables.Append(table.cast<Table>());
            table->Connect(this, Table::AssumeExists, this->ignoreUnknownColumns);
        }
    }
}

//------------------------------------------------------------------------------
/**
    Это прикрепляет файл внешней базы данных к этой базе данных под заданным именем.
    имя. Подробности смотрите в документации по SQLite!
*/
bool
Sqlite3Database::AttachDatabase(const URI& uri, const String& dbName)
{
    n_assert(this->IsOpen());
    n_assert(dbName.IsValid());

    // сначала проверьте, существует ли файл на самом деле
    if (IoServer::Instance()->FileExists(uri))
    {
        // SQLite хочет иметь собственный путь в формате UTF-8.
        String nativePath = this->uri.GetTail();
        
        // собрать и выполнить команду SQL
        String sql;
        sql.Format("ПРИСОЕДИНИТЬ БАЗУ ДАННЫХ '%s' КАК '%s'", nativePath.AsCharPtr(), dbName.AsCharPtr());
        Ptr<Command> cmd = DbFactory::Instance()->CreateCommand();
        bool dbAttached = cmd->CompileAndExecute(this, sql);
        if (!dbAttached)
        {
            n_error("Sqlite3Database::AttachDatabase(%s, %s) failed with sqlite error: %s",
                uri.AsString().AsCharPtr(), dbName.AsCharPtr(), cmd->GetError().AsCharPtr());
            return false;
        }
        return true;
    }
    else
    {
        n_error("Sqlite3Database::AttachDatabase: файл «%s» не существует!", uri.AsString().AsCharPtr());
    }
    return false;
}

//------------------------------------------------------------------------------
/**
    Это отключает внешнюю базу данных от этой базы данных. См. SQLite 
    документы для подробностей!
*/
void
Sqlite3Database::DetachDatabase(const String& dbName)
{
    n_assert(this->IsOpen());
    n_assert(dbName.IsValid());
    String sql;
    sql.Format("ОТКЛЮЧИТЬ БАЗУ ДАННЫХ '%s'", dbName.AsCharPtr());
    Ptr<Command> cmd = DbFactory::Instance()->CreateCommand();
    bool dbDetached = cmd->CompileAndExecute(this, sql);
    if (!dbDetached)
    {
        n_error("Sqlite3Database::AttachDatabase(%s, %s) failed with sqlite error: %s",
            uri.AsString().AsCharPtr(), dbName.AsCharPtr(), cmd->GetError().AsCharPtr());
    }
}

//------------------------------------------------------------------------------
/**
*/
void
Sqlite3Database::CopyInMemoryDatabaseToFile(const IO::URI& fileUri)
{
    // присоедините файл базы данных диска и скопируйте все таблицы из базы данных памяти в файл базы данных
    this->AttachDatabase(fileUri, "fileDB");

    // запросить существующие таблицы
    Ptr<Command> cmd = DbFactory::Instance()->CreateCommand();
    Ptr<ValueTable> result = Sqlite3Factory::Instance()->CreateValueTable();
    cmd->CompileAndExecute(this, "SELECT name FROM sqlite_master WHERE type='table'", result);
    
    char* errMsg;
    IndexT i;
    for (i = 0; i < result->GetNumRows(); i++)
    {
        String sqlStatement("CREATE TABLE fileDB.");
        sqlStatement.Append(result->GetString(Attr::name, i));
        sqlStatement.Append(" КАК ВЫБРАТЬ * ИЗ ");
        sqlStatement.Append(result->GetString(Attr::name, i));
        sqlite3_exec(this->sqliteHandle, sqlStatement.AsCharPtr(), NULL, NULL, &errMsg);
    }
    this->DetachDatabase("fileDB");
}

} // пространство имен БД
