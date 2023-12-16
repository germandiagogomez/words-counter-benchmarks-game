if not get_option('download_dataset').allowed()
  warning('In order to run benchmarks, option "download_dataset" must be activated. Benchmarks will not be built unless you activate the option')
elif get_option('download_dataset').allowed()
  wc_bench = executable('benchmarks', 'benchmarks.cpp',
                      dependencies: [count_words_dep, dependency('benchmark')])
  benchmark('benchmarks', wc_bench,
            env: ['COUNT_WORDS_RESOURCES_DIR=@0@/wikipedia_resources'.format(meson.project_source_root())],
            workdir: meson.project_source_root())
endif
