#pragma once
#include <array>
#include <vector>
#include <string>
#include <cstdint>

enum class AESStep {
    InitialState,
    AddRoundKey,
    SubBytes,
    ShiftRows,
    MixColumns,
    Completed
};

struct AESStateSnapshot {
    int round = 0;
    AESStep step = AESStep::InitialState;
    std::array<std::array<uint8_t, 4>, 4> matrix;
    std::string description;
};

class AESVisualizer {
public:
    AESVisualizer();
    void set_input(const std::string& plaintext_16, const std::string& key_16);
    void reset();
    void next_step();
    void prev_step();
    const AESStateSnapshot& get_current_snapshot() const;
    size_t get_current_index() const { return current_snapshot_idx; }
    size_t get_total_snapshots() const { return snapshots.size(); }

private:
    std::vector<AESStateSnapshot> snapshots;
    size_t current_snapshot_idx = 0;

    void generate_all_snapshots(const std::array<uint8_t, 16>& in, const std::array<uint8_t, 16>& key);
};
