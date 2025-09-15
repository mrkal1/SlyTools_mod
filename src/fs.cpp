#include "fs.hpp"
#include "types.hpp"
#include <fstream>

namespace filesystem {

Buffer file_read(const std::filesystem::path& path, size_t read_size) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open: " + path.string());
    }

    Buffer data;
    if (read_size == 0) {
        auto fs = std::filesystem::file_size(path);
        data.resize(static_cast<size_t>(fs));
        if (!data.empty()) {
            file.read(reinterpret_cast<char*>(data.data()),
                      static_cast<std::streamsize>(data.size()));
        }
    } else {
        data.resize(read_size);
        file.read(reinterpret_cast<char*>(data.data()),
                  static_cast<std::streamsize>(read_size));
        data.resize(static_cast<size_t>(file.gcount()));
    }
    return data;
}

void file_write(const std::filesystem::path& path,
                const Buffer& data, bool trunc) {
    if (!trunc && std::filesystem::exists(path)) {
        printf("Skipping overwriting existing file %s\n",
               path.string().c_str());
        return;
    }

    std::ofstream file(path, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open: " + path.string());
    }

    file.write(reinterpret_cast<const char*>(data.data()),
               static_cast<std::streamsize>(data.size()));
}

} // namespace filesystem
