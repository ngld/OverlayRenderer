#include "state_tracker.h"
#include "render_handler.h"

using namespace OverlayRenderer::Cef;

RenderHandler::RenderHandler() {
    
}

void RenderHandler::GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect) {
    OverlayState& state = StateTracker::GetState(browser->GetIdentifier());

    rect.width = std::max(state.width, 1);
    rect.height = std::max(state.height, 1);
}

void RenderHandler::OnPaint(CefRefPtr<CefBrowser> browser, CefRenderHandler::PaintElementType type,
    const CefRenderHandler::RectList& dirtyRects, const void* buffer, int width, int height) {

    StateTracker::SubmitFrame(browser->GetIdentifier(), buffer, width, height);
}

void RenderHandle::OnCursorChange(CefRefPtr<CefBrowser> browser, CefCursorHandle cursor, CursorType type,
        const CefCursorInfo& custom_cursor_info) {

    // TODO
    // StateTracker::SubmitCursorChange(browser->GetIdentifier(), cursor);
}