#pragma once
#include "core/config.h"
#if (__WIN32__ || __OSX__ || __linux__)
#include "io/base/gamecontentserverbase.h"
namespace IO
{
class GameContentServer : public Base::GameContentServerBase
{
    __DeclareClass(GameContentServer);
    __DeclareInterfaceSingleton(GameContentServer);
public:
    GameContentServer();
    virtual ~GameContentServer();
};
}
#else
#error "IO::GameContentServer не реализован на этой платформе!"
#endif