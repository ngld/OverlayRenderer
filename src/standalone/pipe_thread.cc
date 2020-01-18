#include <thread>
#include "../common/logging.h"
#include "../common/win_pipe_wrapper.h"
#include "packets.pb.h"
#include "./pipe_thread.h"
#include "./host.h"

using namespace OverlayRenderer;

static std::thread poll_thread;

void FrameReceiver() {
    loguru::set_thread_name("frame receiver");
    Common::WinPipeWrapper pipe("OverlayRenderer-out");
    Packet pkt;

    while (true) {
        pipe.MaybeConnect();
        if (!pipe.Receive(pkt))
            continue;

        try {
            std::string name = pkt.name();
            auto& ov = overlays[name];
            
            switch (pkt.type()) {
                case Packet_Type_FRAME:
                {
                    std::lock_guard<std::mutex> lock(overlays[name].tex_mutex);

                    ov.tex_buffer_width  = pkt.size().width();
                    ov.tex_buffer_height = pkt.size().height();
                    ov.tex_buffer_used   = pkt.frame().length();

                    if (ov.tex_buffer_used > ov.tex_buffer_size) {
                        if (ov.tex_buffer != nullptr)
                            free(ov.tex_buffer);

                        ov.tex_buffer_size = ov.tex_buffer_used;
                        ov.tex_buffer = malloc(ov.tex_buffer_size);
                    }

                    std::memcpy(ov.tex_buffer, pkt.frame().c_str(), ov.tex_buffer_used);
                    ov.needs_update = true;
                }
                break;

                default:
                    LOG_F(ERROR, "Encountered unkown packet type %d!", pkt.type());
            }
        } catch(std::exception ex) {
            LOG_F(ERROR, "Failed to receive: %s", ex.what());
        }
    }
}

void StartFrameReceiver() {
    poll_thread = std::thread(FrameReceiver);
}
