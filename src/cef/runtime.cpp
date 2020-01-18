#include <windows.h>

#include "include/cef_app.h"
#include "include/base/cef_scoped_ptr.h"
#include "include/cef_command_line.h"
#include "include/cef_sandbox_win.h"
#include "browser_client.h"

#include "cef/client_app.h"
#include "cef/other_app.h"

using namespace OverlayRenderer::Cef;


int main(int argc, char** argv) {
    CefEnableHighDPISupport();

#ifdef _WIN32
    CefMainArgs main_args(GetModuleHandle(NULL));
#else
    CefMainArgs main_args(argc, argv);
#endif

    void* sandbox_info = NULL;

#if defined(CEF_USE_SANDBOX)
#ifdef _WIN32
    LOG(INFO) << "Initializing sandbox.";

    // Manage the life span of the sandbox information object. This is necessary
    // for sandbox support on Windows. See cef_sandbox_win.h for complete details.
    CefScopedSandboxInfo scoped_sandbox;
    sandbox_info = scoped_sandbox.sandbox_info();
#else
    #error "Sandbox logic not yet implemented for this platform."
#endif
#endif

    CefRefPtr<ClientApp> app = new ClientApp();

    DLOG(INFO) << "Launching CEF.";

    // Execute the secondary process, if any.
    int exit_code = CefExecuteProcess(main_args, app, sandbox_info);
    if (exit_code >= 0)
        return exit_code;

    CefSettings settings;
    settings.windowless_rendering_enabled = true;

#if !defined(CEF_USE_SANDBOX)
    settings.no_sandbox = true;
#endif

    // Initialize CEF.
    CefInitialize(main_args, settings, app, sandbox_info);

    app->StartStatePoller();

    DLOG(INFO) << "Entering CEF loop.";
    CefRunMessageLoop();

    CefShutdown();

    return 0;
}
