#include "include/wrapper/cef_closure_task.h"
#include "include/wrapper/cef_helpers.h"
#include "include/base/cef_bind.h"
#include "include/cef_browser.h"
#include "../common/packets.h"
#include "../common/win_pipe_wrapper.h"
#include "browser_client.h"
#include "packets.pb.h"
#include "state_tracker.h"

namespace OverlayRenderer {
namespace Cef {

#define IN_SIZE 300
#define OUT_SIZE 0x500000

Common::PipeWrapper* StateTracker::_out;
Common::PipeWrapper* StateTracker::_in;

std::unordered_map<int, OverlayState> StateTracker::states;
std::unordered_map<int, std::string> StateTracker::names;
std::unordered_map<std::string, int> StateTracker::rNames;

void StateTracker::Init() {
    // CEF_REQUIRE_UI_THREAD();
    
    _out = new Common::WinPipeWrapper("OverlayRenderer-out");
    _out->Listen();

    _in = new Common::WinPipeWrapper("OverlayRenderer-in");
    _in->Listen();
}

void StateTracker::DeInit() {
    delete _out;
}

OverlayState& StateTracker::GetState(int id) {
    return states[id];
}

void StateTracker::OpenNewOverlay(CefString name, OverlayState* state) {
    CefWindowInfo info;
    info.SetAsWindowless(NULL);

    CefBrowserSettings settings;
    settings.windowless_frame_rate = 30;

    CefRefPtr<CefClient> client(new BrowserClient());
    CefRefPtr<CefBrowser> browser = CefBrowserHost::CreateBrowserSync(info, client, "about:blank", settings, NULL, NULL);

    StateTracker::RegisterBrowser(browser, name);
    OverlayState& ns = StateTracker::states[browser->GetIdentifier()];

    ns.width = state->width;
    ns.height = state->height;
    ns.url = state->url;

    free(state);

    if (!ns.url.empty())
        browser->GetMainFrame()->LoadURL(ns.url);

    browser->GetHost()->WasResized();
}

void StateTracker::Poll() {
    Packet pkt;
    size_t received;
    unsigned int priority;

    Init();

    LOG(INFO) << "Polling for messages...";

    while (true) {
        bool success = false;

        try {
            success = _in->Receive(pkt);
        } catch(std::exception ex) {
            LOG(ERROR) << "Failed to receive: " << ex.what();
            continue;
        }

        if (success) {
            std::string name = pkt.name();
            OverlayState* tmp_state;

            switch (pkt.type()) {
                case Packet_Type_INIT:
                    LOG(INFO) << "Got " << pkt.DebugString();
                    tmp_state = new OverlayState;

                    tmp_state->width = pkt.size().width();
                    tmp_state->height = pkt.size().height();
                    tmp_state->url = pkt.url();

                    CefPostTask(TID_UI, base::Bind(&StateTracker::OpenNewOverlay, name, tmp_state));
                    continue;

                case Packet_Type_CLEAR:
                    for (auto& pair : states) {
                        if (pair.second.browser.get()) {
                            pair.second.browser->GetHost()->CloseBrowser(true);
                        }
                    }
                    continue;

                default:
                    // Do nothing
                    break;
            }

            if (rNames.find(name) == rNames.end()) {
                LOG(ERROR) << "Got message for missing overlay " << name << "!";
                continue;
            } else {
                LOG(INFO) << "Got " << pkt.DebugString();
            }

            OverlayState& state = states[rNames[name]];
            CefMouseEvent mouseEvent;
            CefBrowserHost::MouseButtonType mbt;

            switch (pkt.type()) {
                case Packet_Type_MOUSEMOVE:
                    
                    mouseEvent.x = pkt.size().width();
                    mouseEvent.y = pkt.size().height();

                    CefPostTask(TID_UI, base::Bind(&CefBrowserHost::SendMouseMoveEvent,
                        state.browser->GetHost(), mouseEvent, false));
                    break;

                case Packet_Type_MOUSECLICK:
                    mouseEvent.x = pkt.size().width();
                    mouseEvent.y = pkt.size().height();

                    switch (pkt.button()) {
                        case PKT_MB_LEFT:
                            mbt = MBT_LEFT;
                            break;
                        case PKT_MB_MIDDLE:
                            mbt = MBT_MIDDLE;
                            break;
                        case PKT_MB_RIGHT:
                            mbt = MBT_RIGHT;
                            break;

                        default:
                            NOTREACHED();
                    }

                    CefPostTask(TID_UI, base::Bind(&CefBrowserHost::SendMouseClickEvent,
                        state.browser->GetHost(), mouseEvent, mbt, !pkt.button_down(), 1));
                    break;

                case Packet_Type_MOUSESCROLL:
                    mouseEvent.x = pkt.size().width();
                    mouseEvent.y = pkt.size().height();

                    CefPostTask(TID_UI, base::Bind(&CefBrowserHost::SendMouseWheelEvent,
                        state.browser->GetHost(), mouseEvent, pkt.delta().width(), pkt.delta().height()));
                    break;

                case Packet_Type_CAPTURE_LOST:
                    mouseEvent.x = pkt.size().width();
                    mouseEvent.y = pkt.size().height();

                    CefPostTask(TID_UI, base::Bind(&CefBrowserHost::SendMouseMoveEvent,
                        state.browser->GetHost(), mouseEvent, true));
                    break;

                case Packet_Type_SIZE:
                    state.width = pkt.size().width();
                    state.height = pkt.size().height();

                    CefPostTask(TID_UI, base::Bind(&CefBrowserHost::WasResized,
                        state.browser->GetHost()));
                    
                    if (pkt.type() != Packet_Type_INIT) break;

                case Packet_Type_URL:
                    if (!pkt.url().empty())
                        CefPostTask(TID_UI, base::Bind(&CefFrame::LoadURL,
                            state.browser->GetMainFrame(), pkt.url()));
                    break;

                case Packet_Type_JAVASCRIPT:
                    CefPostTask(TID_UI, base::Bind(&CefFrame::ExecuteJavaScript,
                        state.browser->GetMainFrame(), pkt.javascript(), "chrome://injected", 1));
                    break;

                default:
                    LOG(ERROR) << "Received unknown message type " << pkt.type() << "! " << pkt.DebugString();
            }
        } else {
            LOG(ERROR) << "Got invalid packet! " << pkt.DebugString();
        }
    }
}

void StateTracker::SubmitFrame(int id, const void* buffer, int width, int height) {
    Packet pkt;
    pkt.set_name(names[id]);
    pkt.set_frame(buffer, width * height * 4);
    
    Size* frame_size = pkt.mutable_size();
    frame_size->set_width(width);
    frame_size->set_height(height);

    try {
        // LOG(INFO) << names[id] << ": Frame!";
        _out->Send(pkt);
    } catch(std::exception ex) {
        LOG(ERROR) << "Failed to send frame: " << ex.what();
    }
}

void StateTracker::RegisterBrowser(CefRefPtr<CefBrowser> browser, std::string name) {
    int id = browser->GetIdentifier();
    rNames[name] = id;
    names[id] = name;
    states[id].browser = browser;
}

void StateTracker::ClearBrowser(int id) {
    std::string name = names[id];

    names.erase(id);
    states.erase(id);
    rNames.erase(name);
}

}
}
