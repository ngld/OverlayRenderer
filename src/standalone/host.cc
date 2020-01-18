#include <stdio.h>
#include <map>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <imgui_stdlib.h>
#include <loguru.hpp>

#include "settings.h"
#include "widgets.h"
#include "cef_overlay.h"
#include "pipe_thread.h"
#include "host.h"

using namespace OverlayRenderer::Standalone;

std::map<std::string, overlay_info> overlays;

static void glfw_error_callback(int error, const char* description) {
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

int main(int argc, char** argv) {
    loguru::init(argc, argv);
    loguru::add_file("host.log", loguru::Append, loguru::Verbosity_MAX);

    glfwSetErrorCallback(glfw_error_callback);
    
    DLOG_F(INFO, "Initializing GLFW.");
    if (!glfwInit())
        return 1;

    const char* glsl_version = "#version 450";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    //glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);

    const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

    DLOG_F(INFO, "Creating window.");
    GLFWwindow* window = glfwCreateWindow(mode->width, mode->height, "OverlayRenderer Host", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return 1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    DLOG_F(INFO, "Loading GL.");
    if (!gladLoadGL()) {
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }

    DLOG_F(INFO, "Initializing ImGui.");
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    
    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
    InitOverlayShader(glsl_version);

    bool show_demo_window = true;
    ImVec4 clear_color = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

    DLOG_F(INFO, "Loading settings.");
    LoadSettings();

    DLOG_F(INFO, "Starting frame receiver.");
    StartFrameReceiver();

    DLOG_F(INFO, "Entering main loop.");
    while (!glfwWindowShouldClose(window)) {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        render_root_widgets();

        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        ImGui::Render();
        
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);

        // glDisable(GL_BLEND);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    SaveSettings();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
