//------------------------------------------------------------------------------
//  stream.cc
//  (C) 2006 г. Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "io/stream.h"

namespace IO
{
__ImplementClass(IO::Stream, 'STRM', Core::RefCounted);

using namespace Util;

//------------------------------------------------------------------------------
/**
*/
Stream::Stream() :
    accessMode(ReadAccess),
    accessPattern(Sequential),
    isOpen(false),
    isMapped(false)
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
Stream::~Stream()
{
    n_assert(!this->IsOpen());
    n_assert(!this->IsMapped());
}

//------------------------------------------------------------------------------
/**
    Установите URI потока в виде строки. URI идентифицирует источник
    ресурс потока.
*/
void
Stream::SetURI(const URI& u)
{
    n_assert(!this->IsOpen());
    this->uri = u;
}

//------------------------------------------------------------------------------
/**
    Получите URI потока в виде строки.
*/
const URI&
Stream::GetURI() const
{
    return this->uri;
}

//------------------------------------------------------------------------------
/**
    Этот метод должен возвращать значение true, если производный класс потока поддерживает чтение.
*/
bool
Stream::CanRead() const
{
    return false;
}

//------------------------------------------------------------------------------
/**
    Этот метод должен возвращать true, если производный класс потока поддерживает запись.
*/
bool
Stream::CanWrite() const
{
    return false;
}

//------------------------------------------------------------------------------
/**
    Этот метод должен возвращать значение true, если производный поток поддерживает поиск.
*/
bool
Stream::CanSeek() const
{
    return false;
}

//------------------------------------------------------------------------------
/**
    This method must return true if the stream supports direct memory
    access through the Map()/Unmap() methods.
*/
bool
Stream::CanBeMapped() const
{
    return false;
}

//------------------------------------------------------------------------------
/**
    Это устанавливает новый размер потока. Не все потоки поддерживают этот метод.
    Если новый размер меньше существующего, содержимое будет
    быть обрезанным.
*/
void
Stream::SetSize(Size /*s*/)
{
    n_assert(!this->IsOpen());
}

//------------------------------------------------------------------------------
/**
    Этот метод возвращает размер потока в байтах.
*/
Stream::Size
Stream::GetSize() const
{
    return 0;
}

//------------------------------------------------------------------------------
/**
    Этот метод возвращает текущую позицию курсора чтения/записи.
*/
Stream::Position
Stream::GetPosition() const
{
    return 0;
}

//------------------------------------------------------------------------------
/**
    Этот метод устанавливает предполагаемый режим доступа к потоку. Фактический
    поведение зависит от реализации производного класса.
    по умолчанию — ReadWrite.
*/
void
Stream::SetAccessMode(AccessMode m)
{
    n_assert(!this->IsOpen());
    this->accessMode = m;
}

//------------------------------------------------------------------------------
/**
    Получите режим доступа потока.
*/
Stream::AccessMode
Stream::GetAccessMode() const
{
    return this->accessMode;
}

//------------------------------------------------------------------------------
/**
    Set the prefered access pattern of the stream. This can be Random or
    Sequential. This is an optional flag to improve performance with
    some stream implementations. The default is sequential. The pattern
    cannot be changed while the stream is open.
*/
void
Stream::SetAccessPattern(AccessPattern p)
{
    n_assert(!this->IsOpen());
    this->accessPattern = p;
}

//------------------------------------------------------------------------------
/**
    Получите текущий установленный предпочтительный шаблон доступа к потоку.
*/
Stream::AccessPattern
Stream::GetAccessPattern() const
{
    return this->accessPattern;
}

//------------------------------------------------------------------------------
/**
*/
void
Stream::SetMediaType(const MediaType& t)
{
    this->mediaType = t;
}

//------------------------------------------------------------------------------
/**
*/
const MediaType&
Stream::GetMediaType() const
{
    return this->mediaType;
}

//------------------------------------------------------------------------------
/**
    Откройте поток. Только один поток может открыть поток в любой момент времени.
    Возвращает true в случае успеха.
*/
bool
Stream::Open()
{
    n_assert(!this->IsOpen());
    this->isOpen = true;
    return true;
}

//------------------------------------------------------------------------------
/**
    Закрывает поток.
*/
void
Stream::Close()
{
    n_assert(this->IsOpen());
    this->isOpen = false;
}

//------------------------------------------------------------------------------
/**
    Верните true, если поток в данный момент открыт.
*/
bool
Stream::IsOpen() const
{
    return this->isOpen;
}

//------------------------------------------------------------------------------
/**
    Write raw data to the stream. For more convenient writing, attach the
    stream to an IO::StreamWriter object. This method is only valid if the 
    stream class returns true in CanWrite().
*/
void
Stream::Write(const void* /*ptr*/, Size /*numBytes*/)
{
    n_assert(this->IsOpen());
    n_assert(!this->isMapped);
}

//------------------------------------------------------------------------------
/**
    Read raw data from the stream. For more convenient reading, attach
    the stream to an IO::StreamReader object. The method returns the number
    of bytes actually read. This method is only valid if the stream
    class returns true in CanRead(). Returns the number of bytes actually
    read from the stream, this may be less then numBytes, or 0 if end-of-stream
    is reached.
*/
Stream::Size
Stream::Read(void* /*ptr*/, Size /*numBytes*/)
{
    n_assert(this->IsOpen());
    n_assert(!this->isMapped);
    return 0;
}

//------------------------------------------------------------------------------
/**
    Move the read/write cursor to a new position, returns the new position
    in the stream. This method is only supported if the stream class
    returns true in CanSeek().
*/
void
Stream::Seek(Offset /*offset*/, SeekOrigin /*origin*/)
{
    n_assert(this->IsOpen());
    n_assert(!this->isMapped);
}

//------------------------------------------------------------------------------
/**
    Сбросьте все несохраненные данные. Обратите внимание, что несохраненные данные также будут удалены.
    автоматически при закрытии потока.
*/
void
Stream::Flush()
{
    n_assert(this->IsOpen());
    n_assert(!this->isMapped);
}

//------------------------------------------------------------------------------
/**
    Возвращайте true, если курсор чтения/записи находится в конце потока.
*/
bool
Stream::Eof() const
{
    n_assert(this->IsOpen());
    n_assert(!this->isMapped);
    return true;
}

//------------------------------------------------------------------------------
/**
    If the stream provides memory mapping, this method will return a pointer
    to the beginning of the stream data in memory. The application is 
    free to read and write to the stream through direct memory access. Special
    care must be taken to not read or write past the end of the mapped data
    (indicated by GetSize()). The normal Read()/Write() method are not
    valid while the stream is mapped, also the read/write cursor position
    will not be updated.
*/
void*
Stream::Map()
{
    n_assert(this->IsOpen());
    n_assert(this->CanBeMapped());
    n_assert(!this->isMapped);
    this->isMapped = true;
    return 0;
}

//------------------------------------------------------------------------------
/**
    Это приведет к отмене сопоставления потока, отображенного в памяти.
*/
void
Stream::Unmap()
{
    n_assert(this->IsOpen());
    n_assert(this->CanBeMapped());
    n_assert(this->isMapped);
    this->isMapped = false;
}

//------------------------------------------------------------------------------
/**
*/
void* 
Stream::MemoryMap()
{
    n_assert(this->IsOpen());
    n_assert(this->CanBeMapped());
    n_assert(!this->isMapped);
    this->isMapped = true;
    return 0;
}

//------------------------------------------------------------------------------
/**
*/
void 
Stream::MemoryUnmap()
{
    n_assert(this->IsOpen());
    n_assert(this->CanBeMapped());
    n_assert(this->isMapped);
    this->isMapped = false;
}

//------------------------------------------------------------------------------
/**
    Возвращает true, если поток в данный момент сопоставлен.
*/
bool
Stream::IsMapped() const
{
    return this->isMapped;
}


} // пространство имен ввода-вывода
