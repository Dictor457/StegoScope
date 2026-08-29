#include "ImageBuffer.hpp"
#include <iostream>
#include <algorithm>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

void ImageBuffer::update_gl_texture() {
    if (pixels.empty() || width <= 0 || height <= 0) return;
    if (gl_texture == 0) {
        glGenTextures(1, &gl_texture);
    }
    glBindTexture(GL_TEXTURE_2D, gl_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
}

void ImageBuffer::destroy() {
    if (gl_texture != 0) {
        glDeleteTextures(1, &gl_texture);
        gl_texture = 0;
    }
    pixels.clear();
    width = 0;
    height = 0;
}

bool ImageBuffer::load_from_file(const std::string& path) {
    int w, h, c;
    uint8_t* data = stbi_load(path.c_str(), &w, &h, &c, 4);
    if (!data) return false;

    destroy();
    width = w;
    height = h;
    pixels.assign(data, data + (w * h * 4));
    stbi_image_free(data);
    update_gl_texture();
    return true;
}

bool ImageBuffer::save_to_png(const std::string& path) const {
    if (pixels.empty() || width <= 0 || height <= 0) return false;
    return stbi_write_png(path.c_str(), width, height, 4, pixels.data(), width * 4) != 0;
}

ImageBuffer ImageBuffer::generate_gradient(int w, int h) {
    ImageBuffer img;
    img.width = w;
    img.height = h;
    img.pixels.resize(w * h * 4);

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            int idx = (y * w + x) * 4;
            img.pixels[idx + 0] = static_cast<uint8_t>((x ^ y) & 0xFF);
            img.pixels[idx + 1] = static_cast<uint8_t>((x * 255) / w);
            img.pixels[idx + 2] = static_cast<uint8_t>((y * 255) / h);
            img.pixels[idx + 3] = 255;
        }
    }
    img.update_gl_texture();
    return img;
}

ImageBuffer ImageBuffer::generate_mandelbrot(int w, int h) {
    ImageBuffer img;
    img.width = w;
    img.height = h;
    img.pixels.resize(w * h * 4);

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            double cr = (x - w / 1.5) * 4.0 / w;
            double ci = (y - h / 2.0) * 4.0 / h;
            double zr = 0.0, zi = 0.0;
            int iter = 0;
            const int max_iter = 128;

            while (zr * zr + zi * zi <= 4.0 && iter < max_iter) {
                double temp = zr * zr - zi * zi + cr;
                zi = 2.0 * zr * zi + ci;
                zr = temp;
                iter++;
            }

            int idx = (y * w + x) * 4;
            uint8_t val = static_cast<uint8_t>((iter * 255) / max_iter);
            img.pixels[idx + 0] = static_cast<uint8_t>(val * 0.4f);
            img.pixels[idx + 1] = static_cast<uint8_t>(val * 0.8f);
            img.pixels[idx + 2] = val;
            img.pixels[idx + 3] = 255;
        }
    }
    img.update_gl_texture();
    return img;
}
