//------------------------------------------------------------------------------
//  zipfilestream.cc
//  (C) 2006 г. Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "io/zipfs/zipfilestream.h"
#include "io/zipfs/zipfilesystem.h"
#include "io/zipfs/ziparchive.h"
#include "io/zipfs/zipfileentry.h"
#include "io/archfs/archive.h"

namespace IO
{
__ImplementClass(IO::ZipFileStream, 'ZFST', IO::Stream);

using namespace Util;

//------------------------------------------------------------------------------
/**
*/
ZipFileStream::ZipFileStream() :
    size(0),
    position(0),
    mapBuffer(0)
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
ZipFileStream::~ZipFileStream()
{
    if (this->IsOpen())
    {
        this->Close();
    }
    n_assert(!this->mapBuffer);
}

//------------------------------------------------------------------------------
/**
*/
bool
ZipFileStream::CanRead() const
{
    return true;
}

//------------------------------------------------------------------------------
/**
*/
bool
ZipFileStream::CanWrite() const
{
    return false;
}

//------------------------------------------------------------------------------
/**
*/
bool
ZipFileStream::CanSeek() const
{
    return true;
}

//------------------------------------------------------------------------------
/**
*/
bool
ZipFileStream::CanBeMapped() const
{
    return true;
}

//------------------------------------------------------------------------------
/**
*/
Stream::Size
ZipFileStream::GetSize() const
{
    return this->size;
}

//------------------------------------------------------------------------------
/**
*/
Stream::Position
ZipFileStream::GetPosition() const
{
    return this->position;
}

//------------------------------------------------------------------------------
/**
    Open the stream for reading. This will decompress the entire file
    from the zip archive into memory.
*/
bool
ZipFileStream::Open()
{
    n_assert(!this->IsOpen());
    n_assert(!this->mapBuffer);
    // разрешить доступ только для чтения
    if (ReadAccess == this->accessMode)
    {
        if (Stream::Open())
        {
            // получить zip-архив, содержащий файл
            Ptr<ZipArchive> zipArchive = ZipFileSystem::Instance()->FindArchive(this->uri).cast<ZipArchive>();
            if (zipArchive.isvalid())
            {
                Dictionary<String,String> params = this->uri.ParseQuery();
                if (params.Contains("file"))
                {
                    const String& pathInZip = params["file"];
                    String pwd;
                    if (params.Contains("pwd"))
                    {
                        pwd = params["pwd"];
                    }

                    // найдите в архиве zip-файл, путь до него
                    // архив закодирован в части запроса нашего URI
                    this->zipFileEntry = zipArchive->FindFileEntry(pathInZip);
                    if (0 != this->zipFileEntry)
                    {
                        // прочитать содержимое записи zip-файла в частный буфер
                        this->size = this->zipFileEntry->GetFileSize();
                        if(!this->zipFileEntry->Open(pwd)) return false;
                        if(!this->CopyToMap()) return false;
                        this->position = 0;
                        this->zipFileEntry->Close();
                        this->zipFileEntry = nullptr;
                        return true;
                    }
                }
            }
            // провал: провал
            this->Close();
        }
    }    
    return false;
}

//------------------------------------------------------------------------------
/**
*/
void
ZipFileStream::Close()
{
    n_assert(this->IsOpen());
    if (this->IsMapped())
    {
        this->Unmap();
    }
    Memory::Free(Memory::StreamDataHeap, this->mapBuffer);
    this->mapBuffer = NULL;
    Stream::Close();
    this->size = 0;
    this->position = 0;
}

//------------------------------------------------------------------------------
/**
*/
Stream::Size
ZipFileStream::Read(void* ptr, Size numBytes)
{
    n_assert(ptr);
    n_assert(this->IsOpen());
    n_assert(ReadAccess == this->accessMode)
    n_assert((this->position >= 0) && (this->position <= this->size));

    // проверьте, близок ли конец потока
    Size readBytes = Math::min(numBytes, this->size - this->position);
    n_assert((this->position + readBytes) <= this->size);
    if (readBytes > 0)
    {
        Memory::Copy(this->mapBuffer + this->position, ptr, readBytes);
        this->position += readBytes;
    }
    return readBytes;
}

//------------------------------------------------------------------------------
/**
*/
void
ZipFileStream::Seek(Offset offset, SeekOrigin origin)
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
        default:
            n_assert(false);
    }

    // убедитесь, что позиция чтения/записи не стала недействительной
    this->position = Math::clamp(this->position, (Stream::Size)0, this->size);
}

//------------------------------------------------------------------------------
/**
*/
bool
ZipFileStream::Eof() const
{
    n_assert(this->IsOpen());
    //n_assert(!this->IsMapped());
    n_assert((this->position >= 0) && (this->position <= this->size));
    return (this->position == this->size);
}

//------------------------------------------------------------------------------
/**
*/
void*
ZipFileStream::Map()
{
    n_assert(this->IsOpen());
    n_assert(this->mapBuffer != nullptr);
    n_assert(ReadAccess == this->accessMode);
    Stream::Map();
    return this->mapBuffer;
}

//------------------------------------------------------------------------------
/**
*/
void
ZipFileStream::Unmap()
{
    n_assert(this->IsOpen());
    Stream::Unmap();
}

//------------------------------------------------------------------------------
/**
*/
void*
ZipFileStream::MemoryMap()
{
    return this->Map();
}

//------------------------------------------------------------------------------
/**
*/
void
ZipFileStream::MemoryUnmap()
{
    return this->Unmap();
}

//------------------------------------------------------------------------------
/**
*/
bool 
ZipFileStream::CopyToMap()
{
    n_assert(this->IsOpen());
    n_assert(this->GetSize() > 0);
    n_assert(!this->mapBuffer);
    this->mapBuffer = (unsigned char*)Memory::Alloc(Memory::StreamDataHeap, this->size);
    n_assert(0 != this->mapBuffer);
    return this->zipFileEntry->Read(this->mapBuffer, this->size);
}
} // пространство имен ввода-вывода
