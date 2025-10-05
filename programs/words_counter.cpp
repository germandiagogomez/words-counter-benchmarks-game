#include <count_words/count_words.hpp>
#include <implementations/words_counter/words_counter_from_free_function.hpp>
#include <interfaces/words_counter.hpp>
#include <timing/time_it.hpp>

#include <fmt/chrono.h>
#include <fmt/core.h>
#include <fmt/ranges.h>
#include <boost/scope_exit.hpp>
#include <cxxopts.hpp>

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <functional>
#include <iostream>
#include <numeric>
#include <ranges>
#include <string_view>
#include <unordered_map>
#include <vector>
#include "types/cw_types.hpp"

namespace cwo = cw;
namespace rc = ::std::ranges;
namespace fs = std::filesystem;

template<class T>
T
getenv_or_default(std::string_view var_name, T default_val = {})
{
    using ValT = std::remove_cvref_t<T>;
    if (auto env = std::getenv(var_name.data()); env != nullptr) {
        if constexpr (std::is_integral_v<ValT>) {
            return std::remove_cvref_t<T>{std::stoi(env)};
        } else {
            return ValT(env);
        }
    }
    return default_val;
}

class MonoThreadWordsCounter final : public cw::WordsCounter {
   private:
    cw::WordsCountResult do_count_words(std::span<std::filesystem::path const> files_range,
                                        cw::AlgorithmOptions const& opts = {}) const override
    {
        cw::WordsCountMap result;
        auto stats = std::make_unique<cw::CountWordsStats>();
        for (const auto& file : files_range)
            cwo::combine_maps(result, cwo::count_all_words_in(file, *stats, opts));
        return cw::WordsCountResult{result, std::move(stats)};
    }
};

using WordsCountersMap = std::unordered_map<std::string_view, cw::WordsCounterPtr>;

template<class D, class B = cw::WordsCounter>
inline auto words_counter_creator = []() -> std::shared_ptr<B> {
    return std::shared_ptr<B>(new D);
};

using AsyncWordsCounter = cw::WordsCounterFromFreeFunction<cwo::multithread_async_count_words>;
using BoostFutureWordsCounter = cw::WordsCounterFromFreeFunction<cwo::simple_boost_future>;
using BoostFutureMemMappedFileWordsCounter =
    cw::WordsCounterFromFreeFunction<cwo::boost_future_mem_mapped_files_string_view>;
using ExecutorBasedFutureWordsCounter =
    cw::WordsCounterFromFreeFunction<cwo::executor_based_boost_future>;
using ThreadPoolWithCoroutine = cw::WordsCounterFromFreeFunction<cwo::threadpool_with_coroutine>;

#define M_ENTRY(T)                                                                                 \
    {                                                                                              \
        #T, words_counter_creator<T>()                                                             \
    }

struct AlgorithmOptionDesc {
    std::string_view name;
    std::string_view description;

    std::string to_string() const { return fmt::format("{} ({})", name, description); }
};

template<>
struct fmt::formatter<AlgorithmOptionDesc> : fmt::formatter<std::string_view> {
    template<class FormatContext>
    auto format(const AlgorithmOptionDesc& d, FormatContext& fc) const
    {
        std::string formatted = d.to_string();
        return fmt::formatter<std::string_view>::format(std::string_view(formatted), fc);
    }
};

static constexpr AlgorithmOptionDesc kalgorithms_desc[] = {
    {"merge_results_pool_size", "Number of threads in the pool for merge_results stage"},
    {"split_words_pool_size", "Number of threads in the pool for merge_results stage"},
    {"count_words_pool_size", "Number of threads in the pool for merge_results stage"}};

struct CmdLineOptions {
    std::string wc_implementation;
    int num_files{};
    std::string dir_res{};
    bool verbose{};
    std::string out_file{};
    std::vector<std::string> algorithms;
};

template<typename S = std::string>
std::vector<S>
split(std::string_view s, std::string_view delim)
{
    std::vector<S> results;
    unsigned i = 0;
    auto pos = s.find(delim);
    while (pos != S::npos) {
        results.push_back(S(s.substr(i, pos - i)));
        i = ++pos;
        pos = s.find(delim, pos);

        if (pos == S::npos && i != s.length())
            results.push_back(S(s.substr(i, s.length())));
    }

    if (results.empty())
        results.push_back(S(s));
    return results;
}

int
main(int argc, char* argv[])
{
    using namespace std::literals;
    const WordsCountersMap words_counters = {M_ENTRY(MonoThreadWordsCounter),
                                             M_ENTRY(AsyncWordsCounter),
                                             M_ENTRY(ThreadPoolWithCoroutine),
                                             M_ENTRY(BoostFutureMemMappedFileWordsCounter),
                                             M_ENTRY(ExecutorBasedFutureWordsCounter)};
    std::vector<std::string_view> keys;
    keys.reserve(words_counters.size());
    for (const auto& [k, _] : words_counters)
        keys.push_back(k);
    CmdLineOptions options_result;
    cxxopts::Options options(fs::path(argv[0]).filename().string(),
                             "Words counter implementations tester");
    options.add_options()("n,num-files",
                          "Number of files to process by the words counter",
                          cxxopts::value<int>(options_result.num_files)->default_value("0"))(
        "D,textfiles-dir",
        "Directory containing the text files to read",
        cxxopts::value<std::string>(options_result.dir_res)->default_value("resources/"))(
        "v,verbose",
        "Dump results to stdout",
        cxxopts::value<bool>(options_result.verbose)
            ->implicit_value("true")
            ->default_value("false"))(
        "i,impl",
        fmt::format(FMT_STRING("Choose implementation to run from: {}"), fmt::join(keys, ", ")),
        cxxopts::value<std::string>(options_result.wc_implementation)
            ->default_value("MonoThreadWordsCounter"))(
        "o,out-file",
        "File where to dump the results. When only the argument is used without "
        "a value, the name of the implementation + '.out' is used. "
        "If a name is given to the argument, that name is used to save the "
        "output.",
        cxxopts::value<std::string>(options_result.out_file)
            ->implicit_value(".implementation_name")
            ->default_value(""))(
        "a,algorithm-option",
        fmt::format("Algorithm option. An option is written as a key=value. You "
                    "can repeat "
                    "this option many times. Available options: {}",
                    fmt::join(kalgorithms_desc, ", ")),
        cxxopts::value<std::vector<std::string>>(options_result.algorithms))("h,help",
                                                                             "Show this help");

    auto arg_opts = options.parse(argc, argv);
    if (arg_opts.count("help")) {
        fmt::print("{}\n", options.help());
        return 1;
    }

    cw::WordsCounterPtr words_counter = words_counters.at(options_result.wc_implementation);
    const auto algo_opts = [&] {
        cw::AlgorithmOptions algo_options;
        for (const auto& opt : options_result.algorithms) {
            auto key_val = split(opt, "=");
            algo_options[key_val.at(0)] = std::stoi(key_val.at(1));
        }
        return algo_options;
    }();

    std::vector<fs::path> filenames;
    for (int i = 0; const auto& entry : fs::directory_iterator{options_result.dir_res}) {
        filenames.push_back(entry.path());
        ++i;
        if (i >= options_result.num_files)
            break;
    }

    auto [words_result, total_time] = cw::time_it(
        [&] { return words_counter->count_words(filenames, algo_opts); });

    auto& words = words_result.words_map;
    using Val =
        std::pair<std::remove_const_t<cw::WordsCountMap::key_type>, cw::WordsCountMap::mapped_type>;
    std::vector<Val> words_vec(words.size());
    rc::copy(words, words_vec.begin());
    rc::sort(words_vec, [](auto&& a, auto&& b) {
        if (a.second == b.second) [[unlikely]]
            return a.first < b.first;
        return a.second > b.second;
    });
    auto total_words = std::accumulate(
        words_vec.begin(),
        words_vec.end(),
        std::int64_t{0},
        [](std::int64_t acc, auto const& v) { return acc + v.second; });

    auto save_stream = [&]() -> FILE* {
        if (options_result.out_file.empty())
            return nullptr;
        return options_result.out_file == ".implementation_name"
                   ? std::fopen((options_result.wc_implementation + ".out").c_str(), "wb")
                   : std::fopen(options_result.out_file.c_str(), "wb");
    }();
    BOOST_SCOPE_EXIT_ALL(&)
    {
        if (save_stream) {
            std::fclose(save_stream);
        }
    };
    namespace c = std::chrono;
    auto print_results = [&](FILE* out) {
        fmt::print(out,
                   FMT_STRING("MiB read: {:.2f}\nExecution time: {}\nProcessing speed: "
                              "{:.2f} MiB/s\nNumber of files processed: {}\nWords/s: "
                              "{:.1f}\n\n"),
                   words_result.stats->bytes_read / 1024.0 / 1024.0,
                   c::duration_cast<c::duration<double>>(total_time),
                   words_result.stats->bytes_read / 1024.0 / 1024 /
                       c::duration_cast<c::duration<double>>(total_time).count(),
                   words_result.stats->num_files_processed,
                   total_words / c::duration_cast<c::duration<double>>(total_time).count());

        for (const auto& [word, occurrences] : words_vec)
            fmt::print(out, "{:<20} {:>8}\n", word, occurrences);
    };
    if (save_stream)
        print_results(save_stream);
    if (options_result.verbose)
        print_results(stdout);
}
