/****************************************************************

The author of this software is David M. Gay.

Copyright (C) 1998, 1999 by Lucent Technologies
All Rights Reserved

Permission to use, copy, modify, and distribute this software and
its documentation for any purpose and without fee is hereby
granted, provided that the above copyright notice appear in all
copies and that both that the copyright notice and this
permission notice and warranty disclaimer appear in supporting
documentation, and that the name of Lucent or any of its entities
not be used in advertising or publicity pertaining to
distribution of the software without specific, written prior
permission.

LUCENT DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
IN NO EVENT SHALL LUCENT OR ANY OF ITS ENTITIES BE LIABLE FOR ANY
SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF
THIS SOFTWARE.

****************************************************************/

/* Please send bug reports to David M. Gay (dmg at acm dot org,
 * with " at " changed at "@" and " dot " changed to ".").	*/

#include "gdtoaimp.h"

static Bigint* freelist[Kmax + 1];
#ifndef Omit_Private_Memory
	#ifndef PRIVATE_MEM
		#define PRIVATE_MEM 2304
	#endif
	#define PRIVATE_mem ((PRIVATE_MEM + sizeof(double) - 1) / sizeof(double))
static double private_mem[PRIVATE_mem], *pmem_next = private_mem;
#endif

Bigint* Balloc(int k)
{
	Bigint* rv;

	ACQUIRE_DTOA_LOCK(0);

	if((rv = freelist[k]) != 0)
	{
		freelist[k] = rv->next;
	}
	else
	{
		int x = 1 << k;
#ifdef Omit_Private_Memory
		rv = (Bigint*)MALLOC(sizeof(Bigint) + (x - 1) * sizeof(uint32_t));
#else
		size_t len;
		len = (sizeof(Bigint) + ((unsigned)x - 1) * sizeof(uint32_t) + sizeof(double) - 1) /
			  sizeof(double);
		if(((unsigned)(pmem_next - private_mem) + len) <= PRIVATE_mem)
		{
			rv = (Bigint*)pmem_next;
			pmem_next += len;
		}
		else
		{
			rv = (Bigint*)MALLOC(len * sizeof(double));
		}
#endif
		rv->k = k;
		rv->maxwds = x;
	}

	FREE_DTOA_LOCK(0);
	rv->sign = rv->wds = 0;

	return rv;
}

void Bfree(Bigint* v)
{
	if(v)
	{
		ACQUIRE_DTOA_LOCK(0);
		v->next = freelist[v->k];
		freelist[v->k] = v;
		FREE_DTOA_LOCK(0);
	}
}

int lo0bits(uint32_t* y)
{
	int k;
	uint32_t x = *y;

	if(x & 7)
	{
		if(x & 1)
		{
			return 0;
		}

		if(x & 2)
		{
			*y = x >> 1;
			return 1;
		}

		*y = x >> 2;

		return 2;
	}

	k = 0;

	if(!(x & 0xffff))
	{
		k = 16;
		x >>= 16;
	}

	if(!(x & 0xff))
	{
		k += 8;
		x >>= 8;
	}

	if(!(x & 0xf))
	{
		k += 4;
		x >>= 4;
	}

	if(!(x & 0x3))
	{
		k += 2;
		x >>= 2;
	}

	if(!(x & 1))
	{
		k++;
		x >>= 1;

		if(!x)
		{
			return 32;
		}
	}

	*y = x;

	return k;
}

Bigint* multadd(Bigint* b, int m, int a) /* multiply by m and add a */
{
	int i;
	int wds;
#ifndef NO_LONG_LONG
	uint32_t* x;
	uint64_t carry;
	uint64_t y;
#else
	uint32_t carry;
	uint32_t* x;
	uint32_t y;
	#ifdef Pack_32
	uint32_t xi;
	uint32_t z;
	#endif
#endif
	Bigint* b1;

	wds = b->wds;
	x = b->x;
	i = 0;
	carry = (uint64_t)a;

	do
	{
#ifndef NO_LONG_LONG
		y = *x * (uint64_t)m + carry;
		carry = y >> 32;
		*x++ = y & 0xffffffffUL;
#else
	#ifdef Pack_32
		xi = *x;
		y = (xi & 0xffff) * m + carry;
		z = (xi >> 16) * m + (y >> 16);
		carry = z >> 16;
		*x++ = (z << 16) + (y & 0xffff);
	#else
		y = *x * m + carry;
		carry = y >> 16;
		*x++ = y & 0xffff;
	#endif
#endif
	} while(++i < wds);

	if(carry)
	{
		if(wds >= b->maxwds)
		{
			b1 = Balloc(b->k + 1);
			Bcopy(b1, b);
			Bfree(b);
			b = b1;
		}

		b->x[wds++] = (uint32_t)carry;
		b->wds = wds;
	}

	return b;
}

int hi0bits(register uint32_t x)
{
	int k = 0;

	if(!(x & 0xffff0000))
	{
		k = 16;
		x <<= 16;
	}

	if(!(x & 0xff000000))
	{
		k += 8;
		x <<= 8;
	}

	if(!(x & 0xf0000000))
	{
		k += 4;
		x <<= 4;
	}

	if(!(x & 0xc0000000))
	{
		k += 2;
		x <<= 2;
	}

	if(!(x & 0x80000000))
	{
		k++;

		if(!(x & 0x40000000))
		{
			return 32;
		}
	}

	return k;
}

Bigint* i2b(int i)
{
	Bigint* b;

	b = Balloc(1);
	b->x[0] = (uint32_t)i;
	b->wds = 1;

	return b;
}

Bigint* mult(Bigint* a, Bigint* b)
{
	Bigint* c;
	int k;
	int wa;
	int wb;
	int wc;
	uint32_t* x;
	uint32_t* xa;
	uint32_t* xae;
	uint32_t* xb;
	uint32_t* xbe;
	uint32_t* xc;
	uint32_t* xc0;
	uint32_t y;
#ifndef NO_LONG_LONG
	uint64_t carry;
	uint64_t z;
#else
	uint32_t carry;
	uint32_t z;
	#ifdef Pack_32
	uint32_t z2;
	#endif
#endif

	if(a->wds < b->wds)
	{
		c = a;
		a = b;
		b = c;
	}

	k = a->k;
	wa = a->wds;
	wb = b->wds;
	wc = wa + wb;

	if(wc > a->maxwds)
	{
		k++;
	}

	c = Balloc(k);

	for(x = c->x, xa = x + wc; x < xa; x++)
	{
		*x = 0;
	}

	xa = a->x;
	xae = xa + wa;
	xb = b->x;
	xbe = xb + wb;
	xc0 = c->x;

#ifndef NO_LONG_LONG
	for(; xb < xbe; xc0++)
	{
		if((y = *xb++) != 0)
		{
			x = xa;
			xc = xc0;
			carry = 0;

			do
			{
				z = *x++ * (uint64_t)y + *xc + carry;
				carry = z >> 32;
				*xc++ = z & 0xffffffffUL;
			} while(x < xae);

			*xc = (uint32_t)carry;
		}
	}
#else
	#ifdef Pack_32
	for(; xb < xbe; xb++, xc0++)
	{
		if((y = *xb & 0xffff) != 0)
		{
			x = xa;
			xc = xc0;
			carry = 0;

			do
			{
				z = (*x & 0xffff) * y + (*xc & 0xffff) + carry;
				carry = z >> 16;
				z2 = (*x++ >> 16) * y + (*xc >> 16) + carry;
				carry = z2 >> 16;
				Storeinc(xc, z2, z);
			} while(x < xae);

			*xc = carry;
		}

		if((y = *xb >> 16) != 0)
		{
			x = xa;
			xc = xc0;
			carry = 0;
			z2 = *xc;

			do
			{
				z = (*x & 0xffff) * y + (*xc >> 16) + carry;
				carry = z >> 16;
				Storeinc(xc, z, z2);
				z2 = (*x++ >> 16) * y + (*xc & 0xffff) + carry;
				carry = z2 >> 16;
			} while(x < xae);

			*xc = z2;
		}
	}
	#else
	for(; xb < xbe; xc0++)
	{
		if((y = *xb++) != 0)
		{
			x = xa;
			xc = xc0;
			carry = 0;

			do
			{
				z = *x++ * y + *xc + carry;
				carry = z >> 16;
				*xc++ = z & 0xffff;
			} while(x < xae);

			*xc = carry;
		}
	}
	#endif
#endif
	for(xc0 = c->x, xc = xc0 + wc; wc > 0 && !*--xc; --wc)
	{
		;
	}

	c->wds = wc;

	return c;
}

static Bigint* p5s;

Bigint* pow5mult(Bigint* b, int k)
{
	Bigint* b1;
	Bigint* p5;
	Bigint* p51;
	int i;

	if((i = k & 3) != 0)
	{
		static int p05[3] = {5, 25, 125};
		b = multadd(b, p05[i - 1], 0);
	}

	if(!(k >>= 2))
	{
		return b;
	}
	if((p5 = p5s) == 0)
	{
		/* first time */
#ifdef MULTIPLE_THREADS
		ACQUIRE_DTOA_LOCK(1);
		if(!(p5 = p5s))
		{
			p5 = p5s = i2b(625);
			p5->next = 0;
		}
		FREE_DTOA_LOCK(1);
#else
		p5 = p5s = i2b(625);
		p5->next = 0;
#endif
	}
	for(;;)
	{
		if(k & 1)
		{
			b1 = mult(b, p5);
			Bfree(b);
			b = b1;
		}
		if(!(k >>= 1))
		{
			break;
		}
		if((p51 = p5->next) == 0)
		{
#ifdef MULTIPLE_THREADS
			ACQUIRE_DTOA_LOCK(1);
			if(!(p51 = p5->next))
			{
				p51 = p5->next = mult(p5, p5);
				p51->next = 0;
			}
			FREE_DTOA_LOCK(1);
#else
			p51 = p5->next = mult(p5, p5);
			p51->next = 0;
#endif
		}
		p5 = p51;
	}
	return b;
}

Bigint* lshift(Bigint* b, int k)
{
	int i;
	int k1;
	int n;
	int n1;
	Bigint* b1;
	uint32_t* x;
	uint32_t* x1;
	uint32_t* xe;
	uint32_t z;

	n = k >> kshift;
	k1 = b->k;
	n1 = n + b->wds + 1;

	for(i = b->maxwds; n1 > i; i <<= 1)
	{
		k1++;
	}

	b1 = Balloc(k1);
	x1 = b1->x;

	for(i = 0; i < n; i++)
	{
		*x1++ = 0;
	}

	x = b->x;
	xe = x + b->wds;

	if(k &= kmask)
	{
#ifdef Pack_32
		k1 = 32 - k;
		z = 0;

		do
		{
			*x1++ = *x << k | z;
			z = *x++ >> k1;
		} while(x < xe);

		if((*x1 = z) != 0)
		{
			++n1;
		}
#else
		k1 = 16 - k;
		z = 0;

		do
		{
			*x1++ = *x << k & 0xffff | z;
			z = *x++ >> k1;
		} while(x < xe);

		if(*x1 = z)
		{
			++n1;
		}
#endif
	}
	else
	{
		do
		{
			*x1++ = *x++;
		} while(x < xe);
	}

	b1->wds = n1 - 1;
	Bfree(b);

	return b1;
}

int cmp(Bigint* a, Bigint* b)
{
	uint32_t* xa;
	uint32_t* xa0;
	uint32_t* xb;
	uint32_t* xb0;
	int i;
	int j;

	i = a->wds;
	j = b->wds;
#ifdef DEBUG
	if(i > 1 && !a->x[i - 1])
	{
		Bug("cmp called with a->x[a->wds-1] == 0");
	}

	if(j > 1 && !b->x[j - 1])
	{
		Bug("cmp called with b->x[b->wds-1] == 0");
	}
#endif
	if(i -= j)
	{
		return i;
	}

	xa0 = a->x;
	xa = xa0 + j;
	xb0 = b->x;
	xb = xb0 + j;

	for(;;)
	{
		if(*--xa != *--xb)
		{
			return *xa < *xb ? -1 : 1;
		}

		if(xa <= xa0)
		{
			break;
		}
	}

	return 0;
}

Bigint* diff(Bigint* a, Bigint* b)
{
	Bigint* c;
	int i;
	int wa;
	int wb;
	uint32_t* xa;
	uint32_t* xae;
	uint32_t* xb;
	uint32_t* xbe;
	uint32_t* xc;
#ifndef NO_LONG_LONG
	uint64_t borrow;
	uint64_t y;
#else
	uint32_t borrow;
	uint32_t y;
	#ifdef Pack_32
	uint32_t z;
	#endif
#endif

	i = cmp(a, b);

	if(!i)
	{
		c = Balloc(0);
		c->wds = 1;
		c->x[0] = 0;

		return c;
	}

	if(i < 0)
	{
		c = a;
		a = b;
		b = c;
		i = 1;
	}
	else
	{
		i = 0;
	}

	c = Balloc(a->k);
	c->sign = i;
	wa = a->wds;
	xa = a->x;
	xae = xa + wa;
	wb = b->wds;
	xb = b->x;
	xbe = xb + wb;
	xc = c->x;
	borrow = 0;

#ifndef NO_LONG_LONG
	do
	{
		y = (uint64_t)*xa++ - *xb++ - borrow;
		borrow = y >> 32 & 1UL;
		*xc++ = y & 0xffffffffUL;
	} while(xb < xbe);

	while(xa < xae)
	{
		y = *xa++ - borrow;
		borrow = y >> 32 & 1UL;
		*xc++ = y & 0xffffffffUL;
	}
#else
	#ifdef Pack_32
	do
	{
		y = (*xa & 0xffff) - (*xb & 0xffff) - borrow;
		borrow = (y & 0x10000) >> 16;
		z = (*xa++ >> 16) - (*xb++ >> 16) - borrow;
		borrow = (z & 0x10000) >> 16;
		Storeinc(xc, z, y);
	} while(xb < xbe);

	while(xa < xae)
	{
		y = (*xa & 0xffff) - borrow;
		borrow = (y & 0x10000) >> 16;
		z = (*xa++ >> 16) - borrow;
		borrow = (z & 0x10000) >> 16;
		Storeinc(xc, z, y);
	}
	#else
	do
	{
		y = *xa++ - *xb++ - borrow;
		borrow = (y & 0x10000) >> 16;
		*xc++ = y & 0xffff;
	} while(xb < xbe);

	while(xa < xae)
	{
		y = *xa++ - borrow;
		borrow = (y & 0x10000) >> 16;
		*xc++ = y & 0xffff;
	}
	#endif
#endif
	while(!*--xc)
	{
		wa--;
	}

	c->wds = wa;

	return c;
}

double b2d(Bigint* a, int* e)
{
	uint32_t* xa;
	uint32_t* xa0;
	uint32_t w;
	uint32_t y;
	uint32_t z;
	int k;
	double d;
#ifdef VAX
	uint32_t d0;
	uint32_t d1;
#else
	#define d0 word0(d)
	#define d1 word1(d)
#endif

	xa0 = a->x;
	xa = xa0 + a->wds;
	y = *--xa;
#ifdef DEBUG
	if(!y)
	{
		Bug("zero y in b2d");
	}
#endif
	k = hi0bits(y);
	*e = 32 - k;
#ifdef Pack_32
	if(k < Ebits)
	{
		d0 = Exp_1 | y >> (Ebits - k);
		w = xa > xa0 ? *--xa : 0;
		d1 = y << ((32 - Ebits) + k) | w >> (Ebits - k);
		goto ret_d;
	}

	z = xa > xa0 ? *--xa : 0;

	if(k -= Ebits)
	{
		d0 = Exp_1 | y << k | z >> (32 - k);
		y = xa > xa0 ? *--xa : 0;
		d1 = z << k | y >> (32 - k);
	}
	else
	{
		d0 = Exp_1 | y;
		d1 = z;
	}
#else
	if(k < Ebits + 16)
	{
		z = xa > xa0 ? *--xa : 0;
		d0 = Exp_1 | y << k - Ebits | z >> Ebits + 16 - k;
		w = xa > xa0 ? *--xa : 0;
		y = xa > xa0 ? *--xa : 0;
		d1 = z << k + 16 - Ebits | w << k - Ebits | y >> 16 + Ebits - k;
		goto ret_d;
	}

	z = xa > xa0 ? *--xa : 0;
	w = xa > xa0 ? *--xa : 0;
	k -= Ebits + 16;
	d0 = Exp_1 | y << k + 16 | z << k | w >> 16 - k;
	y = xa > xa0 ? *--xa : 0;
	d1 = w << k + 16 | y << k;
#endif

ret_d:
#ifdef VAX
	word0(d) = d0 >> 16 | d0 << 16;
	word1(d) = d1 >> 16 | d1 << 16;
#endif

	return dval(d);
}
#undef d0
#undef d1

Bigint* d2b(double d, int* e, int* bits)
{
	Bigint* b;
#ifndef Sudden_Underflow
	int i;
#endif
	int de;
	int k;
	uint32_t* x;
	uint32_t y;
	uint32_t z;
#ifdef VAX
	uint32_t d0, d1;
	d0 = word0(d) >> 16 | word0(d) << 16;
	d1 = word1(d) >> 16 | word1(d) << 16;
#else
	#define d0 word0(d)
	#define d1 word1(d)
#endif

#ifdef Pack_32
	b = Balloc(1);
#else
	b = Balloc(2);
#endif
	x = b->x;

	z = d0 & Frac_mask;
	d0 &= 0x7fffffff; /* clear sign bit, which we ignore */
#ifdef Sudden_Underflow
	de = (int)(d0 >> Exp_shift);
	#ifndef IBM
	z |= Exp_msk11;
	#endif
#else
	if((de = (int)(d0 >> Exp_shift)) != 0)
	{
		z |= Exp_msk1;
	}
#endif
#ifdef Pack_32
	if((y = d1) != 0)
	{
		if((k = lo0bits(&y)) != 0)
		{
			x[0] = y | z << (32 - k);
			z >>= k;
		}
		else
		{
			x[0] = y;
		}
	#ifndef Sudden_Underflow
		i =
	#endif
			b->wds = (x[1] = z) != 0 ? 2 : 1;
	}
	else
	{
		k = lo0bits(&z);
		x[0] = z;
	#ifndef Sudden_Underflow
		i =
	#endif
			b->wds = 1;
		k += 32;
	}
#else
	if((y = d1) != 0)
	{
		if((k = lo0bits(&y)) != 0)
		{
			if(k >= 16)
			{
				x[0] = y | z << 32 - k & 0xffff;
				x[1] = z >> k - 16 & 0xffff;
				x[2] = z >> k;
				i = 2;
			}
			else
			{
				x[0] = y & 0xffff;
				x[1] = y >> 16 | z << 16 - k & 0xffff;
				x[2] = z >> k & 0xffff;
				x[3] = z >> k + 16;
				i = 3;
			}
		}
		else
		{
			x[0] = y & 0xffff;
			x[1] = y >> 16;
			x[2] = z & 0xffff;
			x[3] = z >> 16;
			i = 3;
		}
	}
	else
	{
	#ifdef DEBUG
		if(!z)
		{
			Bug("Zero passed to d2b");
		}
	#endif
		k = lo0bits(&z);

		if(k >= 16)
		{
			x[0] = z;
			i = 0;
		}
		else
		{
			x[0] = z & 0xffff;
			x[1] = z >> 16;
			i = 1;
		}

		k += 32;
	}

	while(!x[i])
	{
		--i;
	}

	b->wds = i + 1;
#endif
#ifndef Sudden_Underflow
	if(de)
	{
#endif
#ifdef IBM
		*e = (de - Bias - (P - 1) << 2) + k;
		*bits = 4 * P + 8 - k - hi0bits(word0(d) & Frac_mask);
#else
	*e = de - Bias - (P - 1) + k;
	*bits = P - k;
#endif
#ifndef Sudden_Underflow
	}
	else
	{
		*e = de - Bias - (P - 1) + 1 + k;
	#ifdef Pack_32
		*bits = 32 * i - hi0bits(x[i - 1]);
	#else
		*bits = (i + 2) * 16 - hi0bits(x[i]);
	#endif
	}
#endif
	return b;
}
#undef d0
#undef d1

const double
#ifdef IEEE_Arith
	bigtens[] = {1e16, 1e32, 1e64, 1e128, 1e256};
const double tinytens[] = {1e-16, 1e-32, 1e-64, 1e-128, 1e-256};
#else
	#ifdef IBM
	bigtens[] = {1e16, 1e32, 1e64};
const double tinytens[] = {1e-16, 1e-32, 1e-64};
	#else
	bigtens[] = {1e16, 1e32};
const double tinytens[] = {1e-16, 1e-32};
	#endif
#endif

const double tens[] = {1e0,
					   1e1,
					   1e2,
					   1e3,
					   1e4,
					   1e5,
					   1e6,
					   1e7,
					   1e8,
					   1e9,
					   1e10,
					   1e11,
					   1e12,
					   1e13,
					   1e14,
					   1e15,
					   1e16,
					   1e17,
					   1e18,
					   1e19,
					   1e20,
					   1e21,
					   1e22
#ifdef VAX
					   ,
					   1e23,
					   1e24
#endif
};

char* strcp(char* a, const char* b)
{
	while((*a = *b++))
	{
		a++;
	}

	return a;
}

#ifdef NO_STRING_H

void* memcpy(void* a1, void* b1, size_t len)
{
	char *a = (char*)a1, *ae = a + len;
	char *b = (char*)b1, *a0 = a;

	while(a < ae)
	{
		*a++ = *b++;
	}

	return a0;
}

#endif /* NO_STRING_H */
