#include "browser_client.h"
#include "state_tracker.h"
#include "browser_handler.h"

using namespace OverlayRenderer::Cef;

BrowserHandler::BrowserHandler() {
}

void BrowserHandler::OnContextInitialized() {
    OverlayState* tmp_state = new OverlayState;

    tmp_state->width = 100;
    tmp_state->height = 100;
    tmp_state->url = "https://test.de";

    StateTracker::OpenNewOverlay("never user this name ever", tmp_state);
}
