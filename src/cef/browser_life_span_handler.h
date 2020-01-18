#ifndef OVERLAY_CEF_BROWSER_LIFE_SPAN_HANDLER_H_
#define OVERLAY_CEF_BROWSER_LIFE_SPAN_HANDLER_H_

#include "include/cef_life_span_handler.h"

namespace OverlayRenderer {
namespace Cef {

class BrowserLifeSpanHandler :public CefLifeSpanHandler {
public:
    virtual void OnBeforeClose(CefRefPtr<CefBrowser> browser) override;

private:
    IMPLEMENT_REFCOUNTING(BrowserLifeSpanHandler);
};

}
}

#endif