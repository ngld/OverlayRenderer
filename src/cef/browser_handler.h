#ifndef OVERLAY_CEF_RENDERER_BROWSER_HANDLER_H_
#define OVERLAY_CEF_RENDERER_BROWSER_HANDLER_H_

#include "include/cef_browser_process_handler.h"

namespace OverlayRenderer {
namespace Cef {

class BrowserHandler :public CefBrowserProcessHandler {
public:
    BrowserHandler();

    // CefBrowserProcessHandler
    virtual void OnContextInitialized() OVERRIDE;

private:
    IMPLEMENT_REFCOUNTING(BrowserHandler);
};

}
}

#endif