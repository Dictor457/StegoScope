#pragma once
#include "imgui.h"
#include <vector>
#include <string>
#include <filesystem>

inline void apply_liquid_glass_theme() {
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    style.WindowRounding    = 14.0f;
    style.ChildRounding     = 12.0f;
    style.FrameRounding     = 8.0f;
    style.PopupRounding     = 12.0f;
    style.ScrollbarRounding = 12.0f;
    style.GrabRounding      = 6.0f;
    style.TabRounding       = 8.0f;
    
    style.WindowBorderSize  = 1.0f;
    style.ChildBorderSize   = 1.0f;
    style.FrameBorderSize   = 1.0f;
    style.PopupBorderSize   = 1.0f;
    
    style.WindowPadding     = ImVec2(16.0f, 16.0f);
    style.FramePadding      = ImVec2(10.0f, 6.0f);
    style.ItemSpacing       = ImVec2(8.0f, 8.0f);
    style.ItemInnerSpacing  = ImVec2(6.0f, 6.0f);

    colors[ImGuiCol_Text]                  = ImVec4(0.95f, 0.96f, 0.98f, 1.00f);
    colors[ImGuiCol_TextDisabled]          = ImVec4(0.45f, 0.48f, 0.55f, 1.00f);
    colors[ImGuiCol_WindowBg]              = ImVec4(0.04f, 0.05f, 0.07f, 0.98f);
    colors[ImGuiCol_ChildBg]               = ImVec4(0.08f, 0.09f, 0.13f, 0.65f);
    colors[ImGuiCol_PopupBg]               = ImVec4(0.06f, 0.07f, 0.10f, 0.95f);
    colors[ImGuiCol_Border]                = ImVec4(1.00f, 1.00f, 1.00f, 0.08f);
    colors[ImGuiCol_BorderShadow]          = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg]               = ImVec4(0.12f, 0.14f, 0.20f, 0.50f);
    colors[ImGuiCol_FrameBgHovered]        = ImVec4(0.18f, 0.22f, 0.32f, 0.70f);
    colors[ImGuiCol_FrameBgActive]         = ImVec4(0.24f, 0.28f, 0.40f, 0.85f);
    colors[ImGuiCol_TitleBg]               = ImVec4(0.04f, 0.05f, 0.07f, 1.00f);
    colors[ImGuiCol_TitleBgActive]         = ImVec4(0.06f, 0.07f, 0.10f, 1.00f);
    colors[ImGuiCol_MenuBarBg]             = ImVec4(0.05f, 0.06f, 0.08f, 1.00f);
    colors[ImGuiCol_CheckMark]             = ImVec4(0.38f, 0.55f, 1.00f, 1.00f);
    colors[ImGuiCol_SliderGrab]            = ImVec4(0.38f, 0.55f, 1.00f, 0.90f);
    colors[ImGuiCol_SliderGrabActive]      = ImVec4(0.55f, 0.70f, 1.00f, 1.00f);
    colors[ImGuiCol_Button]                = ImVec4(0.14f, 0.16f, 0.24f, 0.75f);
    colors[ImGuiCol_ButtonHovered]         = ImVec4(0.25f, 0.30f, 0.48f, 0.85f);
    colors[ImGuiCol_ButtonActive]          = ImVec4(0.38f, 0.45f, 0.95f, 0.95f);
    colors[ImGuiCol_Header]                = ImVec4(0.16f, 0.20f, 0.30f, 0.60f);
    colors[ImGuiCol_HeaderHovered]         = ImVec4(0.25f, 0.32f, 0.48f, 0.80f);
    colors[ImGuiCol_HeaderActive]          = ImVec4(0.38f, 0.45f, 0.95f, 0.85f);
    colors[ImGuiCol_Tab]                   = ImVec4(0.08f, 0.10f, 0.14f, 0.80f);
    colors[ImGuiCol_TabHovered]            = ImVec4(0.20f, 0.25f, 0.38f, 0.90f);
    colors[ImGuiCol_TabActive]             = ImVec4(0.18f, 0.22f, 0.34f, 1.00f);
    colors[ImGuiCol_Separator]             = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
}

inline void load_clean_font(ImGuiIO& io) {
#ifdef _WIN32
    const std::vector<std::string> sans_fonts = {
        "C:\\Windows\\Fonts\\segoeui.ttf",
        "C:\\Windows\\Fonts\\arial.ttf"
    };
#else
    const std::vector<std::string> sans_fonts = {
        "/usr/share/fonts/noto/NotoSans-Medium.ttf",
        "/usr/share/fonts/noto/NotoSans-Regular.ttf",
        "/usr/share/fonts/TTF/DejaVuSans.ttf",
        "/usr/share/fonts/liberation/LiberationSans-Regular.ttf"
    };
#endif

    static const ImWchar glyph_ranges[] = {
        0x0020, 0x00FF,
        0x0400, 0x052F,
        0x2000, 0x206F,
        0x2022, 0x2022,
        0x25CF, 0x25CF,
        0,
    };

    for (const auto& path : sans_fonts) {
        if (std::filesystem::exists(path)) {
            ImFontConfig config;
            config.OversampleH = 3;
            config.OversampleV = 2;
            io.Fonts->AddFontFromFileTTF(path.c_str(), 16.0f, &config, glyph_ranges);
            return;
        }
    }
    io.Fonts->AddFontDefault();
}
