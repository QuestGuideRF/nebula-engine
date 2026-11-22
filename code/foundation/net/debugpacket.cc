//------------------------------------------------------------------------------
//  debugpacket.cc
//  (C) 2009 г. Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "net/debugpacket.h"
#include "system/byteorder.h"

namespace Net
{

using namespace Util;
using namespace IO;
using namespace System;

//------------------------------------------------------------------------------
/**
*/
DebugPacket::DebugPacket() :
    hasData(false)
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
DebugPacket::~DebugPacket()
{
    // empty
}

//------------------------------------------------------------------------------
/**
    Write data to the packet. Returns the number of data written. If all
    data could be written, the returned number is identical with the
    requested size, and the packet's payload size will be set to that size.
    This means the packet completes a message.
    If not all data could be written, the actually written size (MaxPayloadSize)
    will be returned, and the internal payloadSize will be set to 0xFFFF, meaning
    that the packet contains incomplete data which will be continued with the
    next packet.
*/
SizeT
DebugPacket::Write(ushort portNum, uint packetCount, const ubyte* buf, SizeT numBytes)
{
    n_assert(0 != buf);
    n_assert(numBytes > 0);
    n_assert(!this->hasData);    
    this->hasData = true;

    ByteOrder byteOrder(ByteOrder::Host, ByteOrder::Network);
    union { ubyte *b; uint *u; ushort *us; } pun;
    pun.b = this->buffer;
    uint* uintHeader = pun.u;
    ushort* ushortHeader = pun.us;
    ubyte* payloadPtr = this->buffer + HeaderSize;
    
    SizeT numActualBytes = numBytes < MaxPayloadSize ? numBytes : MaxPayloadSize;
    if (numActualBytes < numBytes)
    {    
        // настроить заголовок для неполного пакета
        uintHeader[0]   = byteOrder.Convert<uint>(FourCC('DPKT').AsUInt());
        uintHeader[1]   = byteOrder.Convert<uint>(packetCount);
        ushortHeader[4] = byteOrder.Convert<ushort>(portNum);
        ushortHeader[5] = byteOrder.Convert<ushort>(0xFFFF);
    }
    else
    {
        // настроить заголовок для полного пакета
        uintHeader[0]   = byteOrder.Convert<uint>(FourCC('DPKT').AsUInt());
        uintHeader[1]   = byteOrder.Convert<uint>(packetCount);
        ushortHeader[4] = byteOrder.Convert<ushort>(portNum);
        ushortHeader[5] = byteOrder.Convert<ushort>((ushort)numActualBytes);
    }

    // скопировать полезную нагрузку
    Memory::Copy(buf, payloadPtr, numActualBytes);
    
    // return number of bytes written
    return numActualBytes;
}

//------------------------------------------------------------------------------
/**
    Запишите необработанные данные в пакет. Исходный буфер должен содержать допустимый
    заголовок пакета и размер исходных данных должны быть равны 
    ДебагПакет::ПакетСизе!
*/
void
DebugPacket::WriteRaw(const void* buf, SizeT bufSize)
{
    n_assert(PacketSize == bufSize);
    Memory::Copy(buf, this->buffer, PacketSize);
    this->hasData = true;
}

//------------------------------------------------------------------------------
/**
    Получите магический код в начале сообщения. Должно быть
    FourCC('DPKT').
*/
FourCC
DebugPacket::GetMagic() const
{
    n_assert(this->hasData);
    ByteOrder byteOrder(ByteOrder::Network, ByteOrder::Host);
    union pun { const uint *u; const ubyte *b; } pun;
    pun.b = this->buffer;
    FourCC magic = byteOrder.Convert<uint>(pun.u[0]);
    return magic;
}

//------------------------------------------------------------------------------
/**
    Получите счетчик пакетов этого пакета.
*/
uint
DebugPacket::GetCount() const
{
    n_assert(this->hasData);
    ByteOrder byteOrder(ByteOrder::Network, ByteOrder::Host);
    union pun { const uint *u; const ubyte *b; } pun;
    pun.b = this->buffer;
    uint count = byteOrder.Convert<uint>(pun.u[1]);
    return count;
}

//------------------------------------------------------------------------------
/**
    Получите номер порта из пакета.
*/
ushort
DebugPacket::GetPort() const
{
    n_assert(this->hasData);
    ByteOrder byteOrder(ByteOrder::Network, ByteOrder::Host);
    union pun { const ushort *us; const ubyte *b; } pun;
    pun.b = this->buffer;
    ushort portNum = byteOrder.Convert<ushort>(pun.us[4]);
    return portNum;
}

//------------------------------------------------------------------------------
/**
    Возвращайте true, если это последний пакет в многопакетном сообщении.
*/
bool
DebugPacket::IsFinalPacket() const
{
    n_assert(this->hasData);
    ByteOrder byteOrder(ByteOrder::Network, ByteOrder::Host);
    union pun { const ushort *us; const ubyte *b; } pun;
    pun.b = this->buffer; 
    ushort payloadSize = byteOrder.Convert<ushort>(pun.us[5]);
    return (0xFFFF != payloadSize);
}

//------------------------------------------------------------------------------
/**
    Возвращает количество байтов полезных данных в пакете. Это будет
    всегда возвращайте допустимое число, даже если элемент заголовка payloadSize
    установлен специальный код 0xFFFF.
*/
SizeT
DebugPacket::GetPayloadSize() const
{
    n_assert(this->hasData);
    ByteOrder byteOrder(ByteOrder::Network, ByteOrder::Host);
    union pun { const ushort *us; const ubyte *b; } pun;
    pun.b = this->buffer; 
    ushort payloadSize = byteOrder.Convert<ushort>(pun.us[5]);
    if (0xFFFF == payloadSize)
    {
        payloadSize = MaxPayloadSize;
    }
    return payloadSize;
}

//------------------------------------------------------------------------------
/**
    Получите указатель на фактические данные полезной нагрузки.
*/
const ubyte*
DebugPacket::GetPayload() const
{
    n_assert(this->hasData);
    return this->buffer + HeaderSize;
}

//------------------------------------------------------------------------------
/**
    Получите указатель на необработанные данные пакета.
*/
const ubyte*
DebugPacket::GetRawBuffer() const
{
    return this->buffer;
}

//------------------------------------------------------------------------------
/**
*/
Array<DebugPacket>
DebugPacket::EncodeStream(ushort portNum, uint packetCounter, const Ptr<Stream>& stream)
{
    n_assert(stream->CanBeMapped());
    n_assert(stream->GetSize() < INT_MAX);

    Array<DebugPacket> result;
    SizeT bytesRemaining = (SizeT)stream->GetSize();
    stream->SetAccessMode(Stream::ReadAccess);
    bool streamOpenResult = stream->Open();
    n_assert(streamOpenResult);
    uchar* srcPtr = (uchar*) stream->Map();
    do
    {
        DebugPacket packet;
    
        // записать следующий фрагмент данных 
        SizeT bytesWritten = packet.Write(portNum, packetCounter++, srcPtr, bytesRemaining);
        srcPtr += bytesWritten;
        bytesRemaining -= bytesWritten;

        // добавить пакет в результат
        result.Append(packet);
    }
    while (bytesRemaining > 0);
    stream->Unmap();
    stream->Close();
    return result;
}

//------------------------------------------------------------------------------
/**
*/
void
DebugPacket::DecodePackets(const Array<DebugPacket>& packets,
                           const Ptr<Stream>& stream,
                           ushort& outPortNum)
{
    n_assert(packets.Back().IsFinalPacket());
    n_assert(packets.Size() > 0);

    // прочитать номер порта из первого пакета
    outPortNum = packets[0].GetPort();

    // открытый поток для записи и перебора пакетов
    stream->SetAccessMode(Stream::WriteAccess);
    bool streamOpenResult = stream->Open();
    n_assert(streamOpenResult);
    IndexT i;
    for (i = 0; i < packets.Size(); i++)
    {
        const DebugPacket& curPacket = packets[i];
        n_assert(curPacket.HasData());
        if (i < (packets.Size() - 1))
        {
            // убедитесь, что между ними нет финальных пакетов
            n_assert(!curPacket.IsFinalPacket());
        }

        // убедитесь, что номер порта одинаков для всех пакетов
        n_assert(curPacket.GetPort() == outPortNum);

        // скопировать полезную нагрузку пакетов в поток
        stream->Write(curPacket.GetPayload(), curPacket.GetPayloadSize());
    }
    stream->Close();
}

} // Сеть пространства имен