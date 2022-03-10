// Cmocka needs these
// clang-format off
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <cmocka.h>
// clang-format on

#include <errno.h>
#include "tests.h"
#include "gdtoa.h"
#include "ulpsDistance.h"
#include <stdio.h>
#include <string.h>

#ifdef __cplusplus
extern "C" char* dtoa(double, int, int, int*, int*, char**);
#else
extern char* dtoa(double, int, int, int*, int*, char**);
#endif

#define length(x) (sizeof(x) / sizeof *(x))

static struct
{
	const char* s;
	double f;
} test_values[] = {
	{"1.23", 1.23},
	{"1.23e+20", 123000000000000000000.0},
	{"1.23e-20", 1.23e-20},
	{"1.23456789", 1.23456789},
	{"1.23456589e+20", 1.23456589e+20},
	{"1.23e+30", 1.23e+30},
	{"1.23e-30", 1.23e-30},
	{"1.23456789e-20", 1.23456789e-20},
	{"1.23456789e-30", 1.23456789e-30},
	{"1.234567890123456789", 1.234567890123456789},
	{"1.23456789012345678901234567890123456789", 1.23456789012345678901234567890123456789},
	{"1.23e306", 1.23e306},
	{"1.23e-306", 1.23e-306},
	{"1.23e-320", 1.23e-320},
	{"1.23e-20", 1.23e-20},
	{"1.23456789e307", 1.23456789e307},
	{"1.23456589e-307", 1.23456589e-307},
	{"1.234567890123456789", 1.234567890123456789},
	{"1.234567890123456789e301", 1.234567890123456789e301},
	{"1.234567890123456789e-301", 1.234567890123456789e-301},
	{"1.234567890123456789e-321", 1.234567890123456789e-321},
	{"1e23", 1e23},
	{"1e310", __builtin_inf()},
	{"9.0259718793241475e-277", 9.0259718793241475e-277},
	{"9.025971879324147880346310405869e-277", 9.025971879324147880346310405869e-277},
	{"9.025971879324147880346310405868e-277", 9.025971879324147880346310405868e-277},
	{"2.2250738585072014e-308", 2.2250738585072014e-308},
	{"2.2250738585072013e-308", 2.2250738585072013e-308},
};

#if 0
Sample output
[10:53:16] gdtoa {master} $ cat test/testnos | ./buildresults/test/gdtoa_dt
Input: 1.23
Output: d =
1.23 = 0x3ff3ae14 7ae147ae, se =
	g_fmt gives "1.23"
	dtoa(mode = 0, ndigits = 17):
	dtoa returns sign = 0, decpt = 1, 3 digits:
123
	nextafter(d,+Inf) = 1.2300000000000002 = 0x3ff3ae14 7ae147af:
zsh: done                cat test/testnos |

0 = 0x0 0, se = (lldb) target create "cat"
	g_fmt gives "0"
	dtoa(mode = 0, ndigits = 17):
	dtoa returns sign = 0, decpt = 1, 1 digits:
0
	nextafter(d,+Inf) = 4.9406564584124654e-324 = 0x0 1:
	g_fmt gives "0"
	dtoa returns sign = 0, decpt = 1, 1 digits:
0
sent d = 4.9406564584124654e-324 = 0x0 1, buf = .0e1
got d1 = 0 = 0x0 0
Input: Current executable set to 'cat' (x86_64).
Output: d =
0 = 0x0 0, se = Current executable set to 'cat' (x86_64).
	g_fmt gives "0"
	dtoa(mode = 0, ndigits = 17):
	dtoa returns sign = 0, decpt = 1, 1 digits:
0
	nextafter(d,+Inf) = 4.9406564584124654e-324 = 0x0 1:
	g_fmt gives "0"
	dtoa returns sign = 0, decpt = 1, 1 digits:
0
sent d = 4.9406564584124654e-324 = 0x0 1, buf = .0e1
got d1 = 0 = 0x0 0
Input: (lldb) settings set -- target.run-args  "test/testnos1"
Output: d =
0 = 0x0 0, se = (lldb) settings set -- target.run-args  "test/testnos1"
	g_fmt gives "0"
	dtoa(mode = 0, ndigits = 17):
	dtoa returns sign = 0, decpt = 1, 1 digits:
0
	nextafter(d,+Inf) = 4.9406564584124654e-324 = 0x0 1:
	g_fmt gives "0"
	dtoa returns sign = 0, decpt = 1, 1 digits:
0
#endif

// Defaults: int mode = 0, ndigits = 17;
static void dt_test1_atof(void** state)
{
	(void)state;
	char *s = NULL, *se = NULL;
	int decpt, sign;

	errno = 0;

	for(size_t i = 0; i < length(test_values); i++)
	{
		double atof_conv = atof(test_values[i].s);
		printf("i: %zu, atof: %1.30f, expected: %1.30f, distance: %lld, errno: %d (%s)\n", i, atof_conv, test_values[i].f, ulpsDistanceDouble(atof_conv, test_values[i].f), errno, errno == 0? "OK" : strerror(errno));
		assert_false(errno);
#if 0
		s = dtoa(atof_conv, 0, 17, &decpt, &sign, &se);
		printf("\tdtoa(mode = %d, ndigits = %d):\n", 0, 17);
		printf("\tdtoa returns sign = %d, decpt = %d, %ld digits: %s\n", sign, decpt, se - s, s);
#endif
		assert_true(ulpsDistanceDouble(atof_conv, test_values[i].f) < 2);
		if(s)
		{
			freedtoa(s);
		}
	}

}

static void dt_test1_strtod(void** state)
{
	(void)state;
	char* unused;

	errno = 0;

	for(size_t i = 0; i < length(test_values); i++)
	{
		double strtod_conv = strtod(test_values[i].s, &unused);
		int64_t conv_int, expected_int;
		memcpy(&conv_int, &strtod_conv, sizeof(double));
		memcpy(&expected_int, &test_values[i].f, sizeof(double));
		printf("i: %zu, strtod: 0x%x, expected: 0x%x, distance: %lld, errno: %d (%s)\n", i, conv_int, expected_int, ulpsDistanceDouble(strtod_conv, test_values[i].f), errno, errno == 0? "OK" : strerror(errno));
		assert_false(errno);
		assert_true(ulpsDistanceDouble(strtod_conv, test_values[i].f) < 2);
	}

}

#if 0
Test 2 inputs
// Number:Mode NumDigits
1.23:2 6
1.23:4 6
1.23e+20:2 6
1.23e+20:4 6
1.23e-20:2 6
1.23e-20:4 6
1.23456789:2 6
1.23456789:4 6
1.23456589e+20:2 6
1.23456589e+20:4 6
1.23456789e-20:2 6
1.23456789e-20:4 6
1234565:2 6
1234565:4 6
1.234565:2 6
1.234565:4 6
1.234565e+20:2 6
1.234565e+20:4 6
1.234565e-20:2 6
1.234565e-20:4 6
#endif

int dt_tests()
{
	const struct CMUnitTest dt_tests[] = {
	//	cmocka_unit_test(dt_test1_atof),
		cmocka_unit_test(dt_test1_strtod),
	};

	return cmocka_run_group_tests(dt_tests, NULL, NULL);
}

