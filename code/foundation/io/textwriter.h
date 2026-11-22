#pragma once
#ifndef IO_TEXTWRITER_H
#define IO_TEXTWRITER_H
#include "io/streamwriter.h"
namespace IO
{
class TextWriter : public StreamWriter
{
    __DeclareClass(TextWriter);
public:
    TextWriter();
    virtual ~TextWriter();
    void WriteChar(unsigned char c);
    void WriteString(const Util::String& str);
    void WriteFormatted(const char* fmtString, ...);
    void WriteLine(const Util::String& line);
    void WriteLines(const Util::Array<Util::String>& lines);
    template<typename T> void Write(const T& t);
};
}
#endif