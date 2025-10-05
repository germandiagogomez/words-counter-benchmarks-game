# Detect dependencies
fmt_dep = dependency('fmt', version: '>=9.0', include_type: 'system')

boost_dep = [
  dependency('boost-headers', include_type: 'system'),
  dependency('boost-iostreams', include_type: 'system'), dependency('boost-thread', include_type: 'system')]
cxxopts_dep = dependency('cxxopts', version: '=2.2.1', include_type: 'system')


phmap_dep = dependency('parallel-hashmap', include_type: 'system')

# Configuration header
conf_data = configuration_data()
conf_data.set10('ENABLE_INTERNAL_PROFILING', get_option('enable_internal_profiling'))
conf_data.set('CW_STATIC_LIB_CONFIG', get_option('default_library') == 'static' ? 1 : 0)

#  cpp_extra_args += ['-DASYNC_FILE_IO=1', '-DBOOST_ASIO_DISABLE_EPOLL', '-DBOOST_ASIO_HAS_FILE']
uring_dep = dependency('', required: false)
if host_machine.system() == 'linux'
  uring_dep = dependency('liburing', required: false)
  if uring_dep.found()
    conf_data.set10('USE_ASYNC_FILE_IO', true)
    conf_data.set10('BOOST_ASIO_HAS_IO_URING', true)
  else
    conf_data.set10('USE_ASYNC_FILE_IO', false)
    warning('Liburing not found. Cannot use Boost.Asio coroutine implementation')
  endif
endif

if host_machine.system() == 'linux'

else
  conf_data.set10('USE_ASYNC_FILE_IO', false)
endif



wcr_concurrency_level = get_option('words_count_result_max_concurrency_level')
if wcr_concurrency_level == 0
    cpp_compiler = meson.get_compiler('cpp')
    wcr_concurrency_level = cpp_compiler.run('''
        #include <thread>
        int main() { return std::thread::hardware_concurrency() / 2; }
        ''').returncode()
endif

conf_data.set('WORDS_COUNT_RESULT_MAX_CONCURRENCY_LEVEL', wcr_concurrency_level)
configure_file(input : 'cw_config.hpp.in',
    output: 'cw_config.hpp',
    configuration: conf_data)

cpp_extra_args = ['-fvisibility=hidden']

foundation_dep = []
if host_machine.system() == 'darwin'
  foundation_dep = dependency('appleframeworks', modules: 'foundation')
endif


lib_count_words = library('count_words',
                          sources :
                            ['count_words/count_words.cpp',
                             'timing/profile.cpp',
                             'algo_stages/read_input.cpp'],
                             cpp_args: [cpp_extra_args] + ['-DCW_EXPORTS',
                             '-DCW_STATIC_LIB_CONFIG=@0@'.format(get_option('default_library') == 'static' ? 1 : 0)],
                             dependencies : [fmt_dep, boost_dep, foundation_dep, phmap_dep, uring_dep])


count_words_dep = declare_dependency(link_with: [lib_count_words],
                                     include_directories: '.',
                                     dependencies: [fmt_dep, boost_dep, cxxopts_dep,
                                                    uring_dep, foundation_dep, phmap_dep])
