#include "file_magic_utils.hpp"
#include "fs.hpp"
#include "types.hpp"
#include "wac.hpp"

#include <algorithm>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <cstring>

constexpr bool DEBUG_MODE = false;

int main(int argc, char* argv[]) {
    try {
        if (argc < 2)
            throw std::runtime_error(std::string(argv[0]) + " <input_file> [<output_dir>]");

        const std::filesystem::path wac_path{ argv[1] };
        std::string wal_path_str{ argv[1] };
        wal_path_str.back() = 'L';

        std::filesystem::path output_path;
        if (argc < 3)
            output_path = wac_path.parent_path() / "extracted";
        else
            output_path = argv[2];
        std::filesystem::create_directory(output_path);

        FILE* wac_fp = nullptr;
        #if defined(_WIN32)
            errno_t err;
            err = fopen_s(&wac_fp, wac_path.string().c_str(), "rb");
            if (err != 0 || wac_fp == nullptr)
                throw std::runtime_error("Failed to open: " + wac_path.string());
        #else
            wac_fp = fopen(wac_path.string().c_str(), "rb");
            if (wac_fp == NULL)
                throw std::runtime_error("Failed to open: " + wac_path.string());
        #endif

        const auto wac_entries = parse_wac(wac_fp);

        FILE* wal_fp = nullptr;
        #if defined(_WIN32)
            err = fopen_s(&wal_fp, wal_path_str.c_str(), "rb");
            if (err != 0 || wal_fp == nullptr)
                throw std::runtime_error("wal_fp == NULL");
        #else
            wal_fp = fopen(wal_path_str.c_str(), "rb");
            if (wal_fp == NULL)
                throw std::runtime_error("wal_fp == NULL");
        #endif

        Buffer file_data;
        for (const auto& entry : wac_entries) {
            file_data.resize(entry.size);
            #ifdef __linux__
            fseeko64(wal_fp, entry.offset * SECTOR_SIZE, SEEK_SET);
            #endif
            #ifdef _WIN64
            _fseeki64(wal_fp, entry.offset * SECTOR_SIZE, SEEK_SET);
            #endif
            fread(file_data.data(), entry.size, 1, wal_fp);

            const auto extension = get_file_extension(file_data, (char)entry.type);
            const auto out_path = output_path / (entry.name + extension);

            FILE* out_fp = nullptr;
            #if defined(_WIN32)
                err = fopen_s(&out_fp, out_path.string().c_str(), "wb");
                if (err != 0 || out_fp == nullptr)
                    throw std::runtime_error("out_fp == NULL");
            #else
                out_fp = fopen(out_path.string().c_str(), "wb");
                if (out_fp == NULL)
                    throw std::runtime_error("out_fp == NULL");
            #endif

            fwrite((const char*)file_data.data(), entry.size, 1, out_fp);

            if (DEBUG_MODE)
                printf("Wrote %s offset 0x%08zX size 0x%08zX\n",
                       out_path.string().c_str(),
                       entry.offset * SECTOR_SIZE, file_data.size());

            fclose(out_fp);
        }

        fclose(wal_fp);
        fclose(wac_fp);
    } catch (const std::runtime_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        char errbuf[256];
        #if defined(_WIN32)
            if (!strerror_s(errbuf, sizeof(errbuf), errno))
                std::cerr << "errno: " << errbuf << '\n';
        #else
            if (!strerror_r(errno, errbuf, sizeof(errbuf)))
                std::cerr << "errno: " << errbuf << '\n';
        #endif
        else
            std::cerr << "errno: (unknown error)\n";
        
        return 1;
    }

    return 0;
}
