benchmark_dep = dependency('benchmark')


if get_option('download_lfs_files')
  git_prog = find_program('git', native: false)
  tar_prog = find_program('tar', native: false)
  message('Download git lfs files (this can take a while)...')
  git_lfs_ret_code = run_command(git_prog, 'lfs', 'pull', check: false)
  if git_lfs_ret_code.returncode() != 0
    error('git lfs pull failed. Please, review that you have git lfs installed and that your network is working properly. You can download git lfs from https://git-lfs.com/')
  endif
  run_command(tar_prog, 'xvjf', 'wikipedia_resources.tar.bz2')
endif


wc_bench = executable('benchmarks', 'benchmarks.cpp',
                      dependencies: [count_words_dep, benchmark_dep])

benchmark('benchmarks', wc_bench,
          env: ['COUNT_WORDS_RESOURCES_DIR=@0@/wikipedia_resources'.format(meson.project_source_root())],
          workdir: meson.project_source_root())
