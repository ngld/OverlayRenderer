#include <imgui.h>
#include <glad/glad.h>
#include <loguru.hpp>

#include "../common/packets.h"
#include "../common/win_pipe_wrapper.h"
#include "packets.pb.h"
#include "cef_overlay.h"

#define IN_SIZE 300
#define OUT_SIZE 0x500000

static inline bool operator==(const ImVec2& lhs, const ImVec2& rhs) { return lhs.x == rhs.x && lhs.y == rhs.y; }
static inline bool operator!=(const ImVec2& lhs, const ImVec2& rhs) { return lhs.x != rhs.x && lhs.y != rhs.y; }

namespace OverlayRenderer {
namespace Standalone {

typedef struct {
    ImTextureID tex;
    ImVec2 size;
} imageParams;

static const GLchar* vertex_shader_bgra2rgba = "\n"
    "layout (location = 0) in vec2 Position;\n"
    "layout (location = 1) in vec2 UV;\n"
    "uniform mat4 ProjMtx;\n"
    "out vec2 Frag_UV;\n"
    "void main()\n"
    "{\n"
    "    Frag_UV = UV;\n"
    "    gl_Position = ProjMtx * vec4(Position.xy,0,1);\n"
    "}\n";

static const GLchar* fragment_shader_bgra2rgba = "\n"
    "in vec2 Frag_UV;\n"
    "uniform sampler2D Texture;\n"
    "layout (location = 0) out vec4 Out_Color;\n"
    "void main()\n"
    "{\n"
    "    vec4 color = texture(Texture, Frag_UV.st);\n"
    "    Out_Color = vec4(color.z, color.y, color.x, color.w);\n"
    "}\n";

static GLuint g_VertHandle, g_FragHandle, g_ShaderHandle, g_AttribLocationProjMtx,
    g_AttribLocationTex, g_AttribLocationVtxPos, g_AttribLocationVtxUV;


static bool CheckShader(GLuint handle, const char* desc)
{
    GLint status = 0, log_length = 0;
    glGetShaderiv(handle, GL_COMPILE_STATUS, &status);
    glGetShaderiv(handle, GL_INFO_LOG_LENGTH, &log_length);
    if ((GLboolean)status == GL_FALSE)
        fprintf(stderr, "ERROR: InitOverlayShader: failed to compile %s!\n", desc);
    if (log_length > 1)
    {
        ImVector<char> buf;
        buf.resize((int)(log_length + 1));
        glGetShaderInfoLog(handle, log_length, NULL, (GLchar*)buf.begin());
        fprintf(stderr, "%s\n", buf.begin());
    }
    return (GLboolean)status == GL_TRUE;
}

// If you get an error please report on GitHub. You may try different GL context version or GLSL version.
static bool CheckProgram(GLuint handle, const char* desc)
{
    GLint status = 0, log_length = 0;
    glGetProgramiv(handle, GL_LINK_STATUS, &status);
    glGetProgramiv(handle, GL_INFO_LOG_LENGTH, &log_length);
    if ((GLboolean)status == GL_FALSE)
        fprintf(stderr, "ERROR: InitOverlayShader: failed to link %s!\n", desc);
    if (log_length > 1)
    {
        ImVector<char> buf;
        buf.resize((int)(log_length + 1));
        glGetProgramInfoLog(handle, log_length, NULL, (GLchar*)buf.begin());
        fprintf(stderr, "%s\n", buf.begin());
    }
    return (GLboolean)status == GL_TRUE;
}

void InitOverlayShader(const char* glsl_version) {
    // Create shaders
    const GLchar* vertex_shader_with_version[2] = { glsl_version, vertex_shader_bgra2rgba };
    g_VertHandle = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(g_VertHandle, 2, vertex_shader_with_version, NULL);
    glCompileShader(g_VertHandle);
    CheckShader(g_VertHandle, "vertex shader");

    const GLchar* fragment_shader_with_version[2] = { glsl_version, fragment_shader_bgra2rgba };
    g_FragHandle = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(g_FragHandle, 2, fragment_shader_with_version, NULL);
    glCompileShader(g_FragHandle);
    CheckShader(g_FragHandle, "fragment shader");

    g_ShaderHandle = glCreateProgram();
    glAttachShader(g_ShaderHandle, g_VertHandle);
    glAttachShader(g_ShaderHandle, g_FragHandle);
    glLinkProgram(g_ShaderHandle);
    CheckProgram(g_ShaderHandle, "shader program");

    g_AttribLocationTex = glGetUniformLocation(g_ShaderHandle, "Texture");
    g_AttribLocationProjMtx = glGetUniformLocation(g_ShaderHandle, "ProjMtx");
    g_AttribLocationVtxPos = glGetAttribLocation(g_ShaderHandle, "Position");
    g_AttribLocationVtxUV = glGetAttribLocation(g_ShaderHandle, "UV");
}

void DrawBgraTex(const ImDrawList* cmd_list, const ImDrawCmd* pcmd) {
    imageParams* params = (imageParams*) pcmd->UserCallbackData;
    ImDrawData* draw_data = ImGui::GetDrawData();

    float L = draw_data->DisplayPos.x;
    float R = draw_data->DisplayPos.x + draw_data->DisplaySize.x;
    float T = draw_data->DisplayPos.y;
    float B = draw_data->DisplayPos.y + draw_data->DisplaySize.y;
    const float ortho_projection[4][4] =
    {
        { 2.0f/(R-L),   0.0f,         0.0f,   0.0f },
        { 0.0f,         2.0f/(T-B),   0.0f,   0.0f },
        { 0.0f,         0.0f,        -1.0f,   0.0f },
        { (R+L)/(L-R),  (T+B)/(B-T),  0.0f,   1.0f },
    };

    glUseProgram(g_ShaderHandle);
    glUniform1i(g_AttribLocationTex, 0);
    glUniformMatrix4fv(g_AttribLocationProjMtx, 1, GL_FALSE, &ortho_projection[0][0]);

    free(params);
}

bool SendOverlayMessage(Packet &pkt) {
    static Common::WinPipeWrapper* pipe = nullptr;

    if (pipe == nullptr) {
        pipe = new Common::WinPipeWrapper("OverlayRenderer-in");
    }

    pipe->MaybeConnect();
    return pipe->Send(pkt);

#if 0
    static PacketWrapper* wrapper = nullptr;
    static message_queue mq_send(open_only, "OverlayRenderer-in");

    if (wrapper == nullptr)
        wrapper = (PacketWrapper*) malloc(IN_SIZE);

    wrapper->size = pkt.ByteSizeLong();
    pkt.SerializeToArray((void*) &wrapper->buffer, wrapper->size);

    try {
        return mq_send.try_send((void*) wrapper, sizeof(wrapper->size) + wrapper->size, 0);
    } catch(std::exception ex) {
        LOG_F(ERROR, "Failed to send: %s", ex.what());
        return false;
    }
#endif
}

void SendOverlayConfig() {
    static bool first_time = true;
    Packet pkt;

    if (first_time) {
        pkt.Clear();
        pkt.set_type(Packet_Type_CLEAR);
        SendOverlayMessage(pkt);

        first_time = false;
    }

    for (auto& overlay : overlays) {
        auto& info = overlay.second;

        if (!info.initialized) {
            pkt.Clear();
            pkt.set_type(Packet_Type_INIT);
            pkt.set_name(info.name);
            pkt.set_url(info.url);
            
            if (!SendOverlayMessage(pkt)) {
                LOG_F(WARNING, "Failed to send init packet for %s!", info.name.c_str());
            } else {
                info.initialized = true;
            }
        }

        if (info.needs_sending) {
            pkt.Clear();
            pkt.set_type(Packet_Type_SIZE);
            pkt.set_name(info.name);

            Size* size = pkt.mutable_size();
            size->set_width(info.width);
            size->set_height(info.height);

            if (SendOverlayMessage(pkt)) {
                info.needs_sending = false;
            }
        }

        if (info.url_changed) {
            pkt.Clear();
            pkt.set_type(Packet_Type_URL);
            pkt.set_name(info.name);
            pkt.set_url(info.url);

            if (SendOverlayMessage(pkt)) {
                info.url_changed = false;
            }
        }
    }
}

void UpdateOverlays() {
    for (auto& overlay : overlays) {
        auto& info = overlay.second;

        if (info.needs_update) {
            if (info.tex_mutex.try_lock()) {
                std::lock_guard<std::mutex> lock(info.tex_mutex, std::adopt_lock);

                GLint last_texture;
                GLuint& tex = info.tex;
                int w, h;
                
                // LOG_F(INFO, "Frame!");
                if (tex == 0) {
                    glGenTextures(1, &tex);
                    glBindTexture(GL_TEXTURE_2D, tex);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                } else {
                    glBindTexture(GL_TEXTURE_2D, tex);
                }

                glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
#ifdef GL_UNPACK_ROW_LENGTH
                glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif

                info.tex_width  = w = info.tex_buffer_width;
                info.tex_height = h = info.tex_buffer_height;
                if (w * h * 4 == info.tex_buffer_used) {
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0,
                        GL_RGBA, GL_UNSIGNED_BYTE, info.tex_buffer);
                } else {
                    LOG_F(ERROR, "Expected texture size %d but got %llu! (%d x %d)", w * h * 4,
                        info.tex_buffer_used, w, h);
                }
                glBindTexture(GL_TEXTURE_2D, last_texture);

                info.needs_update = false;
            }
        }
    }
}

#if 0
void PollOverlayMessages() {
    Packet pkt;
    static PacketWrapper* buffer = nullptr;

    if (buffer == nullptr)
        buffer = (PacketWrapper*) malloc(OUT_SIZE);

    size_t received;
    unsigned int prio;

    try {
        message_queue mq_recv(open_only, "OverlayRenderer-out");
        
        while (mq_recv.try_receive((void*) buffer, OUT_SIZE, received, prio)) {
            pkt.Clear();
            if (!pkt.ParseFromArray((void*) &buffer->buffer, buffer->size)) {
                LOG_F(ERROR, "Received invalid packet! (Size: %llu; %s)", received, pkt.DebugString().c_str());
                continue;
            }

            std::string name = pkt.name();
            GLint last_texture;
            GLuint& tex = overlays[name].tex;
            int w, h;

            switch (pkt.type()) {
                case Packet_Type_FRAME:
                    LOG_F(INFO, "Frame!");
                    if (tex == 0) {
                        glGenTextures(1, &tex);
                        glBindTexture(GL_TEXTURE_2D, tex);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                    } else {
                        glBindTexture(GL_TEXTURE_2D, tex);
                    }

                    glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
    #ifdef GL_UNPACK_ROW_LENGTH
                    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    #endif

                    overlays[name].tex_width  = w = pkt.size().width();
                    overlays[name].tex_height = h = pkt.size().height();
                    if (w * h * 4 == pkt.frame().length()) {
                        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0,
                            GL_RGBA, GL_UNSIGNED_BYTE, pkt.frame().c_str());
                    } else {
                        LOG_F(ERROR, "Expected texture size %d but got %llu! (%d x %d)", w * h * 4,
                            pkt.frame().length(), w, h);
                    }
                    glBindTexture(GL_TEXTURE_2D, last_texture);
                    break;

                default:
                    LOG_F(ERROR, "Encountered unkown packet type %d!", pkt.type());
            }
        }
    } catch(std::exception ex) {
        LOG_F(ERROR, "Failed to receive: %s", ex.what());
    }

    static bool first_time = true;
    if (first_time) {
        pkt.Clear();
        pkt.set_type(Packet_Type_CLEAR);
        SendOverlayMessage(pkt);

        first_time = false;
    }

    for (auto& overlay : overlays) {
        auto& info = overlay.second;

        if (!info.initialized) {
            pkt.Clear();
            pkt.set_type(Packet_Type_INIT);
            pkt.set_name(info.name);
            pkt.set_url(info.url);
            
            if (!SendOverlayMessage(pkt)) {
                LOG_F(WARNING, "Failed to send init packet for %s!", info.name.c_str());
            } else {
                info.initialized = true;
            }
        }

        if (info.needs_sending) {
            pkt.Clear();
            pkt.set_type(Packet_Type_SIZE);
            pkt.set_name(info.name);

            Size* size = pkt.mutable_size();
            size->set_width(info.width);
            size->set_height(info.height);

            if (SendOverlayMessage(pkt)) {
                info.needs_sending = false;
            }
        }

        if (info.url_changed) {
            pkt.Clear();
            pkt.set_type(Packet_Type_URL);
            pkt.set_name(info.name);
            pkt.set_url(info.url);

            if (SendOverlayMessage(pkt)) {
                info.url_changed = false;
            }
        }
    }
}
#endif

void RenderOverlays() {
    UpdateOverlays();

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

    Packet pkt;
    ImGuiIO &io = ImGui::GetIO();
    static ImVec2 mousePos;
    float mouseWheel;

    for (auto &pair : overlays) {
        auto &info = pair.second;
        if (!info.visible) continue;

        ImGui::SetNextWindowSize(ImVec2(info.width, info.height), ImGuiCond_Appearing);
        ImGui::SetNextWindowPos(ImVec2(info.x, info.y), ImGuiCond_Appearing);

        std::string title = "Overlay: " + info.name;

        ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoBackground;
        
        if (info.locked) {
            windowFlags |= ImGuiWindowFlags_NoMove;
        }

        if (ImGui::Begin(title.c_str(), nullptr, windowFlags)) {
            ImVec2 pos = ImGui::GetWindowPos();
            info.x = pos.x;
            info.y = pos.y;

            ImVec2 size = ImGui::GetWindowSize();
            if (info.width != size.x || info.height != size.y) {
                info.width = size.x;
                info.height = size.y;
                info.needs_sending = true;
            }

            ImGui::GetWindowDrawList()->AddCallback(&DrawBgraTex, 0);
            ImGui::Image((ImTextureID)(intptr_t)info.tex, ImVec2(info.tex_width, info.tex_height));
            ImGui::GetWindowDrawList()->AddCallback(ImDrawCallback_ResetRenderState, nullptr);

            auto has_hover = ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows);
            // Only handle mouse events if the mouse is actually inside the window.
            if (has_hover) {
                info.hovered = has_hover;

                if (mousePos != io.MousePos) {
                    mousePos = io.MousePos;

                    pkt.Clear();
                    pkt.set_type(Packet_Type_MOUSEMOVE);
                    pkt.set_name(info.name);
                    
                    Size *size = pkt.mutable_size();
                    size->set_width(mousePos.x - info.x);
                    size->set_height(mousePos.y - info.y);

                    SendOverlayMessage(pkt);
                }
                
                for (uint8_t i = 0; i < 3; ++i) {
                    if (io.MouseClicked[i] || io.MouseReleased[i]) {
                        pkt.Clear();
                        pkt.set_type(Packet_Type_MOUSECLICK);
                        pkt.set_name(info.name);
                        pkt.set_button_down(io.MouseClicked[i]);

                        Size* size = pkt.mutable_size();
                        size->set_width(mousePos.x - info.x);
                        size->set_height(mousePos.y - info.y);

                        switch (i) {
                            case 0:
                                pkt.set_button(PKT_MB_LEFT);
                                break;
                            case 1:
                                pkt.set_button(PKT_MB_RIGHT);
                                break;
                            case 2:
                                pkt.set_button(PKT_MB_MIDDLE);
                                break;
                        }

                        SendOverlayMessage(pkt);
                        break;
                    }
                }

                if (mouseWheel != io.MouseWheel) {
                    mouseWheel = io.MouseWheel;

                    pkt.Clear();
                    pkt.set_type(Packet_Type_MOUSESCROLL);
                    pkt.set_name(info.name);

                    Size *size = pkt.mutable_size();
                    size->set_width(mousePos.x - info.x);
                    size->set_height(mousePos.y - info.y);

                    size = pkt.mutable_delta();
                    size->set_width(0);
                    size->set_height(mouseWheel * 100);

                    SendOverlayMessage(pkt);
                }
            } else if (info.hovered) {
                info.hovered = false;

                pkt.Clear();
                pkt.set_type(Packet_Type_CAPTURE_LOST);
                pkt.set_name(info.name);

                Size* size = pkt.mutable_size();
                size->set_width(mousePos.x - info.x);
                size->set_width(mousePos.y - info.y);

                SendOverlayMessage(pkt);
            }
        }

        ImGui::End();
    }

    ImGui::PopStyleVar();
    SendOverlayConfig();
}

}
}
