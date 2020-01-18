#ifndef OVERLAY_CEF_RENDERER_BROWSER_CLIENT_H_
#define OVERLAY_CEF_RENDERER_BROWSER_CLIENT_H_

#include "include/cef_client.h"

namespace OverlayRenderer {
namespace Cef {

class BrowserClient : public CefClient {
public:
    BrowserClient() { };

    virtual CefRefPtr<CefRenderHandler> GetRenderHandler() OVERRIDE;

private:
    CefRefPtr<CefRenderHandler> _renderHandler;

    IMPLEMENT_REFCOUNTING(BrowserClient);
};

}
}

#endif