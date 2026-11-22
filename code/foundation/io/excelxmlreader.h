#pragma once
#include "io/streamreader.h"
#include "io/xmlreader.h"
#include "util/string.h"
#include "util/array.h"
#include "util/fixedtable.h"
namespace IO
{
class ExcelXmlReader : public StreamReader
{
    __DeclareClass(ExcelXmlReader);
public:
    ExcelXmlReader();
    virtual ~ExcelXmlReader();
    virtual bool Open();
    virtual void Close();
    SizeT GetNumTables() const;
    const Util::String& GetTableName(IndexT tableIndex = 0) const;
    IndexT GetTableIndex(const Util::String& tableName);
    SizeT GetNumRows(IndexT tableIndex = 0) const;
    SizeT GetNumColumns(IndexT tableIndex = 0) const;
    bool HasColumn(const Util::String& columnName, IndexT tableIndex = 0) const;
    IndexT FindColumnIndex(const Util::String& columnName, IndexT tableIndex = 0) const;
    const Util::String& GetElement(IndexT rowIndex, IndexT columnIndex, IndexT tableIndex = 0) const;
    const Util::String& GetElement(IndexT rowIndex, const Util::String& columnName, IndexT tableIndex = 0) const;
private:
    bool ParseExcelXmlStream();
    bool ParseTables(const Ptr<IO::XmlReader>& xmlReader);
    Util::Array<Util::String> tableNames;
    Util::Array<Util::FixedTable<Util::String> > tables;
};
}