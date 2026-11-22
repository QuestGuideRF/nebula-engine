//------------------------------------------------------------------------------
//  modelwriter.cc
//  (C) 2008 г. Радон Лабс ГмбХ
//  (C) 2013–2016 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "foundation/stdneb.h"
#include "modelwriter.h"

namespace ToolkitUtil
{
__ImplementAbstractClass(ToolkitUtil::ModelWriter, 'MDLW', IO::StreamWriter);

//------------------------------------------------------------------------------
/**
*/
ModelWriter::ModelWriter() :
    platform(Platform::Win32),
    version(1)
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
ModelWriter::~ModelWriter()
{
    // empty
}

} // пространство имен ModelWriter