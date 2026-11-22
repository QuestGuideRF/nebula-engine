//------------------------------------------------------------------------------
//  stringbuffer.cc
//  (C) 2009 г. Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "util/stringbuffer.h"

#include <string.h>

namespace Util
{

//------------------------------------------------------------------------------
/**
*/
StringBuffer::StringBuffer() :
    chunkSize(0), 
    curPointer(0)
{
    // empty
}    

//------------------------------------------------------------------------------
/**
*/
StringBuffer::~StringBuffer()
{
    if (this->IsValid())
    {
        this->Discard();
    }
}

//------------------------------------------------------------------------------
/**
    ПРИМЕЧАНИЕ. Этот метод необходимо вызывать до создания потоков.
*/
void
StringBuffer::Setup(SizeT size)
{
    n_assert(!this->IsValid());
    n_assert(size > 0);
    this->chunkSize = size;    
    this->AllocNewChunk();
}

//------------------------------------------------------------------------------
/**
*/
void
StringBuffer::Discard()
{
    n_assert(this->IsValid());
    IndexT i;
    for (i = 0; i < this->chunks.Size(); i++)
    {
        Memory::Free(Memory::StringDataHeap, this->chunks[i]);
        this->chunks[i] = 0;
    }
    this->chunks.Clear();
    this->chunkSize = 0;
    this->curPointer = 0;
}

//------------------------------------------------------------------------------
/**
*/
void
StringBuffer::AllocNewChunk()
{
    char* newChunk = (char*) Memory::Alloc(Memory::StringDataHeap, this->chunkSize);
    this->chunks.Append(newChunk);
    this->curPointer = newChunk;
}

//------------------------------------------------------------------------------
/**
    Копирует строку в конец строкового буфера, возвращает указатель на
    скопированная строка.
*/
const char*
StringBuffer::AddString(const char* str)
{
    n_assert(0 != str);
    n_assert(this->IsValid());

    // получить длину строки, она должна быть меньше размера фрагмента
    SizeT strLength = SizeT(strlen(str) + 1);
    n_assert(strLength < this->chunkSize);

    // проверьте, нужно ли выделять новый буфер
    if ((this->curPointer + strLength) >= (this->chunks.Back() + this->chunkSize))
    {
        #if NEBULA_ENABLE_GLOBAL_STRINGBUFFER_GROWTH
        this->AllocNewChunk();
        #else
        n_error("Буфер строк заполнен при добавлении строки '%s' (рост буфера строк отключен)!\n", str);
        #endif
    }

    // скопировать строку в строковый буфер
    char* dstPointer = this->curPointer;
    strcpy(dstPointer, str);
    this->curPointer += strLength;
    return dstPointer;
}

} // пространство имен Util
