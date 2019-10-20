#ifndef OVERLAY_CEF_RENDERER_APP_H_
#define OVERLAY_CEF_RENDERER_APP_H_

#include "include/cef_app.h"
#include "include/cef_browser_process_handler.h"

namespace OverlayRenderer {
namespace Cef {

class RendererApp : public CefApp, public CefBrowserProcessHandler {
public:
    RendererApp();

    // CefApp
    virtual CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler() OVERRIDE {
        return this;
    }

    // CefBrowserPorcessHandler
    virtual void OnContextInitialized() OVERRIDE;

private:
    IMPLEMENT_REFCOUNTING(RendererApp);
};

}
}

#endif