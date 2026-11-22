//------------------------------------------------------------------------------
//  zipfileentry.cc
//  (C) 2006 г. Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "io/zipfs/zipfileentry.h"

namespace IO
{
using namespace Util;
using namespace Threading;

//------------------------------------------------------------------------------
/**
*/
ZipFileEntry::ZipFileEntry() :
    archiveCritSect(0),
    zipFileHandle(0),
    uncompressedSize(0)
{
    Memory::Clear(&this->filePosInfo, sizeof(this->filePosInfo));
}

//------------------------------------------------------------------------------
/**
*/
ZipFileEntry::~ZipFileEntry()
{
    this->zipFileHandle = 0;
}

//------------------------------------------------------------------------------
/**
*/
void
ZipFileEntry::Setup(const StringAtom& n, unzFile h, CriticalSection* critSect)
{
    n_assert(0 != h);
    n_assert(0 == this->zipFileHandle);
    n_assert(0 != critSect);

    this->name = n;
    this->zipFileHandle = h;

    // сохранить указатель на критическую секцию архива
    this->archiveCritSect = critSect;

    // сохранить позицию файла
    int res = unzGetFilePos64(this->zipFileHandle, &this->filePosInfo);
    n_assert(UNZ_OK == res);

    // получить другие данные о файле
    unz_file_info64 fileInfo;
    res = unzGetCurrentFileInfo64(this->zipFileHandle, &fileInfo, 0, 0, 0, 0, 0, 0);
    n_assert(UNZ_OK == res);
    this->uncompressedSize = fileInfo.uncompressed_size;
}

//------------------------------------------------------------------------------
/**
*/
bool
ZipFileEntry::Open(const String& password)
{
    // критическая секция активна до тех пор, пока не будет вызвано close или эта функция не завершится сбоем
    this->archiveCritSect->Enter();

    // установить текущий файл в этот файл
    int res = unzGoToFilePos64(this->zipFileHandle, const_cast<unz64_file_pos*>(&this->filePosInfo));
    if (UNZ_OK != res) 
    {
        this->archiveCritSect->Leave();
        return false;
    }

    // открыть текущий файл с необязательным паролем
    if (password.IsValid())
    {
        res = unzOpenCurrentFilePassword(this->zipFileHandle, password.AsCharPtr());
    }
    else
    {
        res = unzOpenCurrentFile(this->zipFileHandle);
    }

    if (UNZ_OK != res) 
    {
        this->archiveCritSect->Leave();
        return false;
    }

    return true;
}

//------------------------------------------------------------------------------
/**
*/
void
ZipFileEntry::Close()
{
    // закрыть файл
    int res = unzCloseCurrentFile(this->zipFileHandle);
    n_assert(UNZ_OK == res);

    // освободить критическую секцию
    this->archiveCritSect->Leave();
}

//------------------------------------------------------------------------------
/**
*/
bool
ZipFileEntry::Read(void* buf, Stream::Size numBytes) const
{
    n_assert(0 != this->zipFileHandle);
    n_assert(0 != buf);
    n_assert(0 != this->archiveCritSect);
    n_assert(numBytes < INT_MAX);
    // читать несжатые данные 
    int readResult = unzReadCurrentFile(this->zipFileHandle, buf, (uint32_t)numBytes);
    if (numBytes != readResult) return false;

    return true;
}

} // пространство имен
