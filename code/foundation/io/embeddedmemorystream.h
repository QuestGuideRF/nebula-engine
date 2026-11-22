#pragma once
#include "io/memorystream.h"
namespace IO
{
class EmbeddedMemoryStream : public MemoryStream
{
    __DeclareClass(EmbeddedMemoryStream);
public:
    EmbeddedMemoryStream();
    virtual ~EmbeddedMemoryStream();
    virtual bool CanWrite() const;
    virtual bool Open();
};
}