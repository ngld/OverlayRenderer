#ifndef OVERLAYRENDERER_COMMON_LOGGING_H_
#define OVERLAYRENDERER_COMMON_LOGGING_H_

#ifdef USE_CEF_LOGGING
    #include "include/base/cef_build.h"
    #include "include/base/cef_logging.h"
#endif

// If CEF is loaded, the LOG macro is already defined so we just use that.
// Otherwise we load loguru if necessary and fallback to that.
#ifndef LOG
    #include <loguru.hpp>
#endif

#endif
