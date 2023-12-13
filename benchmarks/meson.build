benchmark_dep = dependency('benchmark')

scp_prog = find_program('scp')
uncompress_prog = find_program('xz')
tar_prog = find_program('tar')
script_download_wiki = find_program('scripts/download_wiki.sh')


command_wikipedia = '@0@ @1@ @2@ @3@ @4@ @5@ @6@'.format(
  '@0@/scripts/download_wiki.sh'.format(meson.current_source_dir()),
  scp_prog.full_path(),
  uncompress_prog.full_path(),
  tar_prog.full_path(),
  '.downloaded_wiki_resources',
  '.uncompressed_wiki_resources',
  meson.project_source_root())

message(f'Wikipedia download command is @command_wikipedia@')

download_wiki = custom_target(output: ['.downloaded_wiki_resources',
                                       '.uncompressed_wiki_resources'],
                              command: [script_download_wiki,
                                        scp_prog.full_path(),
                                        uncompress_prog.full_path(),
                                        tar_prog.full_path(),
                                        '@OUTPUT0@',
                                        '@OUTPUT1@',
                                        meson.project_source_root()])


wc_bench = executable('benchmarks', 'benchmarks.cpp',
                      dependencies: [count_words_dep, benchmark_dep])


benchmark('benchmarks', wc_bench, depends: download_wiki,
          env: ['COUNT_WORDS_RESOURCES_DIR=@0@/wikipedia_resources'.format(meson.project_source_root())],
          workdir: meson.project_source_root())
