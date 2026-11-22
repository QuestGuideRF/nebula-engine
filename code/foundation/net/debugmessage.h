#pragma once
#include "core/types.h"
#include "io/stream.h"
namespace Net
{
class DebugMessage
{
public:
    DebugMessage();
    DebugMessage(ushort port, const Ptr<IO::Stream>& data);
    ushort GetPort() const;
    const Ptr<IO::Stream>& GetStream() const;
    bool IsValid() const;
private:
    ushort portNum;
    Ptr<IO::Stream> dataStream;
};
inline
DebugMessage::DebugMessage() :
    portNum(0)
{
}
inline
DebugMessage::DebugMessage(ushort p, const Ptr<IO::Stream>& s) :
    portNum(p),
    dataStream(s)
{
}
inline ushort
DebugMessage::GetPort() const
{
    return this->portNum;
}
inline const Ptr<IO::Stream>&
DebugMessage::GetStream() const
{
    return this->dataStream;
}
inline bool
DebugMessage::IsValid() const
{
    return this->dataStream.isvalid();
}
}