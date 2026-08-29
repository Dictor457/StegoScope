#pragma once
#include "core/ImageBuffer.hpp"
#include "stego/StegoAnalysis.hpp"
#include "crypto/AESVisualizer.hpp"
#include <array>
#include <string>

class AppUI {
public:
    AppUI();
    void render(int display_w, int display_h);
    void handle_dropped_file(const std::string& path);

private:
    ImageBuffer current_image;
    ImageBuffer processed_image;
    AESVisualizer aes_engine;

    int active_tab = 0;
    int selected_channel = 0;
    int selected_bit = 0;
    int block_size = 16;
    bool show_entropy = false;
    int view_mode = 0;
    float curtain_split = 0.5f;

    char secret_buffer[512] = "Secret message payload.";
    std::string extracted_text = "";
    std::string status_filename = "gradient.png";
    bool has_custom_file = false;

    char aes_plain_buf[64] = "DICTOR_CIPHER_16";
    char aes_key_buf[64] = "SECRET_KEY_ARCH!";

    std::array<int, 256> histogram = {0};
    double global_entropy = 0.0;

    int hovered_pixel_x = -1;
    int hovered_pixel_y = -1;
    uint8_t hovered_r = 0, hovered_g = 0, hovered_b = 0, hovered_a = 0;

    void render_stego_sidebar();
    void render_stego_viewport();
    void render_aes_studio();
    void open_native_file_dialog();
    void refresh_analysis();
};
