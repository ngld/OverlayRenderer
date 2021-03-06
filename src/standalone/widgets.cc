#include <string>
#include <windows.h>
#include <imgui.h>
#include <imgui_stdlib.h>
#include <glad/glad.h>
#include <loguru.hpp>

#include "host.h"
#include "settings.h"
#include "cef_overlay.h"
#include "widgets.h"

using namespace OverlayRenderer::Standalone;
static bool add_window_visible = false;

void render_add_window() {
    ImGui::SetNextWindowSize(ImVec2(200, 90), ImGuiCond_Once);
    ImGui::SetNextWindowPos(ImVec2(300, 300));

    ImGui::Begin("Add Overlay");

    static std::string name;
    // ImGui::SetKeyboardFocusHere();
    ImGui::InputText("Name", &name);

    if (ImGui::Button("OK")) {
        overlays[name].name = name;
        name = "";
        add_window_visible = false;
    }
    ImGui::SameLine();

    if (ImGui::Button("Cancel")) {
        name = "";
        add_window_visible = false;
    }

    ImGui::End();
}

bool render_are_you_sure(const char *id, std::string action) {
    ImGui::SetNextWindowSize(ImVec2(200, 90), ImGuiCond_Once);
    ImGui::SetNextWindowPos(ImVec2(300, 300));

    if (!ImGui::BeginPopup(id))
        return false;

    ImGui::Text("Are you sure you want to %s?", action.c_str());

    if (ImGui::Button("Yes")) {
        ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
        return true;
    }

    ImGui::SameLine();
    if (ImGui::Button("No")) {
        ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
        return false;
    }

    ImGui::EndPopup();
    return false;
}

void render_main_window() {
    ImGui::SetNextWindowSize(ImVec2(500, 300), ImGuiCond_FirstUseEver);

    ImGui::Begin("OverlayRenderer Host");

    static std::string selected = "";
    static std::string url_text;

    ImGui::BeginGroup();
        ImGui::BeginChild("overlay list", ImVec2(150, -ImGui::GetFrameHeightWithSpacing()), true);

            for (const auto &pair : overlays) {
                if (ImGui::Selectable(pair.first.c_str(), pair.first == selected)) {
                    selected = pair.first;
                    url_text = pair.second.url;
                }
            }

        ImGui::EndChild();

        if (ImGui::Button("Add")) {
            add_window_visible = true;
        }
        ImGui::SameLine();

        if (ImGui::Button("Remove")) {
            ImGui::OpenPopup("ovl_del_popup");
        }

        if (render_are_you_sure("ovl_del_popup", "delete this overlay")) {
            overlays.erase(selected);
            selected = "";
        }

    ImGui::EndGroup();
    ImGui::SameLine();

    ImGui::BeginChild("overlay settings", ImVec2(0, 0));
        if (selected == "") {
            ImGui::Text("No overlay selected.");
        } else {
            ImGui::Checkbox("Visible", &overlays[selected].visible);
            ImGui::Checkbox("Locked", &overlays[selected].locked);

            ImGui::InputText("URL", &overlays[selected].url);

            ImGui::Separator();

            if (ImGui::Button("Save")) {
                // TODO: Do this properly.
                overlays[selected].url_changed = true;
                SaveSettings();
            }
        }

    ImGui::EndChild();

    ImGui::End();
}

void render_root_widgets() {
    render_main_window();

    if (add_window_visible)
        render_add_window();

    RenderOverlays();
    // PollOverlayMessages();
}
