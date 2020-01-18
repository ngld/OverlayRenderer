#ifndef OVERLAY_CEF_STATE_TRACKER_
#define OVERLAY_CEF_STATE_TRACKER_

#include <map>
#include <vector>
#include "include/cef_browser.h"
#include "../common/pipe_wrapper.h"

namespace OverlayRenderer {
namespace Cef {

typedef struct {
    int width, height;
    CefString url;
    std::vector<CefString> queued_js;
    CefRefPtr<CefBrowser> browser;
} OverlayState;

class StateTracker {
public:
    static void Init();
    static void DeInit();

    static OverlayState& GetState(int id);
    static void Poll();
    static void RunPollRegularly();
    static void SubmitFrame(int id, const void *buffer, int width, int height);
    static void RegisterBrowser(CefRefPtr<CefBrowser> browser, std::string name);
    static void ClearBrowser(int id);

    static void OpenNewOverlay(CefString name, OverlayState* state);

private:
    static Common::PipeWrapper* _out;
    static Common::PipeWrapper* _in;
    static std::unordered_map<int, OverlayState> states;
    static std::unordered_map<int, std::string> names;
    static std::unordered_map<std::string, int> rNames;
};

}
}

#endif
