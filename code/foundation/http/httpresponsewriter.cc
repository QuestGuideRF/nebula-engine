//------------------------------------------------------------------------------
//  httpresponsewriter.cc
//  (C) 2007 Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "http/httpresponsewriter.h"
#include "io/textwriter.h"

namespace Http
{
__ImplementClass(Http::HttpResponseWriter, 'HRSW', IO::StreamWriter);

using namespace IO;

//------------------------------------------------------------------------------
/**
*/
void
HttpResponseWriter::WriteResponse()
{
    Ptr<TextWriter> textWriter = TextWriter::Create();
    textWriter->SetStream(this->stream);
    if (textWriter->Open())
    {
        textWriter->WriteFormatted("HTTP/1.1 %s %s\r\n", 
            HttpStatus::ToString(this->statusCode).AsCharPtr(),
            HttpStatus::ToHumanReadableString(this->statusCode).AsCharPtr());
        if (this->contentStream.isvalid())
        {
            textWriter->WriteFormatted("Длина контента: %d\r\n", 
                this->contentStream->GetSize());
            if (this->contentStream->GetMediaType().AsString().IsValid())
            {
                textWriter->WriteFormatted("Тип контента: %s\r\n", 
                    this->contentStream->GetMediaType().AsString().AsCharPtr());
            }
        }
        else
        {
            textWriter->WriteString("Длина контента: 0\r\n");
        }
        textWriter->WriteString("\r\n");
        textWriter->Close();
    }

    // добавить строку содержимого
    if (this->contentStream.isvalid())
    {
        n_assert(this->contentStream->CanBeMapped());
        this->contentStream->SetAccessMode(IO::Stream::ReadAccess);
        if (this->contentStream->Open())
        {                 
            void* ptr = this->contentStream->Map();
            this->stream->Write(ptr, this->contentStream->GetSize());
            this->contentStream->Unmap();                                                          
            this->contentStream->Close();
        }             
    }
}

} // пространство имен HTTP