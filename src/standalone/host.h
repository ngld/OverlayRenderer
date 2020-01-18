#ifndef OVERLAYRENDERER_STANDALONE_HOST_H_
#define OVERLAYRENDERER_STANDALONE_HOST_H_

#include <string>
#include <map>
#include <mutex>
#include <glad/glad.h>

typedef struct {
    bool initialized;
    std::string name;
    std::string url;
    bool url_changed;
    int x, y, width, height;
    bool visible, locked;
    GLuint tex;
    float tex_width, tex_height;
    
    void *tex_buffer;
    size_t tex_buffer_size, tex_buffer_used;
    float tex_buffer_width, tex_buffer_height;

    std::mutex tex_mutex;
    bool needs_update;
    bool needs_sending;
    bool hovered;
} overlay_info;

extern std::map<std::string, overlay_info> overlays;

#endif
