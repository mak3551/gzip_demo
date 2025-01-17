#include "gzipdata.h"
#include <iostream>
#include <fstream>
#include <filesystem>

constexpr uint64_t LIMIT_FILE_SIZE = 1610612736;

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " input output" << std::endl;
        return 1;
    }
    uint64_t filesize_of_input;
    try{
        filesize_of_input = std::filesystem::file_size(argv[1]);
    } catch (std::filesystem::filesystem_error& fserror) {
        std::cerr << fserror.what() << std::endl;
        return 5;
    }
    if(filesize_of_input > LIMIT_FILE_SIZE){
        std::cerr << "This program cannot read a file whose size is over 1.5 GB." << std::endl;
        return 6;
    }
    GzipData gzip_data;
    GzipData::ErrorCode status = gzip_data.readFile(std::string(argv[1]));
    if(status != GzipData::ErrorCode::SUCCESS) {
        std::cerr << "Error occurred when reading file '" << argv[1] << "'.\n";
        std::cerr << "Error code: " << static_cast<int>(status) << std::endl;
        return 2;
    }
    std::vector<std::byte> decompressed_data;
    GzipData::ErrorCode decompress_status = gzip_data.decompress(decompressed_data);
    if(decompress_status != GzipData::ErrorCode::SUCCESS) {
        std::cerr << "Error occurred when decompressing data." << std::endl;
        return 3;
    }
    gzip_data.free_compressed_data();
    std::ofstream output_file(argv[2], std::ios::binary);
    if(!output_file) {
        std::cerr << "Error occurred when writing decompressed data to output file." << std::endl;
        return 4;
    }
    output_file.write(reinterpret_cast<char*>(decompressed_data.data()), decompressed_data.size());
    output_file.close();

    return 0;
}