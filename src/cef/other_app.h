#ifndef OVERLAY_CEF_RENDERER_OTHER_APP_H_
#define OVERLAY_CEF_RENDERER_OTHER_APP_H_

#include "include/cef_app.h"
#include "include/cef_browser_process_handler.h"
#include "cef/client_app.h"

namespace OverlayRenderer {
namespace Cef {

class OtherApp : public ClientApp {
public:
    OtherApp();

private:
    IMPLEMENT_REFCOUNTING(OtherApp);
};

}
}

#endif