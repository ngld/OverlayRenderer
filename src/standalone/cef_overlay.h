#ifndef OVERLAYRENDERER_STANDALONE_CEF_OVERLAY_H_
#define OVERLAYRENDERER_STANDALONE_CEF_OVERLAY_H_

#include "host.h"

namespace OverlayRenderer {
namespace Standalone {

void InitOverlayShader(const char* glsl_version);
// void PollOverlayMessages();
void RenderOverlays();

}
}

#endif
