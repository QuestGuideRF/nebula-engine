#pragma once
#include "core/config.h"
#include "io/memorystream.h"
namespace Http
{
class HttpNzStream : public IO::MemoryStream
{
    __DeclareClass(HttpNzStream);
public:
    HttpNzStream();
    virtual ~HttpNzStream();
    virtual bool Open();
};
}