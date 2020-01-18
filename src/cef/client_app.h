#ifndef OVERLAY_CEF_RENDERER_CLIENT_APP_H_
#define OVERLAY_CEF_RENDERER_CLIENT_APP_H_

#include "include/cef_app.h"
#include "include/cef_thread.h"
#include "cef/browser_handler.h"
#include "cef/render_process_handler.h"

namespace OverlayRenderer {
namespace Cef {

class ClientApp : public CefApp {
public:
    ClientApp();

    void StartStatePoller();

    virtual CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler() override {
        return _browserHandler;
    }
    virtual CefRefPtr<CefRenderProcessHandler> GetRenderProcessHandler() override {
        return _renderHandler;
    }

private:
    CefRefPtr<BrowserHandler> _browserHandler;
    CefRefPtr<RenderProcessHandler> _renderHandler;
    CefRefPtr<CefThread> _pollThread;

    IMPLEMENT_REFCOUNTING(ClientApp);
};

}
}

#endif