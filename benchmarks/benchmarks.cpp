#include <benchmark/benchmark.h>

#include <algorithm>
#include <types/dynamic_buffer.hpp>
#include <count_words/count_words.hpp>
#include <random>

#define REPEAT2(x) x x
#define REPEAT4(x) REPEAT2(x) REPEAT2(x)
#define REPEAT8(x) REPEAT4(x) REPEAT4(x)
#define REPEAT16(x) REPEAT8(x) REPEAT8(x)
#define REPEAT(x) REPEAT16(x) REPEAT16(x)

template <class T>
static T getenv_or_default(std::string_view var_name, T default_val = {}) {
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

namespace fs = std::filesystem;

auto [DIR_RES, NUM_FILES] =
  std::tuple(getenv_or_default<fs::path>("COUNT_WORDS_RESOURCES_DIR",
                                           "wikipedia_resources/"),
               getenv_or_default<int>("COUNT_WORDS_NUM_FILES", 16));

static std::vector<std::byte> random_numbers(std::size_t bytes) {
  static std::random_device rd;
  static std::mt19937 gen(rd());
  static std::uniform_int_distribution<std::int8_t> dist;
  std::vector<std::byte> v(bytes);
  std::generate_n(v.begin(), bytes,
                  [&] { return static_cast<std::byte>(dist(gen)); });
  return v;
}

static void BM_read_memory(benchmark::State &state) {
  // Perform setup here
  auto MEMORY_SIZE = state.range(0) * 1024 * 1024;
  auto mem_to_read = random_numbers(MEMORY_SIZE);
  const auto beg_mem = reinterpret_cast<std::uint64_t *>(mem_to_read.data());
  const auto end_mem = reinterpret_cast<std::uint64_t *>(mem_to_read.data() +
                                                         mem_to_read.size());
  std::uint64_t sink{};
  for (auto _ : state) {
    for (const volatile std::uint64_t *curr = beg_mem; curr < end_mem;) {
      REPEAT(sink = *curr++;)
    }
    benchmark::ClobberMemory();
  }
  using C = benchmark::Counter;
  state.counters["Rate"] =
      benchmark::Counter(MEMORY_SIZE, C::kIsIterationInvariantRate, C::kIs1024);
  state.SetLabel(std::to_string(state.range(0)) + " MB");
}

BENCHMARK(BM_read_memory)
    ->Arg(32)
    ->Arg(64)
    ->Arg(128)
    ->Unit(benchmark::kMillisecond);


auto to_char_ptr(std::byte const * b) {
  return reinterpret_cast<char const *>(b);
}


static void BM_split_in_words(benchmark::State &state) {
  // Perform setup here
  const auto NUM_FILES = state.range(0);
  namespace fs = std::filesystem;
  std::vector<std::string> data;
  std::size_t bytes_size = 0;
  for (int i = 0; const auto &file : fs::directory_iterator{DIR_RES}) {
    auto f = cw::read_file_zero_copy(file);
    auto b = to_char_ptr(std::addressof(*f.begin()));
    auto e = to_char_ptr(std::addressof(*f.end()));
    data.push_back(std::string(b, e));
    bytes_size += data.back().size();
    ++i;
    if (i == NUM_FILES)
      break;
  }
  for (auto _ : state) {
    for (std::span<char const> str : data) {
      cw::split_in_words(str);
    }
  }

  using C = benchmark::Counter;
  state.counters["Rate"] =
      benchmark::Counter(bytes_size, C::kIsIterationInvariantRate, C::kIs1024);
  state.SetLabel(
      std::to_string(static_cast<double>(bytes_size) / 1024 / NUM_FILES) +
      " KB average file size ");
}

BENCHMARK(BM_split_in_words)
    ->Unit(benchmark::kMillisecond)
    ->Arg(4)
    ->Arg(16)
    ->Arg(64)
    ->Arg(256);


std::vector<std::size_t> indexes_of_word_boundaries(std::span<char const> buf) {
    auto beg = std::ranges::begin(buf);
    auto end = std::ranges::end(buf);
    std::size_t curr_index = 0;
    std::vector<std::size_t> words_boundaries;
    words_boundaries.reserve(buf.size());
    std::for_each(beg, end, [&](unsigned char c) {
        if (std::isspace(c))
            words_boundaries.push_back(curr_index);
        ++curr_index;
    });
    return words_boundaries;
}

static void BM_indexes_of_word_boundaries(benchmark::State &state) {
    // Perform setup here
    const auto NUM_FILES = state.range(0);
    namespace fs = std::filesystem;
    std::vector<std::string> data;
    std::size_t bytes_size = 0;
    for (int i = 0; const auto &file : fs::directory_iterator{DIR_RES}) {
        auto f = cw::read_file_zero_copy(file);
        auto b = to_char_ptr(std::addressof(*f.begin()));
        auto e = to_char_ptr(std::addressof(*f.end()));
        data.push_back(std::string(b, e));
        bytes_size += data.back().size();
        ++i;
        if (i == NUM_FILES)
            break;
    }
    for (auto _ : state) {
        for (std::span<char const> str : data) {
            indexes_of_word_boundaries(str);
        }
    }

    using C = benchmark::Counter;
    state.counters["Rate"] =
            benchmark::Counter(bytes_size, C::kIsIterationInvariantRate, C::kIs1024);
    state.SetLabel(
            std::to_string(static_cast<double>(bytes_size) / 1024 / NUM_FILES) +
            " KB average file size ");
}

BENCHMARK(BM_indexes_of_word_boundaries)
->Unit(benchmark::kMillisecond)
->Arg(4)
->Arg(16)
->Arg(64)
->Arg(256);

static void BM_read_files(benchmark::State &state) {
  namespace fs = std::filesystem;
  const auto NUM_FILES = state.range(0);
  // Perform setup here
  std::vector<cw::FastDynamicBuffer> dyn_bufs;
  dyn_bufs.reserve(NUM_FILES);
  std::size_t bytes_read = 0;
  std::vector<std::byte> buf(1024 * 1024 * 2);
  for (auto _ : state) {
    for (int i = 0; const auto &f : fs::directory_iterator{DIR_RES}) {
      dyn_bufs.push_back(cw::read_file_zero_copy(f));
      benchmark::DoNotOptimize(std::copy(dyn_bufs.back().begin(),
                                         dyn_bufs.back().end(), buf.begin()));
      ++i;
      if (i == NUM_FILES)
        break;
    }
    state.PauseTiming();
    for (const auto &d : dyn_bufs)
      bytes_read += d.size();
    dyn_bufs.clear();
    dyn_bufs.reserve(NUM_FILES);
    state.ResumeTiming();
  }
  using C = benchmark::Counter;
  state.counters["Rate"] =
      benchmark::Counter(bytes_read / state.iterations(), C::kIsRate, C::kIs1024);
  state.SetLabel(std::to_string(static_cast<double>(bytes_read) / 1024 / 1024 /
                                state.iterations()) +
                 " MB of files read");
}

BENCHMARK(BM_read_files)
    ->Unit(benchmark::kMillisecond)
    ->Arg(32)
    ->Arg(64)
    ->Arg(128);

BENCHMARK_MAIN();
