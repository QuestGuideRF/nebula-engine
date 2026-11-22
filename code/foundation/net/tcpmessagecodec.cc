//------------------------------------------------------------------------------
//  tcpmessagecodec.cc
//  (C) 2009 г. Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "net/tcpmessagecodec.h"
#include "io/binarywriter.h"
#include "io/binaryreader.h"
#include "io/memorystream.h"

//------------------------------------------------------------------------------
using namespace Util;
using namespace IO;

namespace Net
{

//------------------------------------------------------------------------------
/**
*/
TcpMessageCodec::TcpMessageCodec():
    receiveState(HeaderData),
    messageSize(0),
    headerPosition(0),
    messagePostition(0)
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
TcpMessageCodec::~TcpMessageCodec()
{
    // закрывает и отбрасывает потоки заголовков и сообщений, если они существуют...
    if (this->headerStream.isvalid())
    {
        if (this->headerStream->IsOpen())
        {
            this->headerStream->Close();
        }
        this->headerStream = nullptr;
    }
    if (this->messageStream.isvalid())
    {
        if (this->messageStream->IsOpen())
        {
            this->messageStream->Close();
        }
        this->messageStream = nullptr;
    }
}

//------------------------------------------------------------------------------
/**
    Записывает копию данного потока в данный выходной поток с заголовком.
    информация в начале.
*/
void
TcpMessageCodec::EncodeToMessage(const Ptr<IO::Stream> &stream, const Ptr<IO::Stream> &output)
{
    n_assert(stream->GetSize() < INT_MAX);
    Ptr<BinaryWriter> writer = BinaryWriter::Create();
    writer->SetStreamByteOrder(System::ByteOrder::Network);
    writer->SetStream(output);
    SizeT streamSize = (SizeT)stream->GetSize();
    stream->SetAccessMode(Stream::ReadAccess);
    output->SetAccessMode(Stream::WriteAccess);
    uchar* ptr = 0;
    if (writer->Open() && stream->Open())
    {
        writer->WriteUInt('TCPM');
        writer->WriteInt(streamSize);
        ptr = (uchar*) stream->Map();
        writer->WriteRawData(ptr,streamSize);
    }
    if (writer->IsOpen())
    {
        writer->Close();
    }
    if (stream->IsOpen())
    {
        stream->Close();
    }
}

//------------------------------------------------------------------------------
/**
    Помещает заданный поток во внутренний буфер. Это может привести к появлению новых
    сообщения в очереди.
*/
void
TcpMessageCodec::DecodeStream(const Ptr<IO::Stream> &stream)
{
    stream->SetAccessMode(Stream::ReadAccess);
    Ptr<BinaryReader> reader = BinaryReader::Create();
    reader->SetStream(stream);
    if(reader->Open())
    {
        uchar byte;
        while(!reader->Eof())
        {
            // прочитать байт
            byte = reader->ReadUChar();
            
            if(HeaderData == this->receiveState)
            {
                // читать заголовок, пока не будет получено достаточно байтов
                if(this->headerPosition == 0)
                {
                    this->headerStream = MemoryStream::Create();
                    this->headerStream->SetAccessMode(Stream::WriteAccess);
                    this->headerStream->Open();
                }
                
                if (headerStream->IsOpen())
                {
                    // записать байт в поток заголовков
                    this->headerStream->Write(&byte,sizeof(byte));
                    headerPosition++;
                    
                    if (this->headerPosition == 8)
                    {
                        this->headerStream->Close();
                        n_assert(this->headerStream->GetSize() == 8);
                        // обрабатывать полный полученный заголовок
                        this->headerStream->SetAccessMode(Stream::ReadAccess);
                        Ptr<BinaryReader> reader = BinaryReader::Create();
                        reader->SetStream(this->headerStream);
                        reader->SetStreamByteOrder(System::ByteOrder::Network);
                        n_assert(reader->Open());
                        // прочитать имя заголовка «TCPM»
                        uint tcpm = reader->ReadUInt();
                        if(tcpm == 'TCPM')
                        {
                            this->messageSize = reader->ReadInt();
                            this->messagePostition = 0;
                            this->receiveState = MessageData;
                        }
                        reader->Close();
                        this->headerPosition = 0;
                    }
                }
            }
            else if(MessageData == this->receiveState)
            {   
                // читать сообщение, пока не будут доставлены все байты
                if (this->messagePostition == 0)
                {
                    this->messageStream = MemoryStream::Create();
                    this->messageStream->SetAccessMode(Stream::WriteAccess);
                    this->messageStream->Open();
                }
                
                if (this->messageStream->IsOpen())
                {
                    // записать байт в поток сообщений
                    this->messageStream->Write(&byte,sizeof(byte));
                    messagePostition++;
                    
                    if(messagePostition == messageSize)
                    {
                        this->messageStream->Close();
                        n_assert(this->messageStream->GetSize() == messageSize);
                        // прикрепить полностью полученное сообщение
                        this->messageStream->SetAccessMode(Stream::ReadAccess);
                        this->completedMessages.Append(this->messageStream);
                        this->messageStream = nullptr;
                        this->messagePostition = 0;
                        this->receiveState = HeaderData;
                    }
                }
            }
        }
    }
}

//------------------------------------------------------------------------------
/**
    Возвращает true, если сообщения доступны
*/
bool
TcpMessageCodec::HasMessages()
{
    return this->completedMessages.Size() > 0;
}

//------------------------------------------------------------------------------
/**
    Возвращает все созданные сообщения с момента последнего звонка и очищает сообщение.
    очередь.
*/
Util::Array<Ptr<IO::Stream> >
TcpMessageCodec::DequeueMessages()
{
    Array<Ptr<Stream> > output;
    output.AppendArray(this->completedMessages);
    this->completedMessages.Clear();
    return output;
}
} // Сеть пространства имен
