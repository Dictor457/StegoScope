#pragma once
#include "core/ImageBuffer.hpp"
#include <array>
#include <string>

class StegoAnalysis {
public:
    static ImageBuffer extract_bit_plane(const ImageBuffer& src, int channel, int bit);
    static ImageBuffer generate_entropy_heatmap(const ImageBuffer& src, int block_size);
    static double compute_shannon_entropy(const ImageBuffer& src, std::array<int, 256>& histogram);
    static void embed_lsb(ImageBuffer& img, const std::string& payload);
    static std::string extract_lsb(const ImageBuffer& img);
};
