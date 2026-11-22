#pragma once
#include "io/filestream.h"
#include "util/string.h"
#include "io/filetime.h"
#include "io/fswrapper.h"
namespace IO
{
class SafeFileStream : public FileStream
{
    __DeclareClass(SafeFileStream);
public:
    SafeFileStream();
    virtual ~SafeFileStream();
    virtual bool Open();
    virtual void Close();
protected:
    IO::URI tmpUri;
};
}