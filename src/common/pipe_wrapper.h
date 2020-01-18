#ifndef OVERLAYRENDERER_COMMON_PIPE_WRAPPER_H_
#define OVERLAYRENDERER_COMMON_PIPE_WRAPPER_H_

#include <packets.pb.h>

namespace OverlayRenderer {
namespace Common {

class PipeWrapper {
public:
    PipeWrapper() {};
    virtual ~PipeWrapper() {};

    virtual bool Listen() =0;
    virtual bool Connect() =0;

    virtual bool Send(const Packet& pkt) =0;
    virtual bool Receive(Packet& pkt) =0;
};

}
}

#endif
