#include "fs.hpp"

#include <cstring>
#include <fstream>
#include <iostream>
#include <future>
#include <array>
#include <semaphore>

constexpr size_t CHUNK_SIZE = 0x2000;
constexpr size_t CHUNK_SIZE_MASK = CHUNK_SIZE - 1;
std::filesystem::path decrypted_folder;

std::vector<std::filesystem::path> find_files_ending_with_W(const std::filesystem::path& folder_path) {
    std::vector<std::filesystem::path> files;
    for (const auto& entry : std::filesystem::directory_iterator(folder_path)) {
        if (entry.is_regular_file() && entry.path().filename().string().ends_with("W"))
            files.push_back(entry.path());
    }
    return files;
}

int decompress_file(const std::filesystem::path& file) {
    const auto input_data = filesystem::file_read(file);
    const size_t input_size = input_data.size();
    if (input_data.empty()) {
        std::cerr << "Empty input file: " << file << "\n";
        return 1;
    }

    std::vector<uint8_t> output_data;
    output_data.reserve(input_size * 4);

    std::array<uint8_t, CHUNK_SIZE> window{};
    size_t win_pos = 0;
    size_t in_pos = 0;

    auto flush_window = [&](size_t count) {
        output_data.insert(output_data.end(), window.begin(), window.begin() + count);
    };

    while (in_pos < input_size) {
        uint8_t bits = input_data[in_pos++];
        for (int i = 0; i < 8 && in_pos < input_size; ++i) {
            if (bits & 1) {
                uint8_t literal = input_data[in_pos++];
                window[win_pos] = literal;
                ++win_pos;
                if (win_pos == CHUNK_SIZE) { flush_window(CHUNK_SIZE); win_pos = 0; }
            } else { // match
                uint16_t lo = input_data[in_pos++];
                uint16_t hi = input_data[in_pos++];
                uint16_t token = lo | (hi << 8);
                int16_t len = (token >> 13) + 3;
                int16_t off = token & CHUNK_SIZE_MASK;
                while (len-- > 0) {
                    window[win_pos] = window[off];
                    ++win_pos;
                    off = (off + 1) & CHUNK_SIZE_MASK;
                    if (win_pos == CHUNK_SIZE) { flush_window(CHUNK_SIZE); win_pos = 0; }
                }
            }
            bits >>= 1;
        }
    }

    if (win_pos > 0) flush_window(win_pos);

    std::filesystem::path out_path = decrypted_folder / file.filename();
    out_path += ".dec";
    std::ofstream out(out_path, std::ios::binary);
    if (!out) {
        std::cerr << "Failed to open output file: " << out_path << "\n";
        return 1;
    }
    out.write(reinterpret_cast<const char*>(output_data.data()), static_cast<std::streamsize>(output_data.size()));
    return 0;
}

int main(int argc, const char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <folder_path>\n";
        return 1;
    }

    std::filesystem::path folder_path = argv[1];
    if (!std::filesystem::exists(folder_path) || !std::filesystem::is_directory(folder_path)) {
        std::cerr << "Invalid folder path: " << folder_path << "\n";
        return 1;
    }

    auto files = find_files_ending_with_W(folder_path);
    if (files.empty()) {
        std::cout << "No files ending with 'W' found in folder.\n";
        return 0;
    }
    
    unsigned int max_threads = std::max(1u, std::thread::hardware_concurrency());
    std::counting_semaphore<> sem(max_threads); // limits concurrent tasks
    
    decrypted_folder = folder_path / "decrypted";
    std::filesystem::create_directories(decrypted_folder);
    
    auto start2 = std::chrono::high_resolution_clock::now();
    std::vector<std::future<void>> tasks;
    for (const auto& file : files) {
        sem.acquire();
        tasks.emplace_back(std::async(std::launch::async, [file, &sem]() {
            decompress_file(file);
            sem.release();
        }));
    }
    
    for (auto& task : tasks) task.get();
    auto end2 = std::chrono::high_resolution_clock::now();
    auto dur2 = std::chrono::duration_cast<std::chrono::milliseconds>(end2-start2).count();
    std::cout << "All files decompressed in: " << dur2 << "ms\n";
    return 0;
}
