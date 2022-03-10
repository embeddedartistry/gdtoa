###############
# gdtoa tests #
###############

gdtoa_dt = executable('gdtoa_dt',
	'original_programs/dt.c',
	dependencies: gdtoa_temporary_dep,
	native: true,
	build_by_default: meson.is_subproject() == false
)

gdtoa_tests = executable('gdtoa_test',
	sources: [
		'main.c'
	],
	dependencies: [
		cmocka_native_dep,
		gdtoa_temporary_dep
	],
	link_args: [
		native_map_file.format(meson.current_build_dir() + '/gdtoa_test'),
	],
	native: true,
	build_by_default: meson.is_subproject() == false
)

#############################
# Register Tests with Meson #
#############################

test_output_dir = 'CMOCKA_XML_FILE=' + meson.build_root() + '/test/%g.xml'

if meson.is_subproject() == false
	test('gdtoa_test',
		gdtoa_tests,
		env: [
			'CMOCKA_MESSAGE_OUTPUT=XML',
			test_output_dir
		])

	run_target('gdtoa-tests',
		command: [gdtoa_tests]
	)

	run_target('gdtoa-tests-xml',
		command: [
			cmocka_with_env_script,
			gdtoa_tests,
			cmocka_test_output_dir
		],
	)
endif
