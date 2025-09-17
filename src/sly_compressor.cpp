#include "fs.hpp"
#include <array>
#include <iostream>
#include <chrono>
#include <optional>
#include <vector>
#include <unordered_map>

constexpr size_t CHUNK_SIZE = 0x2000;
constexpr size_t CHUNK_SIZE_MASK = CHUNK_SIZE - 1;
constexpr size_t MAX_MATCH = 10;
constexpr size_t MAX_CANDIDATES_KEEP = 32;

// Sliding dictionary helper class

template <typename T, size_t Size>
class SlidingDict {
public:
    struct LookupResult { size_t offset; size_t size; };

    bool push(T elem) {
        buffer[index] = elem;
        ++absolute_pos;
        if (absolute_pos >= 2) {
            size_t pair_abs_index = absolute_pos - 2;
            size_t pair_win_index = (index + Size - 1) & (Size - 1);
            uint16_t h = hash_pair(buffer[pair_win_index], buffer[(pair_win_index + 1) & (Size - 1)]);
            auto &vec = hash_table[h];
            vec.emplace_back(pair_win_index, pair_abs_index);
            if (vec.size() > MAX_CANDIDATES_KEEP) vec.erase(vec.begin());
        }

        index = (index + 1) & (Size - 1);
        return index == 0;
    }

    std::optional<LookupResult> look_up(const std::vector<T>& input, size_t pos) const {
        if (pos + 1 >= input.size() || absolute_pos < 2) return std::nullopt;

        size_t max_len = std::min(MAX_MATCH, input.size() - pos);
        uint16_t h = hash_pair(input[pos], input[pos + 1]);
        auto it = hash_table.find(h);
        if (it == hash_table.end()) return std::nullopt;

        const auto &cands = it->second;
        size_t best_len = 0;
        size_t best_win_index = 0;

        size_t start_idx = (cands.size() > 16) ? (cands.size() - 16) : 0;
        for (size_t i = start_idx; i < cands.size(); ++i) {
            const auto &entry = cands[i];
            size_t win_idx = entry.first;
            size_t abs_idx = entry.second;

            if (absolute_pos - abs_idx > Size) continue;

            size_t m = 0;
            size_t w = win_idx;
            while (m < max_len && buffer[w] == input[pos + m]) {
                ++m;
                w = (w + 1) & (Size - 1);
            }

            if (m > best_len) {
                best_len = m;
                best_win_index = win_idx;
                if (best_len == max_len) break;
            }
        }

        if (best_len >= 3) {
            return LookupResult{best_win_index & CHUNK_SIZE_MASK, best_len};
        }
        return std::nullopt;
    }

    void clear() {
        buffer.fill(T{});
        index = 0;
        absolute_pos = 0;
        hash_table.clear();
    }

private:
    std::array<T, Size> buffer{};
    size_t index{};
    size_t absolute_pos{};
    std::unordered_map<uint16_t, std::vector<std::pair<size_t, size_t>>> hash_table;

    static uint16_t hash_pair(T a, T b) { return (uint16_t(a) << 8) | uint16_t(b); }
};

// Entry point

int main(int argc, char* argv[]) {
    try {
        if (argc <= 1)
            throw std::runtime_error(std::string(argv[0]) + " <input_file> [<output_file>]");

        std::string input_path = argv[1];
        std::string output_path = (argc == 3) ? argv[2] : input_path + ".compr";

        const auto input_data = filesystem::file_read(input_path);
        if (input_data.empty()) throw std::runtime_error("Empty input file");

        auto start_time = std::chrono::high_resolution_clock::now();

        std::vector<uint8_t> output;
        output.reserve(input_data.size() + input_data.size() / 8 + 16);

        SlidingDict<uint8_t, CHUNK_SIZE> dict;
        size_t input_pos = 0;
        size_t window_bytes_since_flush = 0;

        auto flush_chunk = [&]() { window_bytes_since_flush = 0; };

        while (input_pos < input_data.size()) {
            uint8_t bits = 0;
            std::vector<uint8_t> payload;
            payload.reserve(32);

            for (uint8_t bit_i = 0; bit_i < 8 && input_pos < input_data.size(); ++bit_i) {
                auto match = dict.look_up(input_data, input_pos);
                if (match) {
                    size_t len = std::min(match->size, MAX_MATCH);
                    uint16_t off = static_cast<uint16_t>(match->offset & CHUNK_SIZE_MASK);
                    uint16_t token = static_cast<uint16_t>(((len - 3) << 13) | off);

                    payload.push_back(static_cast<uint8_t>(token & 0xFF));
                    payload.push_back(static_cast<uint8_t>((token >> 8) & 0xFF));

                    for (size_t k = 0; k < len; ++k) {
                        dict.push(input_data[input_pos + k]);
                    }

                    input_pos += len;
                    window_bytes_since_flush += len;
                } else {
                    uint8_t literal = input_data[input_pos++];
                    payload.push_back(literal);
                    dict.push(literal);
                    bits |= static_cast<uint8_t>(1u << bit_i);
                    window_bytes_since_flush += 1;
                }
            }

            output.push_back(bits);
            output.insert(output.end(), payload.begin(), payload.end());

            if (window_bytes_since_flush >= CHUNK_SIZE) flush_chunk();
        }

        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
        std::cout << "Compressed in: " << duration_ms << "ms\n";

        filesystem::file_write(output_path, output);
    } catch (const std::runtime_error& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
