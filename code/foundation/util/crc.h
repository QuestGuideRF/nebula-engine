#pragma once
#include "core/types.h"
namespace Util
{
class Crc
{
public:
    Crc();
    void Begin();
    template<typename sizetype> void Compute(unsigned char* buf, sizetype numBytes);
    void End();
    unsigned int GetResult() const;
private:
    static const unsigned int Key = 0x04c11db7;
    static const unsigned int NumByteValues = 256;
    static unsigned int Table[NumByteValues];
    static bool TableInitialized;
    bool inBegin;
    bool resultValid;
    unsigned int checksum;
};
}