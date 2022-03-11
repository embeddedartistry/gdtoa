#################
# GDTOA Library #
#################

gdtoa_files = files(
	'dmisc.c',
	'dtoa.c',
	'g__fmt.c',
	'g_ddfmt.c',
	'g_dfmt.c',
	'g_ffmt.c',
	'g_Qfmt.c',
	'g_xfmt.c',
	'g_xLfmt.c',
	'gdtoa.c',
	'gethex.c',
	'gmisc.c',
	'hd_init.c',
	'hexnan.c',
	'misc.c',
	'smisc.c',
	'strtod.c',
	'strtodg.c',
	'strtodI.c',
	'strtof.c',
	'strtoId.c',
	'strtoIdd.c',
	'strtoIf.c',
	'strtoIg.c',
	'strtoIQ.c',
	'strtoIx.c',
	'strtoIxL.c',
	'strtopd.c',
	'strtopdd.c',
	'strtopf.c',
	'strtopQ.c',
	'strtopx.c',
	'strtopxL.c',
	'strtord.c',
	'strtordd.c',
	'strtorf.c',
	'strtorQ.c',
	'strtorx.c',
	'strtorxL.c',
	'sum.c',
	'ulp.c'
)

gdtoa_include = include_directories('../include')
gdtoa_sys_include = include_directories('../include', is_system: true)

gdtoa_compiler_flags = [
	#'-DNO_ERRNO',
	'-DNO_IEEE_Scale',
	'-DIFNAN_CHECK',
	'-DNO_FENV_H',
]

gdtoa_temporary_dep = declare_dependency(
	sources: gdtoa_files,
	include_directories: gdtoa_include,
	compile_args: gdtoa_compiler_flags
)
