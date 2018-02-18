project(
  'piecewise'
, 'cpp'
, default_options: [
    'cpp_std=c++14'
  , 'werror=true'
  , 'warning_level=3'
  ]
)

catch = subproject('catch')

cpp_args = []
cpp_link_args = []

compiler = meson.get_compiler('cpp')
if compiler.get_id() == 'gcc'
  cpp_args = [
    '-fmax-errors=3'
  ]
elif compiler.get_id() == 'clang'
  cpp_args = [
    '-ferror-limit=3'
  ]
endif

if get_option('libcxx')
  cpp_args += ['-stdlib=libc++']
  cpp_link_args += ['-stdlib=libc++', '-lc++abi']
endif

incdir = include_directories(
  'include'
)

test_src = [
  'test/main.cpp'
, 'test/basic_aggregate.cpp'
, 'test/multifail.cpp'
, 'test/tuple_list.cpp'
]

tests = executable(
  'tests'
, test_src
, cpp_args: cpp_args
, link_args: cpp_link_args
, include_directories: incdir
, dependencies: catch.get_variable('catch')
)
test('all tests', tests)

install_subdir('include', install_dir: 'include')

piecewise = declare_dependency(include_directories : incdir)
