catch2_dep = dependency('catch2-with-main')
fmt_dep = dependency('fmt')

count_words_bench = executable('bench_count_words',
                               sources: ['bench_count_words.cpp'],
                              dependencies: [catch2_dep, fmt_dep, count_words_dep])

benchmark('Count words benchmarks', count_words_bench,
          env: {'COUNT_WORDS_RESOURCES_DIR': meson.source_root() / 'resources',
                'COUNT_WORDS_NUM_FILES': '64'})
