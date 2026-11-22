//------------------------------------------------------------------------------
//  crc.cc
//  (C) 2006 г. Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "util/crc.h"

namespace Util
{
unsigned int Crc::Table[NumByteValues] = { 0 };
bool Crc::TableInitialized = false;

template void Crc::Compute(unsigned char*, int32_t);
template void Crc::Compute(unsigned char*, int64_t);

//------------------------------------------------------------------------------
/**
*/
Crc::Crc() :
    inBegin(false),
    resultValid(false),
    checksum(0)
{
    // первая инициализация таблицы байтов
    if (!TableInitialized)
    {
        unsigned int i;
        for (i = 0; i < NumByteValues; ++i)
        {
            unsigned int reg = i << 24;
            int j;
            for (j = 0; j < 8; ++j)
            {
                bool topBit = (reg & 0x80000000) != 0;
                reg <<= 1;
                if (topBit)
                {
                    reg ^= Key;
                }
            }
            Table[i] = reg;
        }
        TableInitialized = true;
    }
}

//------------------------------------------------------------------------------
/**
    Начните вычисление контрольной суммы CRC для нескольких фрагментов данных. Это может быть
    выполняется за несколько прогонов, что является единственным полезным способом вычисления
    контрольная сумма для больших файлов.
*/
void
Crc::Begin()
{
    n_assert(!this->inBegin);    
    this->resultValid = false;
    this->inBegin = true;
    this->checksum = 0;
}

//------------------------------------------------------------------------------
/**
    Do one run of checksum computation for a chunk of data. Must be 
    executed inside Begin()/End().
*/
template <typename sizetype>
void
Crc::Compute(unsigned char* buf, sizetype numBytes)
{
    n_assert(this->inBegin);
    sizetype i;
    for (i = 0; i < numBytes; i++)
    {
        uint top = this->checksum >> 24;
        top ^= buf[i];
        this->checksum = (this->checksum << 8) ^ Table[top];
    }
}

//------------------------------------------------------------------------------
/**
    End checksum computation. This validates the result, so that
    it can be accessed with GetResult().
*/
void
Crc::End()
{
    n_assert(this->inBegin);
    this->inBegin = false;
    this->resultValid = true;
}

//------------------------------------------------------------------------------
/**
    Получите результат вычисления контрольной суммы. Должно быть выполнено
    после Конца().
*/
unsigned int
Crc::GetResult() const
{
    n_assert(!this->inBegin);
    n_assert(this->resultValid);
    return this->checksum;
}

} // пространство имен Util
