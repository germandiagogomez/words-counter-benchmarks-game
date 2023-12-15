#pragma once

#include <count_words/cw_api.hpp>
#include <types/cw_types.hpp>

#include <filesystem>
#include <span>
#include <string>
#include <vector>


namespace cw {

struct CountWordsStats;

// IO from stream, split in words (old and "deprecated")
[[deprecated(
    "Use read_file and split_into_words together to replace this call")]] CW_API std::vector<std::string>
read_words(const std::filesystem::path& p, CountWordsStats& stats, int read_buf_size = 4096);


CW_API WordsCountMap
count_all_words_in(const std::filesystem::path& file,
                   CountWordsStats& stats,
                   AlgorithmOptions const& opts = {});

template<class AssocContainer, class Func = std::plus<>>
void
combine_maps(AssocContainer& map1, const AssocContainer& map2, Func func = {})
{
    for (const auto& [k, v] : map2)
        map1[k] = func(map1[k], v);
}

CW_API WordsCountResult multithread_async_count_words(std::span<std::filesystem::path const> rng,
                              AlgorithmOptions const& opts = {});

CW_API WordsCountResult
multithread_limited_count_words(std::span<std::filesystem::path const> rng,
                                AlgorithmOptions const& opts = {});


CW_API WordsCountResult
simple_boost_future(std::span<std::filesystem::path const> rng, AlgorithmOptions const& opts = {});


CW_API WordsCountResult
executor_based_boost_future(std::span<std::filesystem::path const> rng,
                            AlgorithmOptions const& opts = {});

CW_API WordsCountResult
threadpool_with_coroutine(std::span<std::filesystem::path const> rng, AlgorithmOptions const& opts = {});


CW_API WordsCountResult boost_future_mem_mapped_files_string_view(
    std::span<std::filesystem::path const> filepaths_rng,
    AlgorithmOptions const& opts = {});


} // namespace cw
