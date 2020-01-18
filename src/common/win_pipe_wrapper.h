#ifndef OVERLAYRENDERER_COMMON_WIN_PIPE_WRAPPER_H_
#define OVERLAYRENDERER_COMMON_WIN_PIPE_WRAPPER_H_

#include <string>
#include <thread>
#include <Windows.h>
#include "pipe_wrapper.h"

namespace OverlayRenderer {
namespace Common {

class WinPipeWrapper : public PipeWrapper {
public:
    WinPipeWrapper(std::string name);
    ~WinPipeWrapper();

    virtual bool Listen() override;
    virtual bool Connect() override;
    virtual bool MaybeConnect();

    virtual bool Send(const Packet& pkt) override;
    virtual bool SendRaw(void* buffer, int length, unsigned long* bytesWritten);
    virtual bool Receive(Packet& pkt) override;

private:
    void EncodePacket(void** buffer, int& length, const Packet& pkt);
    void _Listen();

    std::thread _listen_thread;
    std::string _name;
    bool _server;
    HANDLE _pipe;

    std::string _error;
};

}
}

#endif
