#pragma once
#include "core/refcounted.h"
#include "io/stream.h"
namespace Net
{
class DebugPacket
{
public:
    enum Command
    {
        PC2WiiCmd,
        Wii2PCCmd,
        PC2WiiAck,
        Wii2PCAck,
    };
    static const SizeT PacketSize = 512;
    static const SizeT HeaderSize = 12;
    static const SizeT MaxPayloadSize = PacketSize - HeaderSize;
    DebugPacket();
    virtual ~DebugPacket();
    static Util::Array<DebugPacket> EncodeStream(ushort portNum, uint firstPacketCounter, const Ptr<IO::Stream>& stream);
    static void DecodePackets(const Util::Array<DebugPacket>& packets, const Ptr<IO::Stream>& stream, ushort& outPortNum);
    SizeT Write(ushort portNum, uint packetCounter, const ubyte* buf, SizeT numBytes);
    void WriteRaw(const void* buf, SizeT bufSize);
    void SetDataValid(bool b);
    bool HasData() const;
    Util::FourCC GetMagic() const;
    uint GetCount() const;
    ushort GetPort() const;
    bool IsFinalPacket() const;
    SizeT GetPayloadSize() const;
    const ubyte* GetPayload() const;
    const ubyte* GetRawBuffer() const;
private:
    ubyte buffer[PacketSize];
    bool hasData;
};
inline void
DebugPacket::SetDataValid(bool b)
{
    this->hasData = b;
}
inline bool
DebugPacket::HasData() const
{
    return this->hasData;
}
}