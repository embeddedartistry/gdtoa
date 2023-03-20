project('gdtoa',
	['c', 'cpp'],
	default_options : [
		'warning_level=3',
		'werror=false',
		# This project defaults to a release build
		'debug=false',
		'optimization=2',
		# `build.*` options affect `native: true targets`
		# plain options affect `native: false` targets.
		'c_std=c11', 'build.c_std=c11',
		'cpp_std=c++17', 'build.cpp_std=c++17',
	],
	meson_version: '>=0.54.0',
	version: '1.2'
)

################################
# Project Options and Settings #
################################

# Pick up our common compiler variables + desired_*_flags variables
subdir('meson/compiler')
subdir('meson/compiler/c')
subdir('meson/compiler/cpp')

if get_option('debug')
	add_project_arguments('-DDEBUG',
		language: ['c', 'cpp'])
	add_project_arguments('-DDEBUG',
		language: ['c', 'cpp'], native: true)
endif

compile_settings_list = [
	{'lang': 'c', 'compiler': host_c_compiler, 'flags': desired_c_compile_flags, 'isnative': false},
	{'lang': 'c', 'compiler': native_c_compiler, 'flags': desired_native_c_compile_flags, 'isnative': true},
	{'lang': 'cpp', 'compiler': host_cpp_compiler, 'flags': desired_cpp_compile_flags, 'isnative': false},
	{'lang': 'cpp', 'compiler': native_cpp_compiler, 'flags': desired_native_cpp_compile_flags, 'isnative': true},
]

link_settings_list = [
	{'lang': 'c', 'compiler': host_c_compiler, 'flags': [], 'isnative': false},
	{'lang': 'c', 'compiler': native_c_compiler, 'flags': [], 'isnative': true},
	{'lang': 'cpp', 'compiler': host_cpp_compiler, 'flags': [], 'isnative': false},
	{'lang': 'cpp', 'compiler': native_cpp_compiler, 'flags': [], 'isnative': true},
]

# Process the compilation flags
subdir('meson/compiler/check-and-apply-flags')

#################
# Build Modules #
#################

# Include reusable build modules here via subdir() calls
subdir('meson/linker/linker-map')
subdir('meson/objcopy')

#######################
# Process Source Tree #
#######################

# Add files to this variable if you want them analyzed by clang-tidy
clangtidy_files = []
# Add dependencies to this array for testing with Catch2
catch2_tests_dep = []

subdir('src')
subdir('test')

###################
# Tooling Modules #
###################

#subdir('meson/analysis/clang-tidy')
#subdir('meson/analysis/complexity')
#subdir('meson/analysis/cppcheck')
#subdir('meson/analysis/sloccount')
#subdir('meson/analysis/vale')
subdir('meson/format')
#subdir('meson/docs/doxygen')
