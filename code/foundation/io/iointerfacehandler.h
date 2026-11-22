#pragma once
#include "interface/interfacehandlerbase.h"
#include "io/console.h"
#include "io/ioserver.h"
#include "io/iointerfaceprotocol.h"
namespace IO
{
class IoInterfaceHandler : public Interface::InterfaceHandlerBase
{
    __DeclareClass(IoInterfaceHandler);
public:
    IoInterfaceHandler();
    virtual ~IoInterfaceHandler();
    virtual void Open();
    virtual void Close();
    virtual bool HandleMessage(const Ptr<Messaging::Message>& msg);
protected:
    void OnCreateDirectory(const Ptr<IO::CreateDirectory>& msg);
    void OnDeleteDirectory(const Ptr<IO::DeleteDirectory>& msg);
    void OnDeleteFile(const Ptr<IO::DeleteFile>& msg);
    void OnWriteStream(const Ptr<IO::WriteStream>& msg);
    void OnReadStream(const Ptr<IO::ReadStream>& msg);
    void OnCopyFile(const Ptr<IO::CopyFile>& msg);
    void OnMountArchive(const Ptr<IO::MountArchive>& msg);
    Ptr<IO::IoServer> ioServer;
};
}