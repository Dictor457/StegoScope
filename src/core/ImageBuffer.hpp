#pragma once
#include <vector>
#include <string>
#include <cstdint>
#include <GLFW/glfw3.h>

struct ImageBuffer {
    int width = 0;
    int height = 0;
    std::vector<uint8_t> pixels;
    GLuint gl_texture = 0;

    void update_gl_texture();
    void destroy();
    bool load_from_file(const std::string& path);
    bool save_to_png(const std::string& path) const;
    static ImageBuffer generate_gradient(int w = 512, int h = 512);
    static ImageBuffer generate_mandelbrot(int w = 512, int h = 512);
};
