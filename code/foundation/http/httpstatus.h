#pragma once
#ifndef HTTP_HTTPSTATUS_H
#define HTTP_HTTPSTATUS_H
#include "util/string.h"
namespace Http
{
class HttpStatus
{
public:
    enum Code
    {
        Continue                        = 100,
        OK                              = 200,
        BadRequest                      = 400,
        Forbidden                       = 403,
        NotFound                        = 404,
        MethodNotAllowed                = 405,
        NotAcceptable                   = 406,
        InternalServerError             = 500,
        NotImplemented                  = 501,
        ServiceUnavailable              = 503,
        InvalidHttpStatus = 0xffff
    };
    static Code FromString(const Util::String& str);
    static Util::String ToString(Code c);
    static Code FromLong(long l);
    static Util::String ToHumanReadableString(Code c);
};
}
#endif