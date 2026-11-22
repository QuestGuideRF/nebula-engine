//------------------------------------------------------------------------------
//  sqlite3command.cc
//  (C) 2006 г. Радон Лабс ГмбХ
//  (C) 2013–2016 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "foundation/stdneb.h"
#include "db/sqlite3/sqlite3command.h"
#include "db/sqlite3/sqlite3database.h"
#include "system/byteorder.h"

namespace Db
{
__ImplementClass(Db::Sqlite3Command, 'S3CD', Core::RefCounted);    // пропустить виртуальный базовый класс

using namespace Util;
using namespace Math;
using namespace System;

//------------------------------------------------------------------------------
/**
*/
Sqlite3Command::Sqlite3Command() :
    sqliteStatement(0)
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
Sqlite3Command::~Sqlite3Command()
{
    this->Clear();
}

//------------------------------------------------------------------------------
/**
    Очищает текущую скомпилированную команду.
*/
void
Sqlite3Command::Clear()
{
    if (0 != this->sqliteStatement)
    {
        sqlite3_finalize(this->sqliteStatement);
        this->sqliteStatement = 0;
    }
    this->valueTable = nullptr;
    this->resultIndexMap.Clear();
}

//------------------------------------------------------------------------------
/**
*/
void
Sqlite3Command::SetSqliteError()
{
    // получить хэндл базы данных из нашей базы данных
    Ptr<Sqlite3Database> db = this->database.downcast<Sqlite3Database>();
    n_assert(db->IsA(Sqlite3Database::RTTI));
    sqlite3* sqliteHandle = db->GetSqliteHandle();
    this->SetError(sqlite3_errmsg(sqliteHandle));
}

//------------------------------------------------------------------------------
/**
    Compiles a command with optional placeholders. Placeholders are
    identified by a leading $. After the command has been compiled,
    actual values can be bound to the placeholders using the Bind*()
    methods. When everything is in place, use Execute() to execute
    the command. If compilation fails, the method will return false. Get
    an error description with the method GetError().

    Remember that the SQL statement string must be UTF-8 encoded!
*/
bool
Sqlite3Command::Compile(const Ptr<Database>& db, const Util::String& sqlCommand, ValueTable* resultTable)
{
    // первое: уборка
    this->Clear();

    // затем вызовите родительский класс, который инициализирует члены
    Command::Compile(db, sqlCommand, resultTable);

    // получить хэндл базы данных из нашей базы данных
    Ptr<Sqlite3Database> sqliteDatabase = db.downcast<Sqlite3Database>();
    n_assert(db->IsA(Sqlite3Database::RTTI));
    sqlite3* sqliteHandle = sqliteDatabase->GetSqliteHandle();

    // позвольте SQLite скомпилировать команду SQL
    const char* cmdTail = 0;
    int err = sqlite3_prepare_v2(sqliteHandle, this->sqlCommand.AsCharPtr(), -1, &this->sqliteStatement, &cmdTail);
    if (err != SQLITE_OK)
    {
        this->SetSqliteError();
        return false;
    }
    n_assert(0 != this->sqliteStatement);

    // проверьте, было ли в строке более одного оператора SQL, мы это не поддерживаем
    n_assert(0 != cmdTail);
    if (cmdTail[0] != 0)
    {
        n_error("Sqlite3Command::Compile(): разрешен только один оператор SQL (cmd: %s)\n", this->sqlCommand.AsCharPtr());
        this->Clear();
        return false;
    }

    // create an index map to map value table indices to sqlite result indices
    if (this->valueTable.isvalid())
    {
        SizeT numResultColumns = sqlite3_column_count(this->sqliteStatement);
        if (numResultColumns > 0)
        {
            // if the value table has no columns we just add them now,
            // однако для производительности лучше, если таблица значений
            // предварительно настроен
            IndexT resultColumnIndex;
            if (0 == this->valueTable->GetNumColumns())
            {
                for (resultColumnIndex = 0; resultColumnIndex < numResultColumns; resultColumnIndex++)
                {
                    Attr::AttrId colAttrId(String(sqlite3_column_name(this->sqliteStatement, resultColumnIndex)));
                    if (!this->valueTable->HasColumn(colAttrId))
                    {
                        this->valueTable->AddColumn(colAttrId, false);
                    }
                }
            }

            // now create an index map which maps sqlite result indices to value table indices
            n_assert(this->resultIndexMap.Size() == 0);
            for (resultColumnIndex = 0; resultColumnIndex < numResultColumns; resultColumnIndex++)
            {
                Attr::AttrId colAttrId(String(sqlite3_column_name(this->sqliteStatement, resultColumnIndex)));
                if (valueTable->HasColumn(colAttrId))
                {
                    IndexT valueTableIndex = this->valueTable->GetColumnIndex(colAttrId);
                    this->resultIndexMap.Append(valueTableIndex);
                }
            }
        }
    }
    return true;
}

//------------------------------------------------------------------------------
/**
    Возвращает true, если команда скомпилирована и готова к использованию.
    выполнение, false, если команду необходимо перекомпилировать.
*/
bool
Sqlite3Command::IsValid() const
{
    if (0 != this->sqliteStatement)
    {
        // проверьте, нужно ли перекомпилировать оператор sqlite
        if (0 == sqlite3_expired(this->sqliteStatement))
        {
            // все ок
            return true;
        }
    }
    // необходимо составить
    return false;
}

//------------------------------------------------------------------------------
/**
    Выполните скомпилированную команду и получите результат.
*/
bool
Sqlite3Command::Execute()
{
    n_assert(this->IsValid());

    // отключить отслеживание изменений в таблице значений
    if (this->valueTable.isvalid())
    {
        this->valueTable->SetModifiedTracking(false);
    }

    // выполнить команду в несколько шагов
    bool done = false;
    while (!done)
    {
        int result = sqlite3_step(this->sqliteStatement);
        if (SQLITE_DONE == result)
        {
            // все ок и готово
            done = true;
        }
        else if (SQLITE_BUSY == result)
        {
            // кто-то еще сейчас блокирует базу данных, поспите немного
            n_sleep(0.0001);
        }
        else if (SQLITE_ROW == result)
        {
            if (this->valueTable.isvalid())
            {
                this->ReadRow();
            }
        }
        else if (SQLITE_ERROR == result)
        {
            this->SetSqliteError();
            return false;
        }
        else if (SQLITE_MISUSE == result)
        {
            n_error("Sqlite3Command::Execute(): sqlite3_step() вернул SQLITE_MISUSE!");
            return false;
        }
        else
        {
            n_error("Sqlite3Command::Execute(): unknown error code returned from sqlite3_step() (CAN'T HAPPEN!)");
            return false;
        }
    }

    // сбросьте команду, это очистит связанные значения
    int err = sqlite3_reset(this->sqliteStatement);
    n_assert(SQLITE_OK == err);

    // снова включите отслеживание изменений
    if (this->valueTable.isvalid())
    {
        this->valueTable->SetModifiedTracking(true);
    }

    return true;
}

//------------------------------------------------------------------------------
/**
    Gather row of result values from SQLite and add them to the result
    ValueTable. Note that modification tracking is turned off in the
    value table, because reading from the database doesn't count as
    modification.
*/
void
Sqlite3Command::ReadRow()
{
    n_assert(0 != this->sqliteStatement);
    n_assert(this->valueTable.isvalid());

    // добавить строку в таблицу значений результата
    IndexT rowIndex = this->valueTable->AddRow();

    ByteOrder byteOrder(System::ByteOrder::LittleEndian, System::ByteOrder::Host);
    // for each column...
    IndexT resultColumnIndex;
    const SizeT numResultColumns = sqlite3_data_count(this->sqliteStatement);
    for (resultColumnIndex = 0; resultColumnIndex < numResultColumns; resultColumnIndex++)
    {
        // получить тип столбца Sqlite, но он используется только для
        // NULL-значение и проверка утверждений, идентификатор атрибута определяет 
        // фактический тип данных!
        int sqliteColumnType = sqlite3_column_type(this->sqliteStatement, resultColumnIndex);
        if (SQLITE_NULL != sqliteColumnType)
        {
            // Сопоставить индекс результата с индексом таблицы значений
            IndexT valueTableColumnIndex = this->resultIndexMap[resultColumnIndex];
            switch (this->valueTable->GetColumnValueType(valueTableColumnIndex))
            {
                case Attr::IntType:
                    {
                        n_assert(SQLITE_INTEGER == sqliteColumnType);
                        int val = sqlite3_column_int(this->sqliteStatement, resultColumnIndex);                        
                        this->valueTable->SetInt(valueTableColumnIndex, rowIndex, val);
                    }
                    break;

                case Attr::FloatType:
                    {
                        n_assert(SQLITE_FLOAT == sqliteColumnType);
                        float val = (float) sqlite3_column_double(this->sqliteStatement, resultColumnIndex);                        
                        this->valueTable->SetFloat(valueTableColumnIndex, rowIndex, val);
                    }
                    break;

                case Attr::BoolType:
                    {
                        n_assert(SQLITE_INTEGER == sqliteColumnType);
                        int val = sqlite3_column_int(this->sqliteStatement, resultColumnIndex);
                        this->valueTable->SetBool(valueTableColumnIndex, rowIndex, (val == 1));
                    }
                    break;

                case Attr::Vec4Type:
                    {
                        n_assert(SQLITE_BLOB == sqliteColumnType);
                        const void* ptr = sqlite3_column_blob(this->sqliteStatement, resultColumnIndex);
                        uint size = sqlite3_column_bytes(this->sqliteStatement, resultColumnIndex);                        

                        vec4 alignedVal;
                        alignedVal.loadu((Math::scalar*)ptr);
                    #if NEBULA3_DATABASE_LEGACY_VECTORS
                        float4 value;
                        // поддержка устаревшей версии nebula2
                        if (size < sizeof(float4))
                        {
                            n_assert(size == 12);
                            // преобразованный вектор3, добавьте 0
                            value.set(alignedVal.x(), alignedVal.y(), alignedVal.z(), 0);
                        }
                        else
                        {
                            n_assert(size == sizeof(float4));
                            value.set(alignedVal.x(), alignedVal.y(), alignedVal.z(), alignedVal.w());
                        }
                        
                        byteOrder.ConvertInPlace<Math::float4>(value);
                        this->valueTable->SetFloat4(valueTableColumnIndex, rowIndex, value);
                    #else
                        n_assert(size == sizeof(float4));
                        ByteOrder byteOrder(ByteOrder::LittleEndian, ByteOrder::Host);
                        byteOrder.ConvertInPlace<Math::vec4>(alignedVal);
                        /**ptr = ByteOrder::ConvertFloat4(ByteOrder::LittleEndian, ByteOrder::Host, *ptr);*/
                        this->valueTable->SetVec4(valueTableColumnIndex, rowIndex, alignedVal);
                    #endif
                    }
                    break;

                case Attr::StringType:
                    {
                        n_assert(SQLITE_TEXT == sqliteColumnType);
                        String val = (const char*) sqlite3_column_text(this->sqliteStatement, resultColumnIndex);
                        this->valueTable->SetString(valueTableColumnIndex, rowIndex, val);
                    }
                    break;

                case Attr::Mat4Type:
                    {
                        n_assert(SQLITE_BLOB == sqliteColumnType);
                        const void* ptr = sqlite3_column_blob(this->sqliteStatement, resultColumnIndex);
                        int size = sqlite3_column_bytes(this->sqliteStatement, resultColumnIndex);
                        n_assert(size == sizeof(Math::mat4));
                        
                        Math::mat4 alignedVal;
                        alignedVal.loadu((scalar*)ptr);
                        byteOrder.ConvertInPlace<Math::mat4>(alignedVal);
                        this->valueTable->SetMat4(valueTableColumnIndex, rowIndex, alignedVal);                                            
                    }                   
                    break;

                case Attr::BlobType:
                    {
                        n_assert(SQLITE_BLOB == sqliteColumnType);
                        const void* ptr = sqlite3_column_blob(this->sqliteStatement, resultColumnIndex);
                        int size = sqlite3_column_bytes(this->sqliteStatement, resultColumnIndex);
                        Blob blob(ptr, size);
                        this->valueTable->SetBlob(valueTableColumnIndex, rowIndex, blob);
                    }
                    break;

                case Attr::GuidType:
                    {
                        n_assert(SQLITE_BLOB == sqliteColumnType);
                        const unsigned char* ptr = (const unsigned char*) sqlite3_column_blob(this->sqliteStatement, resultColumnIndex);
                        int size = sqlite3_column_bytes(this->sqliteStatement, resultColumnIndex);
                        Guid guid(ptr, size);
                        this->valueTable->SetGuid(valueTableColumnIndex, rowIndex, guid);
                    }
                    break;

                default:
                    n_error("Sqlite3Command::ReadResultRow(): неверный тип атрибута!");
                    break;
            }
        }
    }
}

//------------------------------------------------------------------------------
/**
    Returns the index of a placeholder value by name. The placeholder name
    must be UTF-8 encoded! NOTE: this method is slow because a temporary string
    must be created! It's much faster to work with unnamed wildcards ("?") and
    set them directly by index!
*/
IndexT
Sqlite3Command::IndexOf(const Util::String& name) const
{
    n_assert(0 != this->sqliteStatement);
 
    String wildcard = ":";
    wildcard.Append(name);

    IndexT index = sqlite3_bind_parameter_index(this->sqliteStatement, wildcard.AsCharPtr());
    n_assert2(0 != index, "Недопустимое имя подстановочного знака.");

    // Индексы Sqlite основаны на 1, конвертируются в 0.
    return (index - 1); 
}

//------------------------------------------------------------------------------
/**
    Returns the index of a placeholder value by attribute id. 
    NOTE: this method is slow because a temporary string
    must be created! It's much faster to work with unnamed wildcards ("?") and
    set them directly by index!
*/
IndexT
Sqlite3Command::IndexOf(const Attr::AttrId& attrId) const
{
    n_assert(0 != this->sqliteStatement);

    String wildcard = ":";
    wildcard.Append(attrId.GetName());

    IndexT index = sqlite3_bind_parameter_index(this->sqliteStatement, wildcard.AsCharPtr());
    n_assert2(0 != index, "Недопустимое имя подстановочного знака.");

    // Индексы Sqlite основаны на 1, конвертируются в 0.
    return (index - 1); 
}

//------------------------------------------------------------------------------
/**
    Bind an integer value by placeholder index. Either get the index from
    one of the IndexOf methods or use a 0-based index.
*/
void
Sqlite3Command::BindInt(IndexT index, int val)
{
    n_assert(0 != this->sqliteStatement);
    int err = sqlite3_bind_int(this->sqliteStatement, index + 1, val);
    n_assert(SQLITE_OK == err);
}

//------------------------------------------------------------------------------
/**
    Bind a float value by placeholder index. Either get the index from
    one of the IndexOf methods or use a 0-based index.
*/
void
Sqlite3Command::BindFloat(IndexT index, float val)
{
    n_assert(0 != this->sqliteStatement);
    int err = sqlite3_bind_double(this->sqliteStatement, index + 1, (double) val);
    n_assert(SQLITE_OK == err);
}

//------------------------------------------------------------------------------
/**
    Bind a bool value by placeholder index. Either get the index from
    one of the IndexOf methods or use a 0-based index.
*/
void
Sqlite3Command::BindBool(IndexT index, bool b)
{
    n_assert(0 != this->sqliteStatement);
    // ПРИМЕЧАНИЕ. SQLite не поддерживает собственные логические значения, вместо этого мы используем 
    // an int (1=true, 0=false)
    int val = b ? 1 : 0;
    int err = sqlite3_bind_int(this->sqliteStatement, index + 1, val);
    n_assert(SQLITE_OK == err);
}

//------------------------------------------------------------------------------
/**
    Bind a float4 value by placeholder index. Either get the index from
    one of the IndexOf methods or use a 0-based index.
*/
void
Sqlite3Command::BindVec4(IndexT index, const vec4& val)
{
    n_assert(0 != this->sqliteStatement);
    // ПРИМЕЧАНИЕ. float4 будут сохранены в базе данных как BLOB-объекты, поскольку 
    // float4 может исчезнуть в любой момент, пусть SQLite создаст собственную копию данных
    ByteOrder byteOrder(System::ByteOrder::LittleEndian, System::ByteOrder::Host);
    vec4 convertedVal = val;
    byteOrder.ConvertInPlace<Math::vec4>(convertedVal);
    int err = sqlite3_bind_blob(this->sqliteStatement, index + 1, &convertedVal, sizeof(val), SQLITE_TRANSIENT);
    n_assert(SQLITE_OK == err);
}

//------------------------------------------------------------------------------
/**
    Bind a Math::mat4 value by placeholder index. Either get the index from
    one of the IndexOf methods or use a 0-based index.
*/
void
Sqlite3Command::BindMat4(IndexT index, const Math::mat4& val)
{
    n_assert(0 != this->sqliteStatement);
    // ПРИМЕЧАНИЕ. Math::matrix44 будут сохранены в базе данных в виде больших двоичных объектов, поскольку 
    // Math::matrix44 может исчезнуть в любой момент, пусть SQLite создаст собственную копию данных.
    ByteOrder byteOrder(System::ByteOrder::LittleEndian, System::ByteOrder::Host);
    mat4 convertedVal = val;
    byteOrder.ConvertInPlace<Math::mat4>(convertedVal);
    int err = sqlite3_bind_blob(this->sqliteStatement, index + 1, &val, sizeof(val), SQLITE_TRANSIENT);
    n_assert(SQLITE_OK == err);
}

//------------------------------------------------------------------------------
/**
    Bind a string value by placeholder index. Either get the index from
    one of the IndexOf methods or use a 0-based index.. NOTE: the 
    string should be in UTF-8 format.
*/
void
Sqlite3Command::BindString(IndexT index, const Util::String& val)
{
    n_assert(0 != this->sqliteStatement);
    int err = sqlite3_bind_text(this->sqliteStatement, index + 1, val.AsCharPtr(), -1, SQLITE_TRANSIENT);
    n_assert(SQLITE_OK == err);
}

//------------------------------------------------------------------------------
/**
    Bind a string value by placeholder index. Either get the index from
    one of the IndexOf methods or use a 0-based index. index.
*/
void
Sqlite3Command::BindBlob(IndexT index, const Util::Blob& val)
{
    n_assert(0 != this->sqliteStatement);
    n_assert(val.Size() < INT_MAX);
    if (val.IsValid())
    {
    int err = sqlite3_bind_blob(this->sqliteStatement, index + 1, val.GetPtr(), (SizeT)val.Size(), SQLITE_TRANSIENT);
    n_assert(SQLITE_OK == err);
}
}

//------------------------------------------------------------------------------
/**
    Bind a guid value by placeholder index. Either get the index from
    one of the IndexOf methods or use a 0-based index. index.
*/
void
Sqlite3Command::BindGuid(IndexT index, const Util::Guid& val)
{
    n_assert(0 != this->sqliteStatement);
    // ПРИМЕЧАНИЕ. GUID будут храниться в базе данных в виде больших двоичных объектов!
    const unsigned char* ptr = 0;
    SizeT size = val.AsBinary(ptr);
    int err = sqlite3_bind_blob(this->sqliteStatement, index + 1, ptr, size, SQLITE_TRANSIENT);
    n_assert(SQLITE_OK == err);
}

//------------------------------------------------------------------------------
/**
*/
void
Sqlite3Command::BindInt(const Util::String& name, int val)
{
    Sqlite3Command::BindInt(Sqlite3Command::IndexOf(name), val);
}

//------------------------------------------------------------------------------
/**
*/
void
Sqlite3Command::BindFloat(const Util::String& name, float val)
{
    Sqlite3Command::BindFloat(Sqlite3Command::IndexOf(name), val);
}

//------------------------------------------------------------------------------
/**
*/
void
Sqlite3Command::BindBool(const Util::String& name, bool val)
{
    Sqlite3Command::BindBool(Sqlite3Command::IndexOf(name), val);
}

//------------------------------------------------------------------------------
/**
*/
void
Sqlite3Command::BindVec4(const Util::String& name, const vec4& val)
{
    Sqlite3Command::BindVec4(Sqlite3Command::IndexOf(name), val);
}

//------------------------------------------------------------------------------
/**
*/
void
Sqlite3Command::BindMat4(const Util::String& name, const Math::mat4& val)
{
    Sqlite3Command::BindMat4(Sqlite3Command::IndexOf(name), val);
}

//------------------------------------------------------------------------------
/**
*/
void
Sqlite3Command::BindString(const Util::String& name, const Util::String& val)
{
    Sqlite3Command::BindString(Sqlite3Command::IndexOf(name), val);
}

//------------------------------------------------------------------------------
/**
*/
void
Sqlite3Command::BindBlob(const Util::String& name, const Util::Blob& val)
{
    Sqlite3Command::BindBlob(Sqlite3Command::IndexOf(name), val);
}

//------------------------------------------------------------------------------
/**
*/
void
Sqlite3Command::BindGuid(const Util::String& name, const Util::Guid& val)
{
    Sqlite3Command::BindGuid(Sqlite3Command::IndexOf(name), val);
}

//------------------------------------------------------------------------------
/**
*/
void
Sqlite3Command::BindInt(const Attr::AttrId& id, int val)
{
    Sqlite3Command::BindInt(Sqlite3Command::IndexOf(id), val);
}

//------------------------------------------------------------------------------
/**
*/
void
Sqlite3Command::BindFloat(const Attr::AttrId& id, float val)
{
    Sqlite3Command::BindFloat(Sqlite3Command::IndexOf(id), val);
}

//------------------------------------------------------------------------------
/**
*/
void
Sqlite3Command::BindBool(const Attr::AttrId& id, bool val)
{
    Sqlite3Command::BindBool(Sqlite3Command::IndexOf(id), val);
}

//------------------------------------------------------------------------------
/**
*/
void
Sqlite3Command::BindVec4(const Attr::AttrId& id, const vec4& val)
{
    Sqlite3Command::BindVec4(Sqlite3Command::IndexOf(id), val);
}

//------------------------------------------------------------------------------
/**
*/
void
Sqlite3Command::BindMat4(const Attr::AttrId& id, const Math::mat4& val)
{
    Sqlite3Command::BindMat4(Sqlite3Command::IndexOf(id), val);
}

//------------------------------------------------------------------------------
/**
*/
void
Sqlite3Command::BindString(const Attr::AttrId& id, const Util::String& val)
{
    Sqlite3Command::BindString(Sqlite3Command::IndexOf(id), val);
}

//------------------------------------------------------------------------------
/**
*/
void
Sqlite3Command::BindBlob(const Attr::AttrId& id, const Util::Blob& val)
{
    Sqlite3Command::BindBlob(Sqlite3Command::IndexOf(id), val);
}

//------------------------------------------------------------------------------
/**
*/
void
Sqlite3Command::BindGuid(const Attr::AttrId& id, const Util::Guid& val)
{
    Sqlite3Command::BindGuid(Sqlite3Command::IndexOf(id), val);
}

} // пространство имен БД
