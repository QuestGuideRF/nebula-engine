//------------------------------------------------------------------------------
//  reader.cc
//  (C) 2006 г. Радон Лабс ГмбХ
//  (C) 2013–2016 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "foundation/stdneb.h"
#include "db/reader.h"
#include "db/dbserver.h"

namespace Db
{
__ImplementClass(Db::Reader, 'DBRE', Core::RefCounted);

//------------------------------------------------------------------------------
/**
*/
Reader::Reader() :
    isOpen(false),
    curRowIndex(InvalidIndex)
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
Reader::~Reader()
{
    if (this->isOpen)
    {
        this->Close();
    }
}

//------------------------------------------------------------------------------
/**
    Откройте читалку. Это создаст набор данных, который выполняет запрос
    в базе данных.
*/
bool
Reader::Open()
{
    n_assert(!this->isOpen);
    n_assert(this->database.isvalid());
    
    // получить базу данных с сервера БД
    if (!this->database->HasTable(this->tableName))
    {
        n_error("Db::Reader(): в базе данных «%s» нет таблицы «%s»!", 
            this->database->GetURI().AsString().AsCharPtr(),
            this->tableName.AsCharPtr());
        return false;
    }
    
    // создать и настроить объект набора данных
    Ptr<Table> table = this->database->GetTableByName(this->tableName);
    this->dataset = table->CreateDataset();
    this->dataset->AddAllTableColumns();
    IndexT i;
    IndexT num = this->filterAttrs.Size();
    for (i = 0; i < this->filterAttrs.Size(); i++)
    {
        this->dataset->Filter()->AddEqualCheck(this->filterAttrs[i]);
        if (i < (num - 1))
        {
            this->dataset->Filter()->AddAnd();
        }
    }

    // выполнить запрос к базе данных
    this->dataset->PerformQuery();
    this->valueTable = this->dataset->Values();

    this->isOpen = true;
    return true;
}

//------------------------------------------------------------------------------
/**
    Закройте читалку. При этом объект запроса, созданный в
    конструктор.
*/
void
Reader::Close()
{
    n_assert(this->isOpen);
    this->valueTable = nullptr;
    this->dataset = nullptr;
    this->database = nullptr;
    this->isOpen = false;
}

//------------------------------------------------------------------------------
/**
    Возвращайте true, если определенный атрибут существует в текущей строке.
*/
bool
Reader::HasAttr(Attr::AttrId attrId) const
{
    n_assert(this->isOpen);
    return this->valueTable->HasColumn(attrId);
}

//------------------------------------------------------------------------------
/**
*/
bool
Reader::GetBool(Attr::BoolAttrId attrId) const
{
    n_assert(this->isOpen);
    return this->valueTable->GetBool(attrId, this->curRowIndex);
}

//------------------------------------------------------------------------------
/**
*/
int
Reader::GetInt(Attr::IntAttrId attrId) const
{
    n_assert(this->isOpen);
    return this->valueTable->GetInt(attrId, this->curRowIndex);
}

//------------------------------------------------------------------------------
/**
*/
float
Reader::GetFloat(Attr::FloatAttrId attrId) const
{
    n_assert(this->isOpen);
    return this->valueTable->GetFloat(attrId, this->curRowIndex);
}

//------------------------------------------------------------------------------
/**
*/
const Util::String&
Reader::GetString(Attr::StringAttrId attrId) const
{
    n_assert(this->isOpen);
    return this->valueTable->GetString(attrId, this->curRowIndex);
}

//------------------------------------------------------------------------------
/**
*/
const Math::vec4
Reader::GetVec4(Attr::Vec4AttrId attrId) const
{
    n_assert(this->isOpen);
    return this->valueTable->GetVec4(attrId, this->curRowIndex);
}

//------------------------------------------------------------------------------
/**
*/
const Math::mat4
Reader::GetMat4(Attr::Mat4AttrId attrId) const
{
    n_assert(this->isOpen);
    return this->valueTable->GetMat4(attrId, this->curRowIndex);
}

//------------------------------------------------------------------------------
/**
*/
const Util::Guid&
Reader::GetGuid(Attr::GuidAttrId attrId) const
{
    n_assert(this->isOpen);
    return this->valueTable->GetGuid(attrId, this->curRowIndex);
}

//------------------------------------------------------------------------------
/**
*/
const Util::Blob&
Reader::GetBlob(Attr::BlobAttrId attrId) const
{
    n_assert(this->isOpen);
    return this->valueTable->GetBlob(attrId, this->curRowIndex);
}

}; // пространство имен БД
