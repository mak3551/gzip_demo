#include "gzipdata.h"
#include <cstdint>
#include <new>

GzipData::GzipData() : decompressor(std::nullopt), read_completed_flag(false) {}

GzipData::~GzipData() {
    if (decompressor.has_value()) {
        libdeflate_free_decompressor(decompressor.value());
    }
}

GzipData::ErrorCode GzipData::readFile(const std::string& filename) {
    read_completed_flag = false;
    std::ifstream gzip_file(filename, std::ios::binary);
    if (!gzip_file) {
        return ErrorCode::FILE_OPEN_ERROR;
    }
    
    uint64_t filesize = std::filesystem::file_size(filename);
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
    uint64_t compressed_data_size = filesize - headersize - 8;
    try{
        compressed_data.resize(compressed_data_size);
    } catch (std::bad_alloc& bad_alloc_err) {
        std::cerr << bad_alloc_err.what() << "\n";
        return GzipData::ErrorCode::MEMORY_ALLOCATION_FAILED;
    }
    if (!readBytes(gzip_file, compressed_data.data(), compressed_data_size)) {
        return ErrorCode::READ_ERROR;
    }
    if (!readBytes(gzip_file, crc32.data(), 4) || !readBytes(gzip_file, isize.data(), 4)) {
        return ErrorCode::READ_ERROR;
    }
    gzip_file.close();
    read_completed_flag = true;
    return ErrorCode::SUCCESS;
}
GzipData::ErrorCode GzipData::decompress(std::vector<std::byte>& decompressed_data) {
    if(!read_completed_flag){
        return ErrorCode::READ_UNCOMPLETE;
    }
    decompressed_data.clear();
    uint32_t original_size = 0;
    if (isize.size() == 4) {
        original_size = *reinterpret_cast<uint32_t*>(isize.data());
    }
    try{
        decompressed_data.resize(original_size);
    } catch (std::bad_alloc& bad_alloc_err) {
        std::cerr << bad_alloc_err.what() << "\n";
        return GzipData::ErrorCode::MEMORY_ALLOCATION_FAILED;
    }
    initialize_decompressor();
    enum libdeflate_result result = libdeflate_deflate_decompress(decompressor.value(),compressed_data.data(),compressed_data.size(),
        decompressed_data.data(),original_size,NULL);
    if(result == LIBDEFLATE_SUCCESS){
        return ErrorCode::SUCCESS;
    }else{
        return ErrorCode::DECOMPRESS_ERROR;
    }
}
void GzipData::initialize_decompressor() {
    if(!decompressor.has_value()){
        decompressor = libdeflate_alloc_decompressor();
    }
}

void GzipData::free_compressed_data(){
    compressed_data.clear();
    compressed_data.shrink_to_fit();
}

bool GzipData::readByte(std::ifstream& stream, std::byte& byte) {
    stream.read(reinterpret_cast<char*>(&byte), 1);
    return stream.good();
}

bool GzipData::readBytes(std::ifstream& stream, std::byte* bytes, std::size_t size) {
    stream.read(reinterpret_cast<char*>(bytes), size);
    return stream.good();
}

bool GzipData::read_completed(){
    return read_completed_flag;
}

