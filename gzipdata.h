#ifndef GZIP_DEMO_GZIPDATA_H_
#define GZIP_DEMO_GZIPDATA_H_
#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <vector>
#include <array>
#include <cstddef>
#include <libdeflate.h>

constexpr std::byte GZIP_ID1 = std::byte{0x1f};
constexpr std::byte GZIP_ID2 = std::byte{0x8b};
constexpr std::byte FLG_FEXTRA = std::byte{0x04};
constexpr std::byte FLG_FNAME = std::byte{0x08};
constexpr std::byte FLG_FHCRC = std::byte{0x02};

const int HEADER_SIZE_BASIC = 10;
class GzipData {
    public:
        GzipData();
        ~GzipData();
        enum class ErrorCode {
            SUCCESS = 0,
            FILE_OPEN_ERROR = 1,
            INVALID_ID = 2,
            UNSUPPORTED_EXTRA_FIELD = 3,
            READ_ERROR = 4
        };
        ErrorCode readFile(const std::string& filename);
        bool decompress(std::vector<std::byte>& decompressed_data);
        void initialize_decompressor();
    private:
        bool readByte(std::ifstream& stream, std::byte& byte);
        bool readBytes(std::ifstream& stream, std::byte* bytes, std::size_t size);
        std::byte id1;
        std::byte id2;
        std::byte cm;
        std::byte flg;
        std::array<std::byte, 4> mtime;
        std::byte xfl;
        std::byte os;
        std::array<std::byte, 2> crc16;
        std::vector<std::byte> fname;
        std::vector<std::byte> compressed_data;
        std::array<std::byte, 4> crc32;
        std::array<std::byte, 4> isize;
        struct libdeflate_decompressor *decompressor;
};
#endif