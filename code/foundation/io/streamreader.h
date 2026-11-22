#pragma once
#ifndef IO_STREAMREADER_H
#define IO_STREAMREADER_H
#include "core/refcounted.h"
#include "core/ptr.h"
#include "io/stream.h"
namespace IO
{
class StreamReader : public Core::RefCounted
{
    __DeclareClass(StreamReader);
public:
    StreamReader();
    virtual ~StreamReader();
    void SetStream(const Ptr<Stream>& s);
    const Ptr<Stream>& GetStream() const;
    bool HasStream() const;
    bool Eof() const;
    virtual bool Open();
    virtual void Close();
    bool IsOpen() const;
protected:
    Ptr<Stream> stream;
    bool isOpen;
    bool streamWasOpen;
};
inline
bool
StreamReader::IsOpen() const
{
    return this->isOpen;
}
}
#endif