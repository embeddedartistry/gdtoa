/*
 * Copyright © 2022 Embedded Artistry LLC.
 * License: MIT. See LICENSE file for details.
 */

// CMocka needs these
// clang-format off
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <cmocka.h>
// clang-format on

#include <tests.h>

int main(void)
{
	int overall_result = 0;

	overall_result |= dt_tests();

	return overall_result;
}
