#pragma once
#include "core/refcounted.h"
#include "core/ptr.h"
#include "io/stream.h"
namespace IO
{
class StreamWriter : public Core::RefCounted
{
    __DeclareClass(StreamWriter);
public:
    StreamWriter();
    virtual ~StreamWriter();
    void SetStream(const Ptr<Stream>& s);
    const Ptr<Stream>& GetStream() const;
    bool HasStream() const;
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
StreamWriter::IsOpen() const
{
    return this->isOpen;
}
}