#include "render_handler.h"
#include "browser_client.h"

using namespace OverlayRenderer::Cef;

CefRefPtr<CefRenderHandler> BrowserClient::GetRenderHandler() {
    if (!_renderHandler.get()) {
        _renderHandler = new RenderHandler();
    }

    return _renderHandler;
}
