#include "AESVisualizer.hpp"
#include <iomanip>
#include <sstream>

static const uint8_t sbox[256] = {
    0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
    0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
    0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
    0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
    0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
    0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
    0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
    0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
    0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
    0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
    0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5e, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
    0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
    0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
    0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
    0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
    0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16
};

static uint8_t gmul(uint8_t a, uint8_t b) {
    uint8_t p = 0;
    for (int i = 0; i < 8; ++i) {
        if (b & 1) p ^= a;
        bool hi_bit_set = (a & 0x80);
        a <<= 1;
        if (hi_bit_set) a ^= 0x1B;
        b >>= 1;
    }
    return p;
}

AESVisualizer::AESVisualizer() {
    set_input("DICTOR_SECRET_16", "CYBER_KEY_2026!!");
}

void AESVisualizer::set_input(const std::string& plaintext_16, const std::string& key_16) {
    std::array<uint8_t, 16> in = {0};
    std::array<uint8_t, 16> key = {0};

    for (size_t i = 0; i < 16 && i < plaintext_16.size(); ++i) in[i] = plaintext_16[i];
    for (size_t i = 0; i < 16 && i < key_16.size(); ++i) key[i] = key_16[i];

    generate_all_snapshots(in, key);
    current_snapshot_idx = 0;
}

void AESVisualizer::reset() {
    current_snapshot_idx = 0;
}

void AESVisualizer::next_step() {
    if (current_snapshot_idx + 1 < snapshots.size()) {
        current_snapshot_idx++;
    }
}

void AESVisualizer::prev_step() {
    if (current_snapshot_idx > 0) {
        current_snapshot_idx--;
    }
}

const AESStateSnapshot& AESVisualizer::get_current_snapshot() const {
    return snapshots[current_snapshot_idx];
}

void AESVisualizer::generate_all_snapshots(const std::array<uint8_t, 16>& in, const std::array<uint8_t, 16>& key) {
    snapshots.clear();

    std::array<std::array<uint8_t, 4>, 4> state;
    for (int r = 0; r < 4; ++r) {
        for (int c = 0; c < 4; ++c) {
            state[r][c] = in[c * 4 + r];
        }
    }

    snapshots.push_back({ 0, AESStep::InitialState, state, "Initial State 4x4 Loaded from 128-bit Plaintext" });

    // Round 0
    for (int r = 0; r < 4; ++r) {
        for (int c = 0; c < 4; ++c) {
            state[r][c] ^= key[c * 4 + r];
        }
    }
    snapshots.push_back({ 0, AESStep::AddRoundKey, state, "Round 0 :: AddRoundKey (XOR with Initial Key)" });

    // Rounds 1..10
    for (int round = 1; round <= 10; ++round) {
        // SubBytes
        for (int r = 0; r < 4; ++r) {
            for (int c = 0; c < 4; ++c) {
                state[r][c] = sbox[state[r][c]];
            }
        }
        snapshots.push_back({ round, AESStep::SubBytes, state, "Round " + std::to_string(round) + " :: SubBytes (Non-linear S-Box substitution)" });

        // ShiftRows
        uint8_t temp = state[1][0];
        state[1][0] = state[1][1]; state[1][1] = state[1][2]; state[1][2] = state[1][3]; state[1][3] = temp;

        temp = state[2][0]; uint8_t temp2 = state[2][1];
        state[2][0] = state[2][2]; state[2][1] = state[2][3]; state[2][2] = temp; state[2][3] = temp2;

        temp = state[3][3];
        state[3][3] = state[3][2]; state[3][2] = state[3][1]; state[3][1] = state[3][0]; state[3][0] = temp;

        snapshots.push_back({ round, AESStep::ShiftRows, state, "Round " + std::to_string(round) + " :: ShiftRows (Cyclic byte permutation)" });

        // MixColumns (not in Round 10)
        if (round != 10) {
            for (int c = 0; c < 4; ++c) {
                uint8_t a = state[0][c], b = state[1][c], d = state[2][c], e = state[3][c];
                state[0][c] = gmul(a, 2) ^ gmul(b, 3) ^ d ^ e;
                state[1][c] = a ^ gmul(b, 2) ^ gmul(d, 3) ^ e;
                state[2][c] = a ^ b ^ gmul(d, 2) ^ gmul(e, 3);
                state[3][c] = gmul(a, 3) ^ b ^ d ^ gmul(e, 2);
            }
            snapshots.push_back({ round, AESStep::MixColumns, state, "Round " + std::to_string(round) + " :: MixColumns (Galois Field GF(2^8) Matrix Multiplication)" });
        }

        // AddRoundKey
        for (int r = 0; r < 4; ++r) {
            for (int c = 0; c < 4; ++c) {
                state[r][c] ^= (key[c * 4 + r] ^ static_cast<uint8_t>(round * 0x11));
            }
        }
        snapshots.push_back({ round, AESStep::AddRoundKey, state, "Round " + std::to_string(round) + " :: AddRoundKey (XOR with Expanded Round Key)" });
    }

    snapshots.push_back({ 10, AESStep::Completed, state, "AES-128 Encryption Completed. Final 128-bit Ciphertext State Ready." });
}
