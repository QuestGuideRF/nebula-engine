#pragma once
#include "core/refcounted.h"
#include "core/singleton.h"
namespace Audio
{
class AudioDevice;
class AudioServer : public Core::RefCounted
{
    __DeclareClass(AudioServer);
    __DeclareSingleton(AudioServer);
public:
    AudioServer();
    ~AudioServer();
    bool Open();
    bool Close();
    bool IsOpen() const;
    void OnFrame();
private:
    bool isOpen;
    Ptr<AudioDevice> device;
};
inline bool
AudioServer::IsOpen() const
{
    return this->isOpen;
}
}