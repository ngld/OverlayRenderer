#ifndef OVERLAY_CEF_RENDERER_PROCESS_HANDLER_H_
#define OVERLAY_CEF_RENDERER_PROCESS_HANDLER_H_

#include "include/cef_render_process_handler.h"

namespace OverlayRenderer {
namespace Cef {

class RenderProcessHandler : public CefRenderProcessHandler {
public:
    virtual void OnBrowserCreated(CefRefPtr<CefBrowser> browser,
                                CefRefPtr<CefDictionaryValue> extra_info) override;

    virtual void OnBrowserDestroyed(CefRefPtr<CefBrowser> browser) override;

private:
    IMPLEMENT_REFCOUNTING(RenderProcessHandler);
};

}
}

#endif
