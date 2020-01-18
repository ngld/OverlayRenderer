#include "state_tracker.h"
#include "browser_life_span_handler.h"

using namespace OverlayRenderer::Cef;

void BrowserLifeSpanHandler::OnBeforeClose(CefRefPtr<CefBrowser> browser) {
    StateTracker::ClearBrowser(browser->GetIdentifier());
}
