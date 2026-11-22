//------------------------------------------------------------------------------
//  excelxmlreader.cc
//  (C) 2008 г. Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "io/excelxmlreader.h"

namespace IO
{
__ImplementClass(IO::ExcelXmlReader, 'EXRD', IO::StreamReader);

using namespace Util;

//------------------------------------------------------------------------------
/**
*/
ExcelXmlReader::ExcelXmlReader()
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
ExcelXmlReader::~ExcelXmlReader()
{
    if (this->IsOpen())
    {
        this->Close();
    }
}

//------------------------------------------------------------------------------
/**
    Откройте поток Excel-XML и полностью проанализируйте его содержимое.
*/
bool
ExcelXmlReader::Open()
{
    n_assert(this->tableNames.Size() == 0);
    n_assert(this->tables.Size() == 0);

    if (StreamReader::Open())
    {
        // проанализировать XML-поток Excel
        if (this->ParseExcelXmlStream())
        {
            // поскольку файл Excel теперь загружен, мы можем закрыть исходный поток
            if (!this->streamWasOpen)
            {
                this->stream->Close();
            }

            // все ок
            return true;
        }
    }
    return false;
}

//------------------------------------------------------------------------------
/**
*/
void
ExcelXmlReader::Close()
{
    this->tableNames.Clear();
    this->tables.Clear();
    StreamReader::Close();
}

//------------------------------------------------------------------------------
/**
    This loads the Excel-XML file into a TinyXML document and reads the
    content into 2D string tables.
*/
bool
ExcelXmlReader::ParseExcelXmlStream()
{
    bool success = false;

    // настроить программу чтения потоков XML
    Ptr<XmlReader> xmlReader = XmlReader::Create();
    xmlReader->SetStream(this->stream);
    if (xmlReader->Open())
    {
        if (this->ParseTables(xmlReader))
        {
            success = true;
        }
        xmlReader->Close();
    }
    else
    {
        n_error("ExcelXmlReader::ParseExcelXmlStream(): не удалось загрузить '%s'!", this->stream->GetURI().AsString().AsCharPtr());
        success = false;
    }
    return success;
}

//------------------------------------------------------------------------------
/**
    Setup the table data from a loaded Excel XML file.
*/
bool
ExcelXmlReader::ParseTables(const Ptr<XmlReader>& xmlReader)
{
    n_assert(this->tables.Size() == 0);
    n_assert(this->tableNames.Size() == 0);

    // убедитесь, что это действительно XML-файл Excel
    if (!xmlReader->HasNode("/Workbook"))
    {
        n_error("ExcelXmlReader: «%s» не является XML-файлом Excel!", this->stream->GetURI().AsString().AsCharPtr());
        return false;
    }
    xmlReader->SetToNode("/Workbook");

    // предварительно определите несколько строковых объектов для следующего цикла
    String worksheetString("Worksheet");
    String nameAttrString("ss:Name");
    String tableString("Table");
    String rowString("Row");
    String cellString("Cell");
    String dataString("Data");
    String cellDataString("Cell/Data");
    String contentString;

    // подсчитать количество таблиц в файле
    SizeT numTables = 0;
    if (xmlReader->SetToFirstChild(worksheetString)) do
    {
        numTables++;
    }
    while (xmlReader->SetToNextChild(worksheetString));
    n_assert(numTables > 0);
    this->tableNames.Reserve(numTables);
    this->tables.Reserve(numTables);

    // for each table...
    IndexT curTableIndex = 0;

    if (xmlReader->SetToFirstChild(worksheetString)) do
    {
        // получить имя текущего рабочего листа
        String tableName = xmlReader->GetString(nameAttrString.AsCharPtr());
        this->tableNames.Append(tableName);
        n_assert((this->tableNames.Size() - 1) == curTableIndex);
        xmlReader->SetToFirstChild(tableString);

        // подсчитать количество допустимых строк и столбцов и допустимых столбцов, мы не можем
        // просто используйте expendedRowCount иexpandedColumnCount
        // атрибуты, поскольку они включают ячейки таблицы без каких-либо данных
        IndexT numColumns = 0;
        IndexT numRows = 0;
        if (xmlReader->SetToFirstChild(rowString))do
        {
            // первый ряд? если да, определите количество столбцов,
            // столбец действителен только в том случае, если в первой строке есть данные
            if (0 == numRows)
            {
                n_assert(0 == numColumns);
                if (xmlReader->SetToFirstChild(cellString)) do
                {
                    if (xmlReader->HasNode(dataString))
                    {
                        numColumns++;
                    }
                    else
                    {
                        // остановитесь на первом столбце, в первой строке которого нет данных
                        xmlReader->SetToParent();
                        break;
                    }
                }
                while (xmlReader->SetToNextChild(cellString));
            }

            // увеличить количество строк, строка действительна только в том случае, если она имеет
            // действительные данные в первом столбце
            if (xmlReader->HasNode(cellDataString))
            {
                numRows++;
            }
            else
            {
                // остановитесь на первой строке, в первом столбце которой нет данных
                xmlReader->SetToParent();
                break;
            }
        }
        while (xmlReader->SetToNextChild(rowString));

        // настроить двумерную таблицу строк с количеством строк и столбцов и прочитать фактические данные
        if ((numRows > 0) && (numColumns > 0))
        {
            FixedTable<String> newTable;
            this->tables.Append(newTable);
            n_assert((this->tables.Size() - 1) == curTableIndex);
            this->tables[curTableIndex].SetSize(numColumns, numRows);

            // прочитать фактические данные таблицы
            IndexT curRowIndex = 0;
            if (xmlReader->SetToFirstChild(rowString))do
            {
                IndexT curColumnIndex = 0;
                if (xmlReader->SetToFirstChild(cellString)) do
                {
                    // проверьте, пропущены ли ячейки
                    if (xmlReader->HasAttr("ss:Index"))
                    {
                        // перейти к столбцу
                        curColumnIndex = xmlReader->GetInt("ss:Index")-1;
                    }

                    // уйти пораньше, если мы закончили количество столбцов
                    if (curColumnIndex >= numColumns)
                    {
                        xmlReader->SetToParent();
                        break;
                    }

                    // проверьте, существует ли узел данных
                    if (xmlReader->HasNode(dataString))
                    {
                        xmlReader->SetToNode(dataString);
                        if (xmlReader->HasContent())
                        {
                            this->tables[curTableIndex].Set(curColumnIndex, curRowIndex, xmlReader->GetContent());
                        }
                        xmlReader->SetToParent();
                    }
                    // if no data note exists, fill with empty string
                    else
                    {
                        this->tables[curTableIndex].Set(curColumnIndex, curRowIndex, "");
                    }
                    curColumnIndex++;
                }
                while (xmlReader->SetToNextChild(cellString));
                curRowIndex++;
                if (curRowIndex == numRows)
                {
                    xmlReader->SetToParent();
                    break;
                }
            }
            while (xmlReader->SetToNextChild(rowString));
        }
        xmlReader->SetToParent(); // из «Стола»
        curTableIndex++;
    }
    while (xmlReader->SetToNextChild(worksheetString));
    return true;
}

//------------------------------------------------------------------------------
/**
*/
SizeT
ExcelXmlReader::GetNumTables() const
{
    n_assert(this->IsOpen());
    return this->tables.Size();
}

//------------------------------------------------------------------------------
/**
*/
const String&
ExcelXmlReader::GetTableName(IndexT tableIndex) const
{
    n_assert(this->IsOpen());
    return this->tableNames[tableIndex];
}

//------------------------------------------------------------------------------
/**
*/
SizeT
ExcelXmlReader::GetNumRows(IndexT tableIndex) const
{
    n_assert(this->IsOpen());
    return this->tables[tableIndex].Height();
}

//------------------------------------------------------------------------------
/**
*/
SizeT
ExcelXmlReader::GetNumColumns(IndexT tableIndex) const
{
    n_assert(this->IsOpen());
    return this->tables[tableIndex].Width();
}

//------------------------------------------------------------------------------
/**
    ПРИМЕЧАНИЕ. этот метод медленный, поскольку он выполняет линейный поиск по столбцу.
    имена.
*/
bool
ExcelXmlReader::HasColumn(const String& columnName, IndexT tableIndex) const
{
    n_assert(this->IsOpen());
    return InvalidIndex != this->FindColumnIndex(columnName, tableIndex);
}

//------------------------------------------------------------------------------
/**
    ПРИМЕЧАНИЕ. этот метод медленный, поскольку он выполняет линейный поиск по столбцу.
    имена.
*/
IndexT
ExcelXmlReader::FindColumnIndex(const String& columnName, IndexT tableIndex) const
{
    n_assert(columnName.IsValid());
    n_assert(this->IsOpen());
    const FixedTable<String>& table = this->tables[tableIndex];
    IndexT colIndex;
    for (colIndex = 0; colIndex < table.Width(); colIndex++)
    {
        if (table.At(colIndex, 0) == columnName)
        {
            return colIndex;
        }
    }
    // провал: не найден
    return InvalidIndex;
}

//------------------------------------------------------------------------------
/**
*/
const String&
ExcelXmlReader::GetElement(IndexT rowIndex, IndexT columnIndex, IndexT tableIndex) const
{
    n_assert(this->IsOpen());
    return this->tables[tableIndex].At(columnIndex, rowIndex);
}

//------------------------------------------------------------------------------
/**
*/
const String&
ExcelXmlReader::GetElement(IndexT rowIndex, const String& columnName, IndexT tableIndex) const
{
    n_assert(this->IsOpen());
    IndexT columnIndex = this->FindColumnIndex(columnName, tableIndex);
    return this->tables[tableIndex].At(columnIndex, rowIndex);
}

//------------------------------------------------------------------------------
/**
*/
IndexT 
ExcelXmlReader::GetTableIndex(const Util::String& tableName)
{
    return this->tableNames.FindIndex(tableName);
}

} // пространство имен ввода-вывода
