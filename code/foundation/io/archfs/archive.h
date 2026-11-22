#pragma once
#if __WIN32__ || __linux__
#include "io/zipfs/ziparchive.h"
namespace IO
{
class Archive : public ZipArchive
{
    __DeclareClass(Archive);
};
}
#else
#error "IO::Архив не реализован на этой платформе!"
#endif