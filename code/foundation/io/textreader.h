#pragma once
#ifndef IO_TEXTREADER_H
#define IO_TEXTREADER_H
#include "io/streamreader.h"
namespace IO
{
class TextReader : public StreamReader
{
    __DeclareClass(TextReader);
public:
    TextReader();
    virtual ~TextReader();
    unsigned char ReadChar();
    Util::String ReadLine();
    Util::String ReadAll();
    Util::Array<Util::String> ReadAllLines();
};
};
#endif