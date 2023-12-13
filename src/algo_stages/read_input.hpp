#pragma once

#include <count_words/cw_api.hpp>
#include <types/dynamic_buffer.hpp>

#include <span>
#include <filesystem>
#include <vector>


namespace cw {

// Pure IO into DynamicBuffer
CW_API DynamicBuffer
read_file(const std::filesystem::path& p);

CW_API FastDynamicBuffer
read_file_zero_copy(const std::filesystem::path& p);

CW_API DynamicBuffer
read_files(std::span<std::filesystem::path const> paths);


// IO for reading directory entries
CW_API std::vector<std::filesystem::path>
read_filenames(const std::filesystem::path& direc);

}
