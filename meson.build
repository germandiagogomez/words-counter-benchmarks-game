project('words_counter', 'cpp',
        version : '0.4',
        default_options : ['warning_level=3', 'cpp_std=c++2a',
                           'buildtype=release',
                           'pkg_config_path=@0@/meson-conan'.format(meson.project_build_root())])

conan_prog = find_program('conan')

conan_profile = '@0@/conan/profiles/@1@'.format(meson.project_source_root(),
                                                get_option('conan_profile'))

SYSTEMS = {'darwin': 'macos', 'linux': 'linux'}

if get_option('conan_profile') == 'autodetect'
  cxx_compiler = meson.get_compiler('cpp',
                                  native: false)
  cxx_compiler_id = cxx_compiler.get_id()
  cxx_compiler_version_major = cxx_compiler.version().split('.')[0]
  conan_profile = '@0@/conan/profiles/@1@@2@_@3@'.format(meson.project_source_root(),
                                                        cxx_compiler_id, cxx_compiler_version_major,
                                                        SYSTEMS[host_machine.system()])
endif



conan_buildtype = get_option('buildtype') == 'release' ? 'Release' : 'Debug'
message(f'Using conan profile: @conan_profile@')
run_command(conan_prog, 'install',
            f'--profile=@conan_profile@',
            meson.source_root(), '-r',
            'conancenter',
            '-s',
            'build_type=@0@'.format(conan_buildtype),
            # This line is added just so that Boost conan stops complaining
            '-s', 'compiler.cppstd=20',
            '--build=missing',
            '-if', meson.build_root() / 'meson-conan', check: true)

run_target('conan',
           command: ['conan', 'install',
            f'--profile=@conan_profile@',
            meson.source_root(), '-r',
            'conancenter',
            '-s',
            'build_type=@0@'.format(conan_buildtype),
            '--build=missing',
            '-if', meson.build_root() / 'meson-conan'])


git_prog = find_program('git', native: false, required: get_option('download_dataset'))

if get_option('download_dataset').allowed() and git_prog.found()
  fs = import('fs')
  git_lfs_ret_code = 0
  if not fs.is_dir('@0@/wikipedia_resources'.format(meson.project_source_root()))
    message('Downloading git lfs files. This can potentially take a while.',
            'If you want to skip downloading lfs files, use -Ddownload_dataset=false on setup.',
            'Please note that in this case benchmarks will not be built.')
    git_lfs_ret_code = run_command(git_prog, 'lfs', 'pull', check: false).returncode()
  endif
  if get_option('download_dataset').allowed() and git_lfs_ret_code != 0
    error('git lfs pull failed. Please, review that you have git lfs installed and that your network is working properly.',
          'You can download git lfs from https://git-lfs.com/')
  endif
  tar_prog = find_program('tar', native: false, required: get_option('download_dataset'))

  if not fs.is_dir('@0@/wikipedia_resources'.format(meson.project_source_root()))
    message('Uncompressing wikipedia_resources.tar.bz2...')
    run_command(tar_prog, 'xvjf', '@0@/wikipedia_resources.tar.bz2'.format(meson.project_source_root()), check: true)
  endif
endif

subdir('src')
subdir('programs')
# Old benchmarks with with Catch2, moved to 'benchmarks' dir with Google benchmark
# subdir('test')
subdir('benchmarks')
