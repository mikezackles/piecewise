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

compiler = meson.get_compiler('cpp')
if compiler.get_id() == 'gcc'
  args = [
    '-fmax-errors=3'
  ]
elif compiler.get_id() == 'clang'
  args = [
    '-ferror-limit=3'
  ]
else
  args = []
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
, cpp_args: args
, include_directories: incdir
, dependencies: catch.get_variable('catch')
)
test('all tests', tests)

install_subdir('include', install_dir: 'include')

piecewise = declare_dependency(include_directories : incdir)
