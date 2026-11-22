//------------------------------------------------------------------------------
//  memorystream.cc
//  (C) 2006 г. Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "io/memorystream.h"

namespace IO
{
__ImplementClass(IO::MemoryStream, 'MSTR', IO::Stream);

//------------------------------------------------------------------------------
/**
*/
MemoryStream::MemoryStream() :
    capacity(0),
    size(0),
    position(0),
    buffer(0)
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
MemoryStream::~MemoryStream()
{
    // закройте поток, если он еще открыт
    if (this->IsOpen())
    {
        this->Close();
    }

    // освободить буфер памяти, если он выделен
    if (0 != this->buffer)
    {
        Memory::Free(Memory::StreamDataHeap, this->buffer);
        this->buffer = 0;
    }
}

//------------------------------------------------------------------------------
/**
*/
bool
MemoryStream::CanRead() const
{
    return true;
}

//------------------------------------------------------------------------------
/**
*/
bool
MemoryStream::CanWrite() const
{
    return true;
}

//------------------------------------------------------------------------------
/**
*/
bool
MemoryStream::CanSeek() const
{
    return true;
}

//------------------------------------------------------------------------------
/**
*/
bool
MemoryStream::CanBeMapped() const
{
    return true;
}

//------------------------------------------------------------------------------
/**
*/
void
MemoryStream::SetSize(Size s)
{
    if (s > this->capacity)
    {
        this->Realloc(s);
    }
    this->size = s;
}

//------------------------------------------------------------------------------
/**
*/
Stream::Size
MemoryStream::GetSize() const
{
    return this->size;
}

//------------------------------------------------------------------------------
/**
*/
Stream::Position
MemoryStream::GetPosition() const
{
    return this->position;
}

//------------------------------------------------------------------------------
/**
    Откройте поток для чтения или записи. Поток может уже содержать
    данные, если они были открыты/закрыты ранее. 
*/
bool
MemoryStream::Open()
{
    n_assert(!this->IsOpen());
    
    // здесь делать нечего, выделение происходит при первом вызове Write()
    // if necessary, all we do is reset the read/write position to the
    // начало потока
    if (Stream::Open())
    {
        if (WriteAccess == this->accessMode)
        {
            this->position = 0;
        }
        else if (AppendAccess == this->accessMode)
        {
            this->position = this->size;
        }
        else
        {
            this->position = 0;
        }
        return true;
    }
    return false;
}

//------------------------------------------------------------------------------
/**
    Закройте поток. Содержимое потока останется нетронутым до тех пор, пока
    уничтожение объекта, чтобы можно было получить доступ к тем же данным 
    или изменены во время более позднего сеанса. 
*/
void
MemoryStream::Close()
{
    n_assert(this->IsOpen());
    if (this->IsMapped())
    {
        this->Unmap();
    }
    Stream::Close();
}

//------------------------------------------------------------------------------
/**
*/
void
MemoryStream::Write(const void* ptr, Size numBytes)
{
    n_assert(this->IsOpen());
    n_assert(!this->IsMapped()); 
    n_assert((WriteAccess == this->accessMode) || (AppendAccess == this->accessMode) || (ReadWriteAccess == this->accessMode))
    n_assert((this->position >= 0) && (this->position <= this->size));

    // if not enough room, allocate more memory
    if (!this->HasRoom(numBytes))
    {
        this->MakeRoom(numBytes);
    }

    // записывать данные в поток
    n_assert((this->position + numBytes) <= this->capacity);
    Memory::Copy(ptr, this->buffer + this->position, numBytes);
    this->position += numBytes;
    if (this->position > this->size)
    {
        this->size = this->position;
    }
}

//------------------------------------------------------------------------------
/**
*/
Stream::Size
MemoryStream::Read(void* ptr, Size numBytes)
{
    n_assert(this->IsOpen());
    n_assert(!this->IsMapped()); 
    n_assert((ReadAccess == this->accessMode) || (ReadWriteAccess == this->accessMode))
    n_assert((this->position >= 0) && (this->position <= this->size));

    // проверьте, близок ли конец потока
    Size readBytes = numBytes <= this->size - this->position ? numBytes : this->size - this->position;
    n_assert((this->position + readBytes) <= this->size);
    if (readBytes > 0)
    {
        Memory::Copy(this->buffer + this->position, ptr, readBytes);
        this->position += readBytes;
    }
    return readBytes;
}

//------------------------------------------------------------------------------
/**
*/
void
MemoryStream::Seek(Offset offset, SeekOrigin origin)
{
    n_assert(this->IsOpen());
    n_assert(!this->IsMapped()); 
    n_assert((this->position >= 0) && (this->position <= this->size));
    switch (origin)
    {
        case Begin:
            this->position = offset;
            break;
        case Current:
            this->position += offset;
            break;
        case End:
            this->position = this->size + offset;
            break;
    }

    // убедитесь, что позиция чтения/записи не стала недействительной
    this->position = Math::clamp(this->position, (Stream::Size)0, this->size);
}

//------------------------------------------------------------------------------
/**
*/
bool
MemoryStream::Eof() const
{
    n_assert(this->IsOpen());
    n_assert(!this->IsMapped());
    n_assert((this->position >= 0) && (this->position <= this->size));
    return (this->position == this->size);
}

//------------------------------------------------------------------------------
/**
*/
bool
MemoryStream::HasRoom(Size numBytes) const
{
    return ((this->position + numBytes) <= this->capacity);
}

//------------------------------------------------------------------------------
/**
    This (re-)allocates the memory buffer to a new size. If the new size
    is smaller then the existing size, the buffer contents will be clipped.
*/
void
MemoryStream::Realloc(Size newCapacity)
{
    unsigned char* newBuffer = (unsigned char*) Memory::Alloc(Memory::StreamDataHeap, newCapacity);
    unsigned char* endOfNewBuffer = newBuffer + newCapacity;
    n_assert(0 != newBuffer);
    Size newSize = newCapacity < this->size ? newCapacity : this->size;
    if (0 != this->buffer)
    {
        n_assert((newBuffer + newSize) < endOfNewBuffer);
        Memory::Copy(this->buffer, newBuffer, newSize);
        Memory::Free(Memory::StreamDataHeap, this->buffer);
    }
    this->buffer = newBuffer;
    this->size = newSize;
    this->capacity = newCapacity;
    if (this->position > this->size)
    {
        this->position = this->size;
    }
}

//------------------------------------------------------------------------------
/**
    Этот метод освобождает место как минимум для N дополнительных байтов. Фактически выделено
    буфер памяти будет больше. Эта операция предполагает копирование
    существующих данных.
*/
void
MemoryStream::MakeRoom(Size numBytes)
{
    n_assert(numBytes > 0);
    n_assert((this->size + numBytes) > this->capacity);

    // вычислить новую емкость
    Size oneDotFiveCurrentSize = this->capacity + (this->capacity >> 1);
    Size newCapacity = this->size + numBytes;
    if (oneDotFiveCurrentSize > newCapacity)
    {
        newCapacity = oneDotFiveCurrentSize;
    }
    if (16 > newCapacity)
    {
        newCapacity = 16;
    }
    n_assert(newCapacity > this->capacity);

    // (пере)выделить буфер памяти
    this->Realloc(newCapacity);
}

//------------------------------------------------------------------------------
/**
    Map the stream for direct memory access. This is much faster then 
    reading/writing, but less flexible. A mapped stream cannot grow, instead
    the allowed memory range is determined by GetSize(). The read/writer must 
    take special care to not read or write past the memory buffer boundaries!
*/
void*
MemoryStream::Map()
{
    n_assert(this->IsOpen());
    Stream::Map();
    n_assert(this->GetSize() > 0);
    return this->buffer;
}

//------------------------------------------------------------------------------
/**
  
*/
void*
MemoryStream::MemoryMap()
{
    return this->Map();
}

//------------------------------------------------------------------------------
/**
    Отмените отображение потока, отображенного в памяти.
*/
void
MemoryStream::Unmap()
{
    n_assert(this->IsOpen());
    Stream::Unmap();
}

//------------------------------------------------------------------------------
/**
    Получите прямой указатель на необработанные данные. Это удобный метод
    и работает только для потоков памяти.
    ПРИМЕЧАНИЕ. запись новых данных в поток может/приведет к недопустимому результату.
    указатель, не сохраняйте возвращенный указатель между операциями записи!
*/
void*
MemoryStream::GetRawPointer() const
{
    n_assert(0 != this->buffer);
    return this->buffer;
}

} // пространство имен ввода-вывода
