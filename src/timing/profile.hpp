#pragma once

#include <count_words/cw_api.hpp>
#include <cw_config.hpp>
#include <timing/scoped_timer.hpp>

#if ENABLE_INTERNAL_PROFILING

#define PROFILE_SCOPE_MOVE(f) cw::ScopedTimer timer(std::move(f))
#define PROFILE_SCOPE(f) cw::ScopedTimer timer(f)
#define PROFILE_VAR(type, name) type name
#define PROFILE_SCOPE_FINISH() timer.force_finish();

#else

#define PROFILE_SCOPE_MOVE(f)
#define PROFILE_SCOPE(f)
#define PROFILE_VAR(type, name)
#define PROFILE_SCOPE_FINISH()

#endif

namespace cw {

struct CW_API ProfileTimeStats {
    double total_time{};
    double split_words_elapsed_sec{};
    double count_words_elapsed_sec{};
    double merge_results_elapsed_sec{};
    double normalized_split_words{};
    double normalized_count_words{};
    double normalized_merge_results{};
    double split_words_pool_size{};
    double count_words_pool_size{};
    double merge_results_pool_size{};
};

CW_API ProfileTimeStats
calculate_profile_stats(double split_words_elapsed_sec,
                        double count_words_elapsed_sec,
                        double merge_results_elapsed_sec,
                        double split_words_pool_size,
                        double count_words_pool_size,
                        double merge_results_pool_size);

CW_API void
print_profile_stats(const ProfileTimeStats& pt);

} // namespace cw
