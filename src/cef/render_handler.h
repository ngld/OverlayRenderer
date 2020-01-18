#ifndef OVERLAY_CEF_RENDERER_BROWSER_HANDLER_H_
#define OVERLAY_CEF_RENDERER_BROWSER_HANDLER_H_

#include <windows.h>
#include "include/cef_render_handler.h"

namespace OverlayRenderer {
namespace Cef {

class RenderHandler :public CefRenderHandler {
public:
    RenderHandler();

    virtual void GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect) override;

    virtual void OnPaint(CefRefPtr<CefBrowser> browser, CefRenderHandler::PaintElementType type,
        const CefRenderHandler::RectList& dirtyRects, const void* buffer, int width, int height) override;

    virtual void OnCursorChange(CefRefPtr<CefBrowser> browser, CefCursorHandle cursor, CursorType type,
        const CefCursorInfo& custom_cursor_info) override;

private:
    HANDLE _pipe;

    IMPLEMENT_REFCOUNTING(RenderHandler);
};

}
}

#endif
