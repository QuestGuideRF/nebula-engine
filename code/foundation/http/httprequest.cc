//------------------------------------------------------------------------------
//  httprequest.cc
//  (C) 2007 Радон Лабс ГмбХ
//  (C) 2013-2020 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------

#include "http/httprequest.h"

namespace Http
{
__ImplementClass(Http::HttpRequest, 'HTRQ', Messaging::Message);
__ImplementMsgId(HttpRequest);

//------------------------------------------------------------------------------
/**
*/
HttpRequest::HttpRequest() :
    method(HttpMethod::InvalidHttpMethod),
    status(HttpStatus::InvalidHttpStatus)
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
HttpRequest::~HttpRequest()
{
    // empty
}

} // пространство имен HTTP
