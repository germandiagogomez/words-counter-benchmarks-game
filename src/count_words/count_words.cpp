#include <count_words/count_words.hpp>

#include <cw_config.hpp>

#include <fmt/core.h>
#include <fmt/format.h>

#include <algorithm>
#include <fstream>

#include "types/cw_types.hpp"

namespace cw {

// IO from stream, split in words
CW_API std::vector<std::string>
read_words(const std::filesystem::path& p, CountWordsStats& stats, int read_buf_size)
{
    using namespace std;
    std::vector<std::string> result;
#if cpp_lib_smart_ptr_for_overwrite
    auto dyn_buf = std::make_unique_for_overwrite<char[]>(read_buf_size);
#else
    auto dyn_buf = std::make_unique<char[]>(read_buf_size);
#endif
    ifstream file_stream;
    if (read_buf_size != 4096) {
        file_stream.rdbuf()->pubsetbuf(std::addressof(dyn_buf[0]), read_buf_size);
    }
    file_stream.open(p.c_str());
    if (!file_stream.is_open())
        throw std::runtime_error("Error opening file: " + p.string());
    file_stream.seekg(0, std::ios::end);
    auto file_stream_size = file_stream.tellg();
    file_stream.seekg(0, std::ios::beg);
    stats.bytes_read += file_stream_size;
    std::istream_iterator<std::string> strIt{file_stream}, eof;
    std::copy(std::make_move_iterator(strIt),
              std::make_move_iterator(eof),
              std::back_inserter(result));
    return result;
}

WordsCountMap
count_words(std::span<std::string const> seq) noexcept
{
    return detail::count_words_impl<WordsCountMap>(seq);
}

WordsViewCountMap
count_words(std::span<std::string_view const> seq) noexcept
{
    return detail::count_words_impl<WordsViewCountMap>(seq);
}

WordsCountMap
count_all_words_in(const std::filesystem::path& file,
                   CountWordsStats& stats,
                   AlgorithmOptions const& opts)
{
    int read_buf_size = opts.contains("read-buffer-size")
                            ? std::get<int>(opts.at("read-buffer-size"))
                            : 4096;
    auto all_words = read_words(file, stats, read_buf_size);
    stats.bytes_read += read_buf_size;
    auto words_count = count_words(all_words);
    stats.num_files_processed += 1;
    return words_count;
}

} // namespace cw
