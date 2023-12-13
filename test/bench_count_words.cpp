#define CATCH_CONFIG_ENABLE_BENCHMARKING
#include <catch2/catch_all.hpp>

#include <types/dynamic_buffer.hpp>
#include <count_words/count_words.hpp>
#include <string/immutable_string.hpp>

#include <absl/hash/hash.h>
#include <boost/smart_ptr/local_shared_ptr.hpp>
#include <boost/smart_ptr/make_local_shared.hpp>
#include <fmt/format.h>

#include <cstdlib>
#include <filesystem>
#include <random>

#include <limits>
#include <ranges>
#include <string>
#include <tuple>
#include <type_traits>

//using namespace cw;
//namespace fs = std::filesystem;
//namespace cwo =cw;
//namespace rvc = ::std::ranges::views;
//namespace rc = ::std::ranges;
//
//template <class T>
//T getenv_or_default(std::string_view var_name, T default_val = {}) {
//  using ValT = std::remove_cvref_t<T>;
//  if (auto env = std::getenv(var_name.data()); env != nullptr) {
//    if constexpr (std::is_integral_v<ValT>) {
//      return std::remove_cvref_t<T>{std::stoi(env)};
//    } else {
//      return ValT(env);
//    }
//  }
//  return default_val;
//}
//
//static auto [DIR_RES, NUM_FILES] = std::tuple(
//    getenv_or_default<fs::path>("COUNT_WORDS_RESOURCES_DIR", "resources/"),
//    getenv_or_default<int>("COUNT_WORDS_NUM_FILES", 0));
//
//TEST_CASE("Count words benchmarks", "[.][full][!benchmark]") {
//  BENCHMARK("Monothread implementation") {
//    auto files_range =
//        rc::subrange(fs::directory_iterator{DIR_RES},
//                     fs::directory_iterator{}) |
//        (NUM_FILES
//             ? rvc::take(NUM_FILES)
//             : rvc::take(std::numeric_limits<decltype(NUM_FILES)>::max()));
//
//    WordsCountMap result;
//    for (const auto &file : files_range)
//      cwo::combine_maps(result, cwo::count_all_words_in(file));
//    return result;
//  };
//  BENCHMARK("std::async simple implementation") {
//    auto files_range =
//        rc::subrange(fs::directory_iterator{DIR_RES},
//                     fs::directory_iterator{}) |
//        (NUM_FILES
//             ? rvc::take(NUM_FILES)
//             : rvc::take(std::numeric_limits<decltype(NUM_FILES)>::max()));
//
//    return cwo::multithread_async_count_words(files_range);
//  };
//  BENCHMARK("multithread semaphore-limited implementation") {
//    auto files_range =
//        rc::subrange(fs::directory_iterator{DIR_RES},
//                     fs::directory_iterator{}) |
//        (NUM_FILES
//             ? rvc::take(NUM_FILES)
//             : rvc::take(std::numeric_limits<decltype(NUM_FILES)>::max()));
//
//    return cwo::multithread_limited_count_words(files_range);
//  };
//  BENCHMARK("boost::async + boost::future::then") {
//    auto files_range =
//        rc::subrange(fs::directory_iterator{DIR_RES},
//                     fs::directory_iterator{}) |
//        (NUM_FILES
//             ? rvc::take(NUM_FILES)
//             : rvc::take(std::numeric_limits<decltype(NUM_FILES)>::max()));
//    return cwo::simple_boost_future(files_range);
//  };
//  BENCHMARK("boost::executors::thread_pool + boost::future::then") {
//    auto files_range =
//        rc::subrange(fs::directory_iterator{DIR_RES},
//                     fs::directory_iterator{}) |
//        (NUM_FILES
//             ? rvc::take(NUM_FILES)
//             : rvc::take(std::numeric_limits<decltype(NUM_FILES)>::max()));
//    return cwo::executor_based_boost_future(files_range);
//  };
//  BENCHMARK("mem_mapped_file zero copy + better load balancing") {
//    auto files_range =
//        rc::subrange(fs::directory_iterator{DIR_RES},
//                     fs::directory_iterator{}) |
//        (NUM_FILES
//             ? rvc::take(NUM_FILES)
//             : rvc::take(std::numeric_limits<decltype(NUM_FILES)>::max()));
//    return cwo::boost_future_mem_mapped_files_string_view(files_range);
//  };
//}
//
//TEST_CASE("Pipeline operations benchmark basic",
//          "[basic][pipeline][!benchmark]") {
//  using namespace cw;
//  BENCHMARK_ADVANCED("reading filenames")
//  (Catch::Benchmark::Chronometer meter) {
//    auto filenames = read_filenames(DIR_RES);
//    meter.measure([filenames = std::move(filenames)] {
//      return read_words(filenames.front());
//    });
//  };
//  BENCHMARK_ADVANCED("reading files (memory mapped)")
//  (Catch::Benchmark::Chronometer meter) {
//    auto filenames = read_filenames(DIR_RES);
//    meter.measure([&] { return read_file(filenames.front()); });
//  };
//
//  BENCHMARK_ADVANCED("splitting in words when reading file")
//  (Catch::Benchmark::Chronometer meter) {
//    auto filenames = read_filenames(DIR_RES);
//    meter.measure([&] { return read_words(filenames.front()); });
//  };
//
//  BENCHMARK_ADVANCED("splitting in words from DynamicBuffer")
//  (Catch::Benchmark::Chronometer meter) {
//    auto filenames = read_filenames(DIR_RES);
//    auto buf = read_file(filenames.front());
//    meter.measure([&] { return split_in_words(buf); });
//  };
//
//  BENCHMARK_ADVANCED("counting words")
//  (Catch::Benchmark::Chronometer meter) {
//    auto filenames = read_filenames(DIR_RES);
//    auto words = read_words(filenames.front());
//    meter.measure([&] { return count_words(words); });
//  };
//}
//
//std::vector<std::byte> random_numbers(std::size_t bytes) {
//  static std::random_device rd;
//  static std::mt19937 gen(rd());
//  static std::uniform_int_distribution<char> dist;
//  std::vector<std::byte> v(bytes);
//  std::generate_n(v.begin(), bytes,
//                  [&] { return static_cast<std::byte>(dist(gen)); });
//  return v;
//}
//
//#define REPEAT2(x) x x
//#define REPEAT4(x) REPEAT2(x) REPEAT2(x)
//#define REPEAT8(x) REPEAT4(x) REPEAT4(x)
//#define REPEAT16(x) REPEAT8(x) REPEAT8(x)
//#define REPEAT(x) REPEAT16(x) REPEAT16(x)
//
//TEST_CASE("Memory speed", "[.][basic][memory][!benchmark]") {
//  BENCHMARK_ADVANCED("Write 8 bytes at a time")
//  (Catch::Benchmark::Chronometer meter) {
//    auto MEMORY_SIZE = 1024ull * 1024 * 64;
//    auto mem_to_read = random_numbers(MEMORY_SIZE);
//    const auto beg_mem = reinterpret_cast<std::uint64_t *>(mem_to_read.data());
//    const auto end_mem = reinterpret_cast<std::uint64_t *>(mem_to_read.data() +
//                                                           mem_to_read.size());
//    std::uint64_t sink{};
//    meter.measure([&] {
//      for (const volatile std::uint64_t *curr = beg_mem; curr < end_mem;) {
//        REPEAT(sink = *curr++;)
//      }
//      return sink;
//    });
//  };
//}
