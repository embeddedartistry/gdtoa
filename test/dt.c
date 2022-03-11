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
#include <stdlib.h>

#ifdef __cplusplus
extern "C" char* dtoa(double, int, int, int*, int*, char**);
#else
extern char* dtoa(double, int, int, int*, int*, char**);
#endif

#define length(x) (sizeof(x) / sizeof *(x))

#define DEFAULT_DTOA_MODE 0
#define DEFAULT_DTOA_NUM_DIGITS 17

static struct
{
	const char* s;
	int dtoa_mode;
	int dtoa_digits;
	double f;
	int sign;
	int dcept;
	int num_digits;
	const char* digits;
	int expected_errno;
} test_values[] = {
	// Default Mode:NumDigits tests (0:17)
	{"1.23", DEFAULT_DTOA_MODE, DEFAULT_DTOA_NUM_DIGITS, 1.23, 0, 1, 3, "123", 0},
	{"1.23e+20", DEFAULT_DTOA_MODE, DEFAULT_DTOA_NUM_DIGITS, 123000000000000000000.0, 0, 21, 3, "123", 0},
	{"1.23e-20", DEFAULT_DTOA_MODE, DEFAULT_DTOA_NUM_DIGITS, 1.23e-20, 0, -19, 3, "123", 0},
	{"1.23456789", DEFAULT_DTOA_MODE, DEFAULT_DTOA_NUM_DIGITS, 1.23456789, 0, 1, 9, "123456789", 0},
	{"1.23456589e+20", DEFAULT_DTOA_MODE, DEFAULT_DTOA_NUM_DIGITS, 1.23456589e+20, 0, 21, 9, "123456589", 0},
	{"1.23e+30", DEFAULT_DTOA_MODE, DEFAULT_DTOA_NUM_DIGITS, 1.23e+30, 0, 31, 3, "123", 0},
	{"1.23e-30", DEFAULT_DTOA_MODE, DEFAULT_DTOA_NUM_DIGITS, 1.23e-30, 0, -29, 3, "123", 0},
	{"1.23456789e-20", DEFAULT_DTOA_MODE, DEFAULT_DTOA_NUM_DIGITS, 1.23456789e-20, 0, -19, 9, "123456789", 0},
	{"1.23456789e-30", DEFAULT_DTOA_MODE, DEFAULT_DTOA_NUM_DIGITS, 1.23456789e-30, 0, -29, 9, "123456789", 0},
	{"1.234567890123456789", DEFAULT_DTOA_MODE, DEFAULT_DTOA_NUM_DIGITS, 1.234567890123456789, 0, 1, 17, "12345678901234567", 0},
	{"1.23456789012345678901234567890123456789", DEFAULT_DTOA_MODE, DEFAULT_DTOA_NUM_DIGITS, 1.23456789012345678901234567890123456789, 0, 1, 17, "12345678901234567", 0},
	{"1.23e306", DEFAULT_DTOA_MODE, DEFAULT_DTOA_NUM_DIGITS, 1.23e306, 0, 307, 3, "123", 0},
	{"1.23e-306", DEFAULT_DTOA_MODE, DEFAULT_DTOA_NUM_DIGITS, 1.23e-306, 0, -305, 3, "123", 0},
	{"1.23e-320", DEFAULT_DTOA_MODE, DEFAULT_DTOA_NUM_DIGITS, 1.23e-320, 0, -319, 3, "123", 0},
	{"1.23e-20", DEFAULT_DTOA_MODE, DEFAULT_DTOA_NUM_DIGITS, 1.23e-20, 0, -19, 3, "123", 0},
	{"1.23456789e307", DEFAULT_DTOA_MODE, DEFAULT_DTOA_NUM_DIGITS, 1.23456789e307, 0, 308, 9, "123456789", 0},
	{"1.23456589e-307", DEFAULT_DTOA_MODE, DEFAULT_DTOA_NUM_DIGITS, 1.23456589e-307, 0, -306, 9, "123456789", 0},
	{"1.234567890123456789", DEFAULT_DTOA_MODE, DEFAULT_DTOA_NUM_DIGITS, 1.234567890123456789, 0, 1, 17, "12345678901234567", 0},
	{"1.234567890123456789e301", DEFAULT_DTOA_MODE, DEFAULT_DTOA_NUM_DIGITS, 1.234567890123456789e301, 0, 302, 17, "12345678901234568", 0},
	{"1.234567890123456789e-301", DEFAULT_DTOA_MODE, DEFAULT_DTOA_NUM_DIGITS, 1.234567890123456789e-301, 0, -300, 17, "12345678901234568", 0},
	{"1.234567890123456789e-321", DEFAULT_DTOA_MODE, DEFAULT_DTOA_NUM_DIGITS, 1.234567890123456789e-321, 0, -320, 17, "12345678901234568", 0},
	{"1e23", DEFAULT_DTOA_MODE, DEFAULT_DTOA_NUM_DIGITS, 1e23, 0, 24, 1, "1", 0},
	{"1e310", DEFAULT_DTOA_MODE, DEFAULT_DTOA_NUM_DIGITS, __builtin_inf(), 0, 9999, 8, "Infinity", ERANGE},
	{"9.0259718793241475e-277", DEFAULT_DTOA_MODE, DEFAULT_DTOA_NUM_DIGITS, 9.0259718793241475e-277, 0, -276, 16, "9025971879324148", 0},
	{"9.025971879324147880346310405869e-277", DEFAULT_DTOA_MODE, DEFAULT_DTOA_NUM_DIGITS, 9.025971879324147880346310405869e-277, 0, -276, 16, "9025971879324148", 0},
	{"9.025971879324147880346310405868e-277", DEFAULT_DTOA_MODE, DEFAULT_DTOA_NUM_DIGITS, 9.025971879324147880346310405868e-277, 0, -276, 16, "9025971879324148", 0},
	{"2.2250738585072014e-308", DEFAULT_DTOA_MODE, DEFAULT_DTOA_NUM_DIGITS, 2.2250738585072014e-308, 0, -307, 17, "22250738585072014", 0},
	{"2.2250738585072013e-308", DEFAULT_DTOA_MODE, DEFAULT_DTOA_NUM_DIGITS, 2.2250738585072013e-308, 0, -307, 17, "22250738585072014", 0},
	// Adjusted Mode:NumDigits tests

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
};

static void dt_test(void** state)
{
	(void)state;
	char* unused;
	int dcept = 0, sign = 0;
	char *s = NULL, *se = NULL;

	for(size_t i = 0; i < length(test_values); i++)
	{
		errno = 0;
		printf("Running entry %zu\n", i);
		double strtod_conv = strtod(test_values[i].s, &unused);
		assert_true(errno == test_values[i].expected_errno);

		s = dtoa(strtod_conv, test_values[i].dtoa_mode, test_values[i].dtoa_digits, &dcept, &sign, &se);
		int64_t conv_int, expected_int;
		memcpy(&conv_int, &strtod_conv, sizeof(double));
		memcpy(&expected_int, &test_values[i].f, sizeof(double));
		printf("sign: %d, dcept: %d, num_digits: %ld, digits: %s\n", sign, dcept, se - s, s);
		printf("EXPECTED sign: %d, dcept: %d, num_digits: %d, digits: %s\n", test_values[i].sign, test_values[i].dcept, test_values[i].num_digits, test_values[i].digits);
		printf("i: %zu, strtod: 0x%llx, expected: 0x%llx, distance: %lld, errno: %d (%s)\n", i, conv_int, expected_int, ulpsDistanceDouble(strtod_conv, test_values[i].f), errno, errno == 0? "OK" : strerror(errno));

		assert_true(sign == test_values[i].sign);
		assert_true(dcept == test_values[i].dcept);
		assert_true((se - s) == test_values[i].num_digits);
		assert_true(0 == strcmp(s, test_values[i].digits));
		assert_true(ulpsDistanceDouble(strtod_conv, test_values[i].f) < 2);
	}

}



int dt_tests()
{
	const struct CMUnitTest dt_tests[] = {
		cmocka_unit_test(dt_test),
	};

	return cmocka_run_group_tests(dt_tests, NULL, NULL);
}

