#pragma once

#include <boost/unordered/unordered_flat_map.hpp>
#include <parallel_hashmap/phmap.h>

#include <atomic>
#include <cstddef>
#include <memory>
#include <unordered_map>
#include <variant>

#include <count_words/cw_api.hpp>

namespace cw {

struct StringHash {
    using is_transparent = void;
    [[nodiscard]] size_t operator()(const char* txt) const
    {
        return std::hash<std::string_view>{}(txt);
    }
    [[nodiscard]] size_t operator()(std::string_view txt) const
    {
        return std::hash<std::string_view>{}(txt);
    }
    [[nodiscard]] size_t operator()(const std::string& txt) const
    {
        return std::hash<std::string>{}(txt);
    }
};

using OptionValueType = std::variant<int, std::string>;
using AlgorithmOptions = std::unordered_map<std::string, OptionValueType>;

using WordsCountMap =
    phmap::parallel_flat_hash_map<std::string,
                                  std::uint64_t,
                                  StringHash,
                                  std::equal_to<>,
                                  std::allocator<std::pair<const std::string, std::uint64_t>>,
                                  7,
                                  std::mutex>;

using WordsViewCountMap = boost::unordered_flat_map<std::string_view, std::uint64_t>;
using WordCleanFunc = std::function<std::vector<std::string>(std::string_view)>;

struct CW_API CountWordsStats {
    std::atomic<std::int64_t> bytes_read{};
    std::atomic<std::size_t> num_files_processed{};
    std::atomic<std::size_t> average_files_size_bytes{};
};

struct CW_API WordsCountResult {
    WordsCountMap words_map{};
    std::unique_ptr<CountWordsStats> stats = std::make_unique<CountWordsStats>();
};

} // namespace cw
