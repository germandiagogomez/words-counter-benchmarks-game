#include <timing/profile.hpp>

#include <fmt/format.h>

namespace cw {

ProfileTimeStats
calculate_profile_stats(double split_words_elapsed_sec,
                        double count_words_elapsed_sec,
                        double merge_results_elapsed_sec,
                        double split_words_pool_size,
                        double count_words_pool_size,
                        double merge_results_pool_size)
{
    auto total_time = split_words_elapsed_sec / split_words_pool_size +
                      count_words_elapsed_sec / count_words_pool_size +
                      merge_results_elapsed_sec / merge_results_pool_size;

    return {.total_time = total_time,
            .split_words_elapsed_sec = split_words_elapsed_sec,
            .count_words_elapsed_sec = count_words_elapsed_sec,
            .merge_results_elapsed_sec = merge_results_elapsed_sec,
            .normalized_split_words = split_words_elapsed_sec / split_words_pool_size / total_time,
            .normalized_count_words = count_words_elapsed_sec / count_words_pool_size / total_time,
            .normalized_merge_results = merge_results_elapsed_sec / merge_results_pool_size /
                                        total_time,
            .split_words_pool_size = split_words_pool_size,
            .count_words_pool_size = count_words_pool_size,
            .merge_results_pool_size = merge_results_pool_size};
}

void
print_profile_stats(const ProfileTimeStats& pt)
{
    fmt::print("Count words threads: {}. Split words threads: {}. Merge "
               "results threads: {}\n\n",
               pt.count_words_pool_size,
               pt.split_words_pool_size,
               pt.merge_results_pool_size);
    fmt::print("\tSplit words/nthreads: "
               "({}s (normalized: {})\n\tCount words/nthreads: {}s "
               "(normalized: {})\n\tMerge "
               "results/nthreads: {}s (normalized: {})\n\n",
               pt.split_words_elapsed_sec / pt.split_words_pool_size,
               pt.normalized_split_words,
               pt.count_words_elapsed_sec / pt.count_words_pool_size,
               pt.normalized_count_words,
               pt.merge_results_elapsed_sec / pt.merge_results_pool_size,
               pt.normalized_merge_results);
}

} // namespace cw
