#include "AppUI.hpp"
#include "imgui.h"
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <cstdio>
#include <array>
#include <filesystem>
#include <cmath>

#ifdef _WIN32
#include <windows.h>
#include <commdlg.h>
#endif

AppUI::AppUI() {
    current_image = ImageBuffer::generate_gradient(512, 512);
    refresh_analysis();
}

void AppUI::refresh_analysis() {
    global_entropy = StegoAnalysis::compute_shannon_entropy(current_image, histogram);
    processed_image.destroy();
    if (show_entropy) {
        processed_image = StegoAnalysis::generate_entropy_heatmap(current_image, block_size);
    } else {
        processed_image = StegoAnalysis::extract_bit_plane(current_image, selected_channel, selected_bit);
    }
}

void AppUI::handle_dropped_file(const std::string& path) {
    if (current_image.load_from_file(path)) {
        status_filename = std::filesystem::path(path).filename().string();
        has_custom_file = true;
        refresh_analysis();
    }
}

void AppUI::open_native_file_dialog() {
#ifdef _WIN32
    char filename[MAX_PATH] = "";
    OPENFILENAMEA ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFilter = "Images (*.png;*.jpg;*.jpeg;*.bmp)\0*.png;*.jpg;*.jpeg;*.bmp\0All Files (*.*)\0*.*\0";
    ofn.lpstrFile = filename;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
    if (GetOpenFileNameA(&ofn)) {
        handle_dropped_file(std::string(filename));
    }
#else
    std::array<char, 512> buffer;
    std::string result;
    FILE* pipe = popen("zenity --file-selection --title=\"Select Image\" --file-filter=\"Images | *.png *.jpg *.jpeg *.bmp\" 2>/dev/null", "r");
    if (pipe) {
        while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
            result += buffer.data();
        }
        pclose(pipe);
    }
    while (!result.empty() && (result.back() == '\n' || result.back() == '\r')) {
        result.pop_back();
    }
    if (!result.empty()) {
        handle_dropped_file(result);
    }
#endif
}

void AppUI::render(int display_w, int display_h) {
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(static_cast<float>(display_w), static_cast<float>(display_h)));
    
    ImGuiWindowFlags root_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | 
                                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus |
                                 ImGuiWindowFlags_MenuBar;

    ImGui::Begin("Root", nullptr, root_flags);

    if (ImGui::BeginMenuBar()) {
        ImGui::TextColored(ImVec4(0.55f, 0.70f, 1.0f, 1.0f), "StegoScope");
        ImGui::SameLine();
        ImGui::TextDisabled("|");
        ImGui::SameLine();

        if (ImGui::MenuItem("Forensics & Slices", nullptr, active_tab == 0)) active_tab = 0;
        if (ImGui::MenuItem("AES-128 Visualizer", nullptr, active_tab == 1)) active_tab = 1;
        
        ImGui::SameLine(ImGui::GetContentRegionAvail().x - 180);
        if (ImGui::BeginMenu("Samples")) {
            if (ImGui::MenuItem("Gradient Flow")) {
                current_image.destroy();
                current_image = ImageBuffer::generate_gradient(512, 512);
                status_filename = "gradient.png";
                refresh_analysis();
            }
            if (ImGui::MenuItem("Mandelbrot Set")) {
                current_image.destroy();
                current_image = ImageBuffer::generate_mandelbrot(512, 512);
                status_filename = "mandelbrot.png";
                refresh_analysis();
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    if (active_tab == 0) {
        render_stego_sidebar();
        ImGui::SameLine();
        render_stego_viewport();
    } else {
        render_aes_studio();
    }

    ImGui::End();
}

void AppUI::render_stego_sidebar() {
    ImGui::BeginChild("Sidebar", ImVec2(370.0f, 0), true);

    ImGui::TextColored(ImVec4(0.85f, 0.88f, 0.95f, 1.0f), "Image Source");
    ImGui::Spacing();
    
    if (ImGui::Button("Choose Image...", ImVec2(-1, 36))) {
        open_native_file_dialog();
    }

    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.06f, 0.08f, 0.11f, 0.50f));
    ImGui::BeginChild("DropBox", ImVec2(-1, 42), true);
    ImGui::SetCursorPosY(11);
    ImGui::TextDisabled("   Drag and drop image here");
    ImGui::EndChild();
    ImGui::PopStyleColor();

    if (ImGui::Button("Export PNG", ImVec2(-1, 32))) {
        processed_image.save_to_png("stegoscope_export.png");
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::TextColored(ImVec4(0.85f, 0.88f, 0.95f, 1.0f), "Bit Planes");
    ImGui::Spacing();
    
    const char* chan_labels[] = { "Red", "Green", "Blue" };
    ImVec4 chan_active_colors[] = { ImVec4(0.9f, 0.3f, 0.3f, 0.85f), ImVec4(0.2f, 0.8f, 0.4f, 0.85f), ImVec4(0.3f, 0.55f, 1.0f, 0.85f) };

    float label_col_w = 48.0f;
    float item_spacing_x = ImGui::GetStyle().ItemSpacing.x;
    float total_avail = ImGui::GetContentRegionAvail().x;
    float btns_total_w = total_avail - label_col_w - item_spacing_x;
    float dynamic_btn_w = std::max(22.0f, std::floor((btns_total_w - (7.0f * item_spacing_x)) / 8.0f));

    for (int c = 0; c < 3; ++c) {
        ImGui::AlignTextToFramePadding();
        ImGui::TextDisabled("%s", chan_labels[c]);
        ImGui::SameLine(label_col_w + item_spacing_x);
        
        for (int b = 7; b >= 0; --b) {
            char label[16];
            snprintf(label, sizeof(label), "%d##%d_%d", b, c, b);
            bool is_active = (!show_entropy && selected_channel == c && selected_bit == b);
            if (is_active) ImGui::PushStyleColor(ImGuiCol_Button, chan_active_colors[c]);
            
            if (ImGui::Button(label, ImVec2(dynamic_btn_w, 26))) {
                show_entropy = false;
                selected_channel = c;
                selected_bit = b;
                refresh_analysis();
            }
            if (is_active) ImGui::PopStyleColor();
            if (b > 0) ImGui::SameLine();
        }
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::TextColored(ImVec4(0.85f, 0.88f, 0.95f, 1.0f), "Information Density");
    ImGui::Spacing();
    ImGui::SliderInt("Block Size", &block_size, 4, 64, "%d px");
    if (ImGui::Button("Calculate Shannon Entropy", ImVec2(-1, 34))) {
        show_entropy = true;
        refresh_analysis();
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::TextColored(ImVec4(0.85f, 0.88f, 0.95f, 1.0f), "LSB Payload");
    ImGui::Spacing();
    ImGui::InputText("##Secret", secret_buffer, sizeof(secret_buffer));
    
    float half_btn_w = std::floor((ImGui::GetContentRegionAvail().x - item_spacing_x) * 0.5f);
    if (ImGui::Button("Embed", ImVec2(half_btn_w, 32))) {
        StegoAnalysis::embed_lsb(current_image, secret_buffer);
        refresh_analysis();
    }
    ImGui::SameLine();
    if (ImGui::Button("Extract", ImVec2(half_btn_w, 32))) {
        extracted_text = StegoAnalysis::extract_lsb(current_image);
    }

    if (!extracted_text.empty()) {
        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.05f, 0.06f, 0.09f, 0.8f));
        ImGui::InputTextMultiline("##Extracted", const_cast<char*>(extracted_text.c_str()), 
                                  extracted_text.size(), ImVec2(-1, 50), ImGuiInputTextFlags_ReadOnly);
        ImGui::PopStyleColor();
    }

    ImGui::EndChild();
}

void AppUI::render_stego_viewport() {
    ImGui::BeginChild("ViewportsArea", ImVec2(0, 0), true);

    ImGui::TextColored(ImVec4(0.3f, 0.85f, 0.5f, 1.0f), "*");
    ImGui::SameLine();
    ImGui::Text("%s (%d x %d)", status_filename.c_str(), current_image.width, current_image.height);
    ImGui::SameLine();
    ImGui::TextDisabled("|   Entropy: %.3f / 8.0 bits", global_entropy);

    ImGui::SameLine(ImGui::GetContentRegionAvail().x - 170);
    ImGui::RadioButton("Dual", &view_mode, 0);
    ImGui::SameLine();
    ImGui::RadioButton("Curtain", &view_mode, 1);
    ImGui::Separator();

    float available_w = ImGui::GetContentRegionAvail().x;
    float aspect = (current_image.width > 0) ? (static_cast<float>(current_image.height) / static_cast<float>(current_image.width)) : 1.0f;
    float max_allowed_h = 560.0f;

    if (view_mode == 0) {
        float view_w = (available_w - 20.0f) * 0.5f;
        float view_h = view_w * aspect;
        if (view_h > max_allowed_h) {
            view_h = max_allowed_h;
            view_w = view_h / aspect;
        }

        ImGui::BeginChild("LeftCard", ImVec2(view_w, view_h + 30.0f), false);
        ImGui::TextDisabled("Original");
        ImVec2 img_pos = ImGui::GetCursorScreenPos();
        if (current_image.gl_texture) {
            ImGui::Image(static_cast<ImTextureID>(current_image.gl_texture), ImVec2(view_w, view_h));
            if (ImGui::IsItemHovered()) {
                ImVec2 mouse = ImGui::GetMousePos();
                float u = (mouse.x - img_pos.x) / view_w;
                float v = (mouse.y - img_pos.y) / view_h;
                hovered_pixel_x = static_cast<int>(u * current_image.width);
                hovered_pixel_y = static_cast<int>(v * current_image.height);
                if (hovered_pixel_x >= 0 && hovered_pixel_x < current_image.width &&
                    hovered_pixel_y >= 0 && hovered_pixel_y < current_image.height) {
                    int idx = (hovered_pixel_y * current_image.width + hovered_pixel_x) * 4;
                    hovered_r = current_image.pixels[idx + 0];
                    hovered_g = current_image.pixels[idx + 1];
                    hovered_b = current_image.pixels[idx + 2];
                    hovered_a = current_image.pixels[idx + 3];
                }
            }
        }
        ImGui::EndChild();

        ImGui::SameLine();

        ImGui::BeginChild("RightCard", ImVec2(view_w, view_h + 30.0f), false);
        ImGui::TextDisabled(show_entropy ? "Entropy Heatmap" : "Bit Slice Analysis");
        if (processed_image.gl_texture) {
            ImGui::Image(static_cast<ImTextureID>(processed_image.gl_texture), ImVec2(view_w, view_h));
        }
        ImGui::EndChild();
    } else {
        ImGui::SliderFloat("Curtain Divider", &curtain_split, 0.0f, 1.0f, "%.2f");
        float single_w = std::min(available_w, 800.0f);
        float single_h = single_w * aspect;
        if (single_h > max_allowed_h) {
            single_h = max_allowed_h;
            single_w = single_h / aspect;
        }
        
        ImVec2 p0 = ImGui::GetCursorScreenPos();
        ImDrawList* draw_list = ImGui::GetWindowDrawList();

        if (current_image.gl_texture && processed_image.gl_texture) {
            draw_list->AddImage(static_cast<ImTextureID>(current_image.gl_texture),
                                p0, ImVec2(p0.x + single_w * curtain_split, p0.y + single_h),
                                ImVec2(0, 0), ImVec2(curtain_split, 1.0f));
            draw_list->AddImage(static_cast<ImTextureID>(processed_image.gl_texture),
                                ImVec2(p0.x + single_w * curtain_split, p0.y), ImVec2(p0.x + single_w, p0.y + single_h),
                                ImVec2(curtain_split, 0), ImVec2(1.0f, 1.0f));
            draw_list->AddLine(ImVec2(p0.x + single_w * curtain_split, p0.y),
                               ImVec2(p0.x + single_w * curtain_split, p0.y + single_h),
                               IM_COL32(99, 102, 241, 255), 2.5f);
        }
        ImGui::Dummy(ImVec2(single_w, single_h));
    }

    ImGui::Spacing();
    ImGui::Separator();
    if (hovered_pixel_x >= 0) {
        std::stringstream ss;
        ss << "#" << std::hex << std::uppercase << std::setfill('0')
           << std::setw(2) << (int)hovered_r
           << std::setw(2) << (int)hovered_g
           << std::setw(2) << (int)hovered_b;

        ImGui::Text("X %-4d Y %-4d   |   %s   |   R %-3d  G %-3d  B %-3d", 
                    hovered_pixel_x, hovered_pixel_y, ss.str().c_str(), hovered_r, hovered_g, hovered_b);
    } else {
        ImGui::TextDisabled("Hover cursor over the image to inspect pixel values");
    }

    ImGui::Spacing();
    std::vector<float> hist_norm(256);
    int max_val = *std::max_element(histogram.begin(), histogram.end());
    for (int i = 0; i < 256; ++i) {
        hist_norm[i] = max_val > 0 ? static_cast<float>(histogram[i]) / max_val : 0.0f;
    }
    ImGui::PlotHistogram("##Histogram", hist_norm.data(), 256, 0, nullptr, 0.0f, 1.0f, ImVec2(-1, 48));

    ImGui::EndChild();
}

void AppUI::render_aes_studio() {
    ImGui::BeginChild("AESStudio", ImVec2(0, 0), true);

    ImGui::TextColored(ImVec4(0.85f, 0.88f, 0.95f, 1.0f), "AES-128 Rijndael State Machine");
    ImGui::TextDisabled("Step-by-step matrix transformation across 10 rounds");
    ImGui::Separator();

    ImGui::BeginChild("AESControls", ImVec2(360, 0), true);
    ImGui::TextDisabled("Plaintext (16 bytes)");
    ImGui::InputText("##Plain", aes_plain_buf, sizeof(aes_plain_buf));
    
    ImGui::TextDisabled("Key (16 bytes)");
    ImGui::InputText("##Key", aes_key_buf, sizeof(aes_key_buf));

    if (ImGui::Button("Update Cipher Stream", ImVec2(-1, 34))) {
        aes_engine.set_input(aes_plain_buf, aes_key_buf);
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    float half_btn_w = std::floor((ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) * 0.5f);
    if (ImGui::Button("Prev Step", ImVec2(half_btn_w, 34))) aes_engine.prev_step();
    ImGui::SameLine();
    if (ImGui::Button("Next Step", ImVec2(half_btn_w, 34))) aes_engine.next_step();
    
    if (ImGui::Button("Reset", ImVec2(-1, 28))) aes_engine.reset();

    ImGui::Spacing();
    ImGui::TextDisabled("Step %zu of %zu", aes_engine.get_current_index() + 1, aes_engine.get_total_snapshots());
    ImGui::EndChild();

    ImGui::SameLine();

    ImGui::BeginChild("AESMatrixView", ImVec2(0, 0), true);
    const auto& snap = aes_engine.get_current_snapshot();

    ImGui::TextColored(ImVec4(0.55f, 0.70f, 1.0f, 1.0f), "%s", snap.description.c_str());
    ImGui::TextDisabled("Round %d / 10", snap.round);
    ImGui::Spacing();

    for (int r = 0; r < 4; ++r) {
        for (int c = 0; c < 4; ++c) {
            std::stringstream ss;
            ss << "0x" << std::hex << std::uppercase << std::setfill('0') << std::setw(2) << (int)snap.matrix[r][c];
            
            ImVec4 cell_color = ImVec4(0.12f, 0.14f, 0.20f, 0.6f);
            if (snap.step == AESStep::SubBytes) cell_color = ImVec4(0.38f, 0.20f, 0.50f, 0.7f);
            else if (snap.step == AESStep::ShiftRows) cell_color = ImVec4(0.20f, 0.35f, 0.55f, 0.7f);
            else if (snap.step == AESStep::MixColumns) cell_color = ImVec4(0.50f, 0.32f, 0.15f, 0.7f);
            else if (snap.step == AESStep::AddRoundKey) cell_color = ImVec4(0.18f, 0.45f, 0.28f, 0.7f);

            ImGui::PushStyleColor(ImGuiCol_Button, cell_color);
            ImGui::Button(ss.str().c_str(), ImVec2(78, 52));
            ImGui::PopStyleColor();
            if (c < 3) ImGui::SameLine();
        }
    }

    ImGui::EndChild();
    ImGui::EndChild();
}
