#include "StegoAnalysis.hpp"
#include <cmath>
#include <cstring>
#include <algorithm>

ImageBuffer StegoAnalysis::extract_bit_plane(const ImageBuffer& src, int channel, int bit) {
    ImageBuffer dst;
    dst.width = src.width;
    dst.height = src.height;
    dst.pixels.resize(src.pixels.size());

    for (size_t i = 0; i < src.pixels.size(); i += 4) {
        uint8_t val = 0;
        if (channel >= 0 && channel <= 2) {
            uint8_t byte_val = src.pixels[i + channel];
            uint8_t bit_val = (byte_val >> bit) & 1;
            val = bit_val ? 255 : 0;
        }
        dst.pixels[i + 0] = (channel == 0) ? val : (val ? 40 : 0);
        dst.pixels[i + 1] = (channel == 1) ? val : (val ? 40 : 0);
        dst.pixels[i + 2] = (channel == 2) ? val : (val ? 40 : 0);
        dst.pixels[i + 3] = 255;
    }
    dst.update_gl_texture();
    return dst;
}

double StegoAnalysis::compute_shannon_entropy(const ImageBuffer& src, std::array<int, 256>& histogram) {
    histogram.fill(0);
    if (src.pixels.empty()) return 0.0;

    for (size_t i = 0; i < src.pixels.size(); i += 4) {
        uint8_t lum = static_cast<uint8_t>(0.299f * src.pixels[i] + 0.587f * src.pixels[i+1] + 0.114f * src.pixels[i+2]);
        histogram[lum]++;
    }

    double entropy = 0.0;
    size_t total = src.width * src.height;
    for (int c : histogram) {
        if (c > 0) {
            double p = static_cast<double>(c) / total;
            entropy -= p * std::log2(p);
        }
    }
    return entropy;
}

ImageBuffer StegoAnalysis::generate_entropy_heatmap(const ImageBuffer& src, int block_size) {
    ImageBuffer dst;
    dst.width = src.width;
    dst.height = src.height;
    dst.pixels.resize(src.pixels.size());

    int w = src.width;
    int h = src.height;

    for (int by = 0; by < h; by += block_size) {
        for (int bx = 0; bx < w; bx += block_size) {
            std::array<int, 256> hist = {0};
            int count = 0;

            for (int y = by; y < std::min(by + block_size, h); ++y) {
                for (int x = bx; x < std::min(bx + block_size, w); ++x) {
                    int idx = (y * w + x) * 4;
                    uint8_t lum = static_cast<uint8_t>(0.299f * src.pixels[idx] + 0.587f * src.pixels[idx+1] + 0.114f * src.pixels[idx+2]);
                    hist[lum]++;
                    count++;
                }
            }

            double entropy = 0.0;
            if (count > 0) {
                for (int c : hist) {
                    if (c > 0) {
                        double p = static_cast<double>(c) / count;
                        entropy -= p * std::log2(p);
                    }
                }
            }

            float norm = std::clamp(static_cast<float>(entropy / 8.0), 0.0f, 1.0f);
            uint8_t r = static_cast<uint8_t>(std::clamp(2.0f * (norm - 0.4f), 0.0f, 1.0f) * 255.0f);
            uint8_t g = static_cast<uint8_t>((1.0f - std::abs(2.0f * (norm - 0.5f))) * 255.0f);
            uint8_t b = static_cast<uint8_t>(std::clamp(2.0f * (0.6f - norm), 0.0f, 1.0f) * 255.0f);

            for (int y = by; y < std::min(by + block_size, h); ++y) {
                for (int x = bx; x < std::min(bx + block_size, w); ++x) {
                    int idx = (y * w + x) * 4;
                    dst.pixels[idx + 0] = r;
                    dst.pixels[idx + 1] = g;
                    dst.pixels[idx + 2] = b;
                    dst.pixels[idx + 3] = 255;
                }
            }
        }
    }
    dst.update_gl_texture();
    return dst;
}

void StegoAnalysis::embed_lsb(ImageBuffer& img, const std::string& payload) {
    uint32_t len = static_cast<uint32_t>(payload.size());
    std::vector<uint8_t> buffer(4 + len);
    std::memcpy(buffer.data(), &len, 4);
    std::memcpy(buffer.data() + 4, payload.data(), len);

    size_t bit_idx = 0;
    size_t total_bits = buffer.size() * 8;

    for (size_t i = 0; i < img.pixels.size() && bit_idx < total_bits; ++i) {
        if (i % 4 == 3) continue;
        uint8_t byte_val = buffer[bit_idx / 8];
        uint8_t bit = (byte_val >> (7 - (bit_idx % 8))) & 1;
        img.pixels[i] = (img.pixels[i] & ~1) | bit;
        bit_idx++;
    }
    img.update_gl_texture();
}

std::string StegoAnalysis::extract_lsb(const ImageBuffer& img) {
    std::vector<uint8_t> raw_bits;
    for (size_t i = 0; i < img.pixels.size(); ++i) {
        if (i % 4 == 3) continue;
        raw_bits.push_back(img.pixels[i] & 1);
    }

    if (raw_bits.size() < 32) return "";
    uint32_t len = 0;
    for (int b = 0; b < 4; ++b) {
        uint8_t byte_val = 0;
        for (int bit = 0; bit < 8; ++bit) {
            byte_val = (byte_val << 1) | raw_bits[b * 8 + bit];
        }
        len |= (static_cast<uint32_t>(byte_val) << (b * 8));
    }

    if (len == 0 || len > 100000 || (32 + len * 8) > raw_bits.size()) {
        return "[No valid LSB signature found in payload]";
    }

    std::string result;
    result.resize(len);
    for (size_t b = 0; b < len; ++b) {
        uint8_t byte_val = 0;
        for (int bit = 0; bit < 8; ++bit) {
            byte_val = (byte_val << 1) | raw_bits[32 + b * 8 + bit];
        }
        result[b] = static_cast<char>(byte_val);
    }
    return result;
}
