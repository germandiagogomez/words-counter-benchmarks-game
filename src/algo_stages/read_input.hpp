#pragma once

#include <cw_config.hpp>
#include <count_words/cw_api.hpp>
#include <types/dynamic_buffer.hpp>

#if USE_ASYNC_FILE_IO
#include <boost/container/vector.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/any_io_executor.hpp>
#endif


#include <span>
#include <filesystem>
#include <vector>

#include <cstddef>


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


#if USE_ASYNC_FILE_IO

CW_API boost::asio::awaitable<boost::container::vector<std::byte>>
read_file_async(boost::asio::any_io_executor& exe, const std::filesystem::path& p);

#endif

}
