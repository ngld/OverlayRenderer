#ifndef OVERLAYRENDERER_STANDALONE_HOST_H_
#define OVERLAYRENDERER_STANDALONE_HOST_H_

#include <string>
#include <map>

typedef struct {
    std::string name;
    std::string url;
    int x, y, width, height;
    bool visible;
} overlay_info;

extern std::map<std::string, overlay_info> overlays;

#endif
