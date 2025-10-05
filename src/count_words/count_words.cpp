#include <count_words/count_words.hpp>

#include <cw_config.hpp>

#include <algo_stages/count_words.hpp>
#include <algo_stages/read_input.hpp>
#include <algo_stages/split_words.hpp>
#include <types/cw_types.hpp>

#include <timing/profile.hpp>
#include <timing/scoped_timer.hpp>
#include <types/dynamic_buffer.hpp>

#define BOOST_THREAD_VERSION 4
#define BOOST_THREAD_PROVIDES_EXECUTORS
#include <boost/thread/executors/basic_thread_pool.hpp>
#include <boost/thread/future.hpp>

#include <boost/container/vector.hpp>

#if USE_ASYNC_FILE_IO
#include <boost/asio/any_io_executor.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/executor.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/stream_file.hpp>
#include <boost/asio/this_coro.hpp>
#endif

#include <fmt/core.h>
#include <fmt/format.h>

#include <algorithm>
#include <fstream>
#include <future>
#include <iostream>
#include <semaphore>
#include <thread>

namespace cw {

// IO from stream, split in words
std::vector<std::string> read_words(const std::filesystem::path &p,
                                    CountWordsStats &stats, int read_buf_size) {
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
  std::copy(std::make_move_iterator(strIt), std::make_move_iterator(eof),
            std::back_inserter(result));
  return result;
}

WordsCountMap count_words(std::span<std::string const> seq) noexcept {
  return detail::count_words_impl<WordsCountMap>(seq);
}

WordsViewCountMap count_words(std::span<std::string_view const> seq) noexcept {
  return detail::count_words_impl<WordsViewCountMap>(seq);
}

WordsCountMap count_all_words_in(const std::filesystem::path &file,
                                 CountWordsStats &stats,
                                 AlgorithmOptions const &opts) {
  int read_buf_size = opts.contains("read-buffer-size")
                          ? std::get<int>(opts.at("read-buffer-size"))
                          : 4096;
  auto all_words = read_words(file, stats, read_buf_size);
  stats.bytes_read += read_buf_size;
  auto words_count = count_words(all_words);
  stats.num_files_processed += 1;
  return words_count;
}

WordsCountResult
multithread_async_count_words(std::span<std::filesystem::path const> rng,
                              AlgorithmOptions const &opts) {
  using namespace std;
  vector<future<WordsCountMap>> tasks;
  WordsCountMap words_count;
  auto stats = std::make_unique<CountWordsStats>();
  for (const auto &file : rng) {
    tasks.push_back(std::async([=, &stats]() mutable {
      return count_all_words_in(file, *stats, opts);
    }));
  }
  for (auto &fut : tasks) {
    auto result_map = fut.get();
    combine_maps(words_count, result_map);
  }
  return WordsCountResult{std::move(words_count), std::move(stats)};
}

WordsCountResult
multithread_limited_count_words(std::span<std::filesystem::path const> rng,
                                AlgorithmOptions const &opts) {
  using namespace std;
  WordsCountMap words_count;
  std::counting_semaphore sem(std::thread::hardware_concurrency());
  vector<future<WordsCountMap>> tasks;
  auto stats = std::make_unique<CountWordsStats>();
  for (const auto &file : rng) {
    sem.acquire();
    tasks.push_back(std::async(std::launch::async, [=, &sem, &stats]() mutable {
      auto result = count_all_words_in(file, *stats, opts);
      sem.release();
      return result;
    }));
  }
  for (auto &fut : tasks) {
    auto result_map = fut.get();
    combine_maps(words_count, result_map);
  }
  return WordsCountResult{std::move(words_count), std::move(stats)};
}

WordsCountResult simple_boost_future(std::span<std::filesystem::path const> rng,
                                     AlgorithmOptions const &opts) {
  using namespace std;
  WordsCountMap words_count;
  int read_buf_size = opts.contains("read-buffer-size")
                          ? std::get<int>(opts.at("read-buffer-size"))
                          : 4096;
  std::mutex words_count_mut;
  vector<boost::future<void>> tasks;
  auto stats = std::make_unique<CountWordsStats>();
  for (const auto &file : rng) {
    tasks.push_back(boost::async([=, &stats] {
                      auto words = read_words(file, *stats, read_buf_size);
                      return count_words(words);
                    })
                        .then([&words_count, &words_count_mut](
                                  boost::future<WordsCountMap> words) {
                          auto task_words = words.get();
                          std::lock_guard l(words_count_mut);
                          for (const auto &[word, times] : task_words)
                            words_count[word] += times;
                        }));
  }
  boost::when_all(tasks.begin(), tasks.end()).wait();
  return WordsCountResult{words_count, std::move(stats)};
}

WordsCountResult
executor_based_boost_future(std::span<std::filesystem::path const> rng,
                            AlgorithmOptions const &opts) {
  using namespace std;
  WordsCountMap words_count;
  int read_buf_size = opts.contains("read-buffer-size")
                          ? std::get<int>(opts.at("read-buffer-size"))
                          : 4096;
  std::mutex words_count_mut;
  vector<boost::future<void>> tasks;
  boost::executors::basic_thread_pool pool;
  auto stats = std::make_unique<CountWordsStats>();
  for (const auto &file : rng) {
    tasks.push_back(boost::async(pool,
                                 [=, &stats] {
                                   auto words =
                                       read_words(file, *stats, read_buf_size);
                                   stats->num_files_processed.fetch_add(1);
                                   return count_words(words);
                                 })
                        .then(pool, [&words_count, &words_count_mut](
                                        boost::future<WordsCountMap> words) {
                          auto task_words = words.get();
                          std::lock_guard l(words_count_mut);
                          for (const auto &[word, times] : task_words)
                            words_count[word] += times;
                        }));
  }
  boost::when_all(tasks.begin(), tasks.end()).wait();
  return WordsCountResult{std::move(words_count), std::move(stats)};
}

WordsCountResult
threadpool_with_coroutine(std::span<std::filesystem::path const> rng,
                          AlgorithmOptions const &opts) {
#if !USE_ASYNC_FILE_IO
  throw std::runtime_error("Coroutine-based words count implementation not "
                           "available in your system");
#else
  using namespace std;
  using boost::asio::use_awaitable;
  WordsCountMap words_count;
  boost::executors::basic_thread_pool pool(std::thread::hardware_concurrency());
  boost::asio::io_context ctx;
  std::atomic<std::int64_t> total_files_size{0};
  std::atomic<std::int64_t> num_files_processed{0};

  counting_semaphore<128> sem(128);
  std::vector<boost::future<void>> tasks;
  auto count_words_task =
      [&](std::filesystem::path p) -> boost::asio::awaitable<void> {
    boost::asio::any_io_executor executor =
        co_await boost::asio::this_coro::executor;
    auto file_contents = co_await read_file_async(executor, p);
    tasks.push_back(boost::async(pool, [&, file_contents = std::move(
                                               file_contents)]() mutable {
      auto beg = reinterpret_cast<const char *>(file_contents.data());
      auto words = split_in_words(std::span{beg, file_contents.size()});
      auto words_map = count_words(words);
      auto buf_size = file_contents.size();
      {
        std::for_each(words_map.begin(), words_map.end(), [&](const auto &kv) {
          words_count[kv.first] += kv.second;
        });
      }
      sem.release();
      total_files_size.fetch_add(buf_size);
      num_files_processed.fetch_add(1);
    }));
  };
  std::binary_semaphore sem_start(0);
  boost::async(boost::launch::async, [&] {
    auto work = boost::asio::make_work_guard(ctx);
    for (int i = 0; const auto &file : rng) {
      sem.acquire();
      boost::asio::co_spawn(ctx, count_words_task(file), boost::asio::detached);
      if (!i)
        sem_start.release();
      ++i;
    }
  });
  sem_start.acquire();
  ctx.run();

  WordsCountResult result;
  boost::when_all(tasks.begin(), tasks.end()).wait();
  result.words_map = std::move(words_count);
  result.stats->bytes_read = total_files_size.load();

  result.stats->num_files_processed.store(num_files_processed.load());
  return result;
#endif
}

WordsCountResult boost_future_mem_mapped_files_string_view(
    std::span<std::filesystem::path const> filepaths_rng,
    AlgorithmOptions const &opts) {
  using namespace std;
  PROFILE_VAR(alignas(64) std::atomic<double>, split_words_elapsed_sec{});
  PROFILE_VAR(alignas(64) std::atomic<double>, count_words_elapsed_sec{});
  PROFILE_VAR(alignas(64) std::atomic<double>, merge_results_elapsed_sec{});
  PROFILE_VAR(alignas(64) std::atomic<double>, outer_elapsed_sec{});
  auto prof_func = [&](std::string_view tag) {
    return [&, tag](std::chrono::duration<double> difftime) {
#if ENABLE_INTERNAL_PROFILING
      if (tag == "split_words")
        split_words_elapsed_sec.fetch_add(difftime.count());
      else if (tag == "count_words")
        count_words_elapsed_sec.fetch_add(difftime.count());
      else if (tag == "merge_results")
        merge_results_elapsed_sec.fetch_add(difftime.count());
      else if (tag == "outer") {
        outer_elapsed_sec.fetch_add(difftime.count());
      }
#endif
    };
  };

  unsigned merge_results_pool_size = std::thread::hardware_concurrency() / 2;
  unsigned split_words_pool_size = std::thread::hardware_concurrency() / 2;

  unsigned count_words_pool_size = std::thread::hardware_concurrency() / 2;

  if (opts.contains("merge_results_pool_size"))
    merge_results_pool_size = std::get<int>(opts.at("merge_results_pool_size"));
  if (opts.contains("split_words_pool_size"))
    split_words_pool_size = std::get<int>(opts.at("split_words_pool_size"));
  if (opts.contains("count_words_pool_size"))
    count_words_pool_size = std::get<int>(opts.at("count_words_pool_size"));

  PROFILE_SCOPE(prof_func("outer"));
  WordsCountMap words_count;
  vector<boost::future<void>> tasks;
  boost::executors::basic_thread_pool default_pool(merge_results_pool_size);
  boost::executors::basic_thread_pool split_words_pool(split_words_pool_size);
  boost::executors::basic_thread_pool count_words_pool(count_words_pool_size);

  std::atomic<std::size_t> total_files_size{0};
  std::atomic<std::size_t> total_files_processed{0};

  constexpr int files_batch_size = 64;
  std::vector<std::filesystem::path> files_batch;
  files_batch.reserve(files_batch_size);

  using DynamicBufferVec = std::vector<FastDynamicBuffer>;
  using DynamicBufferVecPtr = std::unique_ptr<DynamicBufferVec>;

  for (const auto &file : filepaths_rng) {
    files_batch.push_back(file);
    if (files_batch.size() % files_batch_size == 0) {
      tasks.push_back(
          boost::async(default_pool,
                       [&prof_func, files_batch = std::move(files_batch)]() {
                         auto buffers = std::make_unique<DynamicBufferVec>();
                         try {
                           for (const auto &f : files_batch)
                             buffers->push_back(read_file_zero_copy(f));
                         } catch (std::exception const &e) {
                           std::cerr << e.what() << std::endl;
                         }
                         return buffers;
                       })
              .then(
                  split_words_pool,
                  [&prof_func](boost::future<DynamicBufferVecPtr> buffers_fut) {
                    try {
                      auto buffers = buffers_fut.get();
                      PROFILE_SCOPE(prof_func("split_words"));
                      std::vector<std::vector<std::string_view>> words_seqs;
                      words_seqs.reserve(files_batch_size);
                      for (const auto &buf : *buffers)
                        words_seqs.emplace_back(split_in_words(buf));
                      return std::pair(std::move(words_seqs),
                                       std::move(buffers));
                    } catch (std::exception const &e) {
                      std::cerr << e.what() << std::endl;
                    }
                    throw std::runtime_error("Unreachable code!");
                  })
              .then(
                  count_words_pool,
                  [&prof_func](
                      boost::future<
                          std::pair<std::vector<std::vector<std::string_view>>,
                                    DynamicBufferVecPtr>>
                          words_fut) mutable {
                    try {
                      PROFILE_SCOPE(prof_func("count_words"));
                      auto words = words_fut.get();
                      auto &[words_seqs, bufs] = words;
                      std::vector<WordsViewCountMap> maps;
                      maps.reserve(files_batch_size);
                      for (const auto &ws : words_seqs)
                        maps.push_back(count_words(ws));
                      return std::pair(std::move(maps), std::move(bufs));
                    } catch (std::exception const &e) {
                      std::cerr << e.what() << std::endl;
                    }
                    throw std::runtime_error("Unreachable code!");
                  })
              .then(default_pool,
                    [&words_count, &total_files_size, &total_files_processed,
                     &prof_func](
                        boost::future<std::pair<std::vector<WordsViewCountMap>,
                                                DynamicBufferVecPtr>>
                            words_maps_fut) {
                      try {
                        PROFILE_SCOPE(prof_func("merge_results"));
                        auto wm = words_maps_fut.get();
                        const auto &[words_maps, bufs] = wm;
                        for (std::size_t j = 0;
                             const auto &task_words : words_maps) {
                          auto bytes_buf = (*bufs).at(j++).size();
                          {
                            std::for_each(
                                task_words.cbegin(), task_words.cend(),
                                [&](const auto &kv) {
                                  words_count[std::string(kv.first)] +=
                                      kv.second;
                                });
                          }
                          total_files_size.fetch_add(bytes_buf);
                        }
                        total_files_processed.fetch_add(words_maps.size());
                      } catch (std::exception const &e) {
                        std::cerr << e.what() << std::endl;
                      }
                    }));
      files_batch = {};
      files_batch.reserve(files_batch_size);
    }
  }
  WordsCountResult result;
  boost::when_all(tasks.begin(), tasks.end()).wait();

  result.words_map = std::move(words_count);
  result.stats->bytes_read = static_cast<std::int64_t>(total_files_size.load());
  result.stats->num_files_processed = total_files_processed.load();
  PROFILE_SCOPE_FINISH()
#if ENABLE_INTERNAL_PROFILING
  auto ps = calculate_profile_stats(
      split_words_elapsed_sec.load(), count_words_elapsed_sec.load(),
      merge_results_elapsed_sec.load(), split_words_pool_size,
      count_words_pool_size, merge_results_pool_size);
  print_profile_stats(ps);
#endif
  return result;
}

} // namespace cw
