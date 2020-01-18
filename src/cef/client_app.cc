#include "include/wrapper/cef_closure_task.h"
#include "include/base/cef_bind.h"
#include "cef/client_app.h"
#include "cef/state_tracker.h"


namespace OverlayRenderer {
namespace Cef {

ClientApp::ClientApp() {
    _browserHandler = new BrowserHandler();
    _renderHandler = new RenderProcessHandler();
}

void ClientApp::StartStatePoller() {
    _pollThread = CefThread::CreateThread("BackendPoller");
    _pollThread->GetTaskRunner()->PostTask(
        CefCreateClosureTask(base::Bind(&StateTracker::Poll)));
}

}
}
