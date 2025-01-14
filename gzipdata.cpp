#include "gzipdata.h"

GzipData::GzipData() : decompressor(std::nullopt) {}

GzipData::~GzipData() {
    if (decompressor.has_value()) {
        libdeflate_free_decompressor(decompressor.value());
    }
}

GzipData::ErrorCode GzipData::readFile(const std::string& filename) {
    std::ifstream gzip_file(filename, std::ios::binary);
    if (!gzip_file) {
        return ErrorCode::FILE_OPEN_ERROR;
    }
    
    long filesize = std::filesystem::file_size(filename);
    int headersize = HEADER_SIZE_BASIC;

    if (!readByte(gzip_file, id1)){
        return ErrorCode::READ_ERROR;
    } 
    if(id1 != GZIP_ID1) {
        return ErrorCode::INVALID_ID;
    }
    if (!readByte(gzip_file, id2)){
        return ErrorCode::READ_ERROR;
    }
    if(id2 != GZIP_ID2) {
        return ErrorCode::INVALID_ID;
    }

    if (!readByte(gzip_file, cm) || !readByte(gzip_file, flg)) {
        return ErrorCode::READ_ERROR;
    }
    if (!readBytes(gzip_file, mtime.data(), 4)) {
        return ErrorCode::READ_ERROR;
    }
    if (!readByte(gzip_file, xfl) || !readByte(gzip_file, os)) {
        return ErrorCode::READ_ERROR;
    }
    if ((flg & FLG_FEXTRA) != std::byte{0}) {
        return ErrorCode::UNSUPPORTED_EXTRA_FIELD;
    }
    if ((flg & FLG_FNAME) != std::byte{0}) {
        std::byte buff;
        if (!readByte(gzip_file, buff)) {
            return ErrorCode::READ_ERROR;
        }
        headersize += 1;
        while (buff != std::byte{0}) {
            fname.push_back(buff);
            if (!readByte(gzip_file, buff)) {
                return ErrorCode::READ_ERROR;
            }
            headersize += 1;
        }
    }
    if ((flg & FLG_FHCRC) != std::byte{0}) {
        if (!readBytes(gzip_file, crc16.data(), 2)) {
            return ErrorCode::READ_ERROR;
        }
        headersize += 2;
    }
    long compressed_data_size = filesize - headersize - 8;
    compressed_data.resize(compressed_data_size);
    if (!readBytes(gzip_file, compressed_data.data(), compressed_data_size)) {
        return ErrorCode::READ_ERROR;
    }
    if (!readBytes(gzip_file, crc32.data(), 4) || !readBytes(gzip_file, isize.data(), 4)) {
        return ErrorCode::READ_ERROR;
    }
    gzip_file.close();
    return ErrorCode::SUCCESS;
}
bool GzipData::decompress(std::vector<std::byte>& decompressed_data) {
    decompressed_data.clear();
    uint32_t original_size = 0;
    if (isize.size() == 4) {
        original_size = *reinterpret_cast<uint32_t*>(isize.data());
    }
    decompressed_data.resize(original_size);
    initialize_decompressor();
    enum libdeflate_result result = libdeflate_deflate_decompress(decompressor.value(),compressed_data.data(),compressed_data.size(),
    decompressed_data.data(),original_size,NULL);
    if(result == LIBDEFLATE_SUCCESS){
        return true;
    }else{
        return false;
    }
}
void GzipData::initialize_decompressor() {
    if(!decompressor.has_value()){
        decompressor = libdeflate_alloc_decompressor();
    }
}

bool GzipData::readByte(std::ifstream& stream, std::byte& byte) {
    stream.read(reinterpret_cast<char*>(&byte), 1);
    return stream.good();
}

bool GzipData::readBytes(std::ifstream& stream, std::byte* bytes, std::size_t size) {
    stream.read(reinterpret_cast<char*>(bytes), size);
    return stream.good();
}

