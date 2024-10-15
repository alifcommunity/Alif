#include "alif.h"

#include "AlifCore_DoubleToASCII.h"
#include "AlifCore_State.h"



#include "float.h"


#define MALLOC alifMem_dataAlloc
#define FREE alifMem_dataFree


typedef int32_t Long; // 161
typedef uint64_t ULLong;

union U { double d; ULong L[2]; };

#define word0(x) (x)->L[1]
#define word1(x) (x)->L[0]
#define dval(x) (x)->d

#ifndef STRTOD_DIGLIM
#define STRTOD_DIGLIM 40
#endif

#ifndef MAX_ABS_EXP
#define MAX_ABS_EXP 1100000000U
#endif

#ifndef MAX_DIGITS
#define MAX_DIGITS 1000000000U
#endif

#if MAX_ABS_EXP > INT_MAX
#error "MAX_ABS_EXP should fit in an int"
#endif
#if MAX_DIGITS > INT_MAX
#error "MAX_DIGITS should fit in an int"
#endif

#define Storeinc(a,b,c) (((unsigned short *)a)[1] = (unsigned short)b,  \


#define Exp_shift  20
#define Exp_shift1 20
#define Exp_msk1    0x100000
#define Exp_msk11   0x100000
#define Exp_mask  0x7ff00000
#define P 53
#define Nbits 53
#define Bias 1023
#define Emax 1023
#define Emin (-1022)
#define Etiny (-1074)  /* smallest denormal is 2**Etiny */
#define Exp_1  0x3ff00000
#define Exp_11 0x3ff00000
#define Ebits 11
#define Frac_mask  0xfffff
#define Frac_mask1 0xfffff
#define Ten_pmax 22
#define Bletch 0x10
#define Bndry_mask  0xfffff
#define Bndry_mask1 0xfffff
#define Sign_bit 0x80000000
#define Log2P 1
#define Tiny0 0
#define Tiny1 1
#define Quick_max 14
#define Int_max 14

#ifndef Flt_Rounds
#ifdef FLT_ROUNDS
#define Flt_Rounds FLT_ROUNDS
#else
#define Flt_Rounds 1
#endif
#endif /*Flt_Rounds*/

#define Rounding Flt_Rounds

#define Big0 (Frac_mask1 | Exp_msk1*(DBL_MAX_EXP+Bias-1))
#define Big1 0xffffffff

/* Bits of the representation of positive infinity. */

#define POSINF_WORD0 0x7ff00000
#define POSINF_WORD1 0


struct BCinfo {
	int e0, nd, nd0, scale;
};

#define FFFFFFFF 0xffffffffUL

typedef struct Bigint Bigint;



static Bigint*
Balloc(int k)
{
	int x;
	Bigint* rv;
	unsigned int len;

	x = 1 << k;
	len = (sizeof(Bigint) + (x - 1) * sizeof(ULong) + sizeof(double) - 1)
		/ sizeof(double);

	rv = (Bigint*)MALLOC(len * sizeof(double));
	if (rv == NULL)
		return NULL;

	rv->k = k;
	rv->maxwds = x;
	rv->sign = rv->wds = 0;
	return rv;
}

/* Free a Bigint allocated with Balloc */

static void
Bfree(Bigint* v)
{
	if (v) {
		FREE((void*)v);
	}
}

#define Bcopy(x,y) memcpy((char *)&x->sign, (char *)&y->sign,   \
                          y->wds*sizeof(Long) + 2*sizeof(int))

/* Multiply a Bigint b by m and add a.  Either modifies b in place and returns
   a pointer to the modified b, or Bfrees b and returns a pointer to a copy.
   On failure, return NULL.  In this case, b will have been already freed. */

static Bigint*
multadd(Bigint* b, int m, int a)       /* multiply by m and add a */
{
	int i, wds;
	ULong* x;
	ULLong carry, y;
	Bigint* b1;

	wds = b->wds;
	x = b->x;
	i = 0;
	carry = a;
	do {
		y = *x * (ULLong)m + carry;
		carry = y >> 32;
		*x++ = (ULong)(y & FFFFFFFF);
	} while (++i < wds);
	if (carry) {
		if (wds >= b->maxwds) {
			b1 = Balloc(b->k + 1);
			if (b1 == NULL) {
				Bfree(b);
				return NULL;
			}
			Bcopy(b1, b);
			Bfree(b);
			b = b1;
		}
		b->x[wds++] = (ULong)carry;
		b->wds = wds;
	}
	return b;
}

/* convert a string s containing nd decimal digits (possibly containing a
   decimal separator at position nd0, which is ignored) to a Bigint.  This
   function carries on where the parsing code in _Py_dg_strtod leaves off: on
   entry, y9 contains the result of converting the first 9 digits.  Returns
   NULL on failure. */

static Bigint*
s2b(const char* s, int nd0, int nd, ULong y9)
{
	Bigint* b;
	int i, k;
	Long x, y;

	x = (nd + 8) / 9;
	for (k = 0, y = 1; x > y; y <<= 1, k++);
	b = Balloc(k);
	if (b == NULL)
		return NULL;
	b->x[0] = y9;
	b->wds = 1;

	if (nd <= 9)
		return b;

	s += 9;
	for (i = 9; i < nd0; i++) {
		b = multadd(b, 10, *s++ - '0');
		if (b == NULL)
			return NULL;
	}
	s++;
	for (; i < nd; i++) {
		b = multadd(b, 10, *s++ - '0');
		if (b == NULL)
			return NULL;
	}
	return b;
}

/* count leading 0 bits in the 32-bit integer x. */

static int
hi0bits(ULong x)
{
	int k = 0;

	if (!(x & 0xffff0000)) {
		k = 16;
		x <<= 16;
	}
	if (!(x & 0xff000000)) {
		k += 8;
		x <<= 8;
	}
	if (!(x & 0xf0000000)) {
		k += 4;
		x <<= 4;
	}
	if (!(x & 0xc0000000)) {
		k += 2;
		x <<= 2;
	}
	if (!(x & 0x80000000)) {
		k++;
		if (!(x & 0x40000000))
			return 32;
	}
	return k;
}

/* count trailing 0 bits in the 32-bit integer y, and shift y right by that
   number of bits. */

static int
lo0bits(ULong* y)
{
	int k;
	ULong x = *y;

	if (x & 7) {
		if (x & 1)
			return 0;
		if (x & 2) {
			*y = x >> 1;
			return 1;
		}
		*y = x >> 2;
		return 2;
	}
	k = 0;
	if (!(x & 0xffff)) {
		k = 16;
		x >>= 16;
	}
	if (!(x & 0xff)) {
		k += 8;
		x >>= 8;
	}
	if (!(x & 0xf)) {
		k += 4;
		x >>= 4;
	}
	if (!(x & 0x3)) {
		k += 2;
		x >>= 2;
	}
	if (!(x & 1)) {
		k++;
		x >>= 1;
		if (!x)
			return 32;
	}
	*y = x;
	return k;
}

/* convert a small nonnegative integer to a Bigint */

static Bigint*
i2b(int i)
{
	Bigint* b;

	b = Balloc(1);
	if (b == NULL)
		return NULL;
	b->x[0] = i;
	b->wds = 1;
	return b;
}

/* multiply two Bigints.  Returns a new Bigint, or NULL on failure.  Ignores
   the signs of a and b. */

static Bigint*
mult(Bigint* a, Bigint* b)
{
	Bigint* c;
	int k, wa, wb, wc;
	ULong* x, * xa, * xae, * xb, * xbe, * xc, * xc0;
	ULong y;
	ULLong carry, z;

	if ((!a->x[0] && a->wds == 1) || (!b->x[0] && b->wds == 1)) {
		c = Balloc(0);
		if (c == NULL)
			return NULL;
		c->wds = 1;
		c->x[0] = 0;
		return c;
	}

	if (a->wds < b->wds) {
		c = a;
		a = b;
		b = c;
	}
	k = a->k;
	wa = a->wds;
	wb = b->wds;
	wc = wa + wb;
	if (wc > a->maxwds)
		k++;
	c = Balloc(k);
	if (c == NULL)
		return NULL;
	for (x = c->x, xa = x + wc; x < xa; x++)
		*x = 0;
	xa = a->x;
	xae = xa + wa;
	xb = b->x;
	xbe = xb + wb;
	xc0 = c->x;
	for (; xb < xbe; xc0++) {
		if ((y = *xb++)) {
			x = xa;
			xc = xc0;
			carry = 0;
			do {
				z = *x++ * (ULLong)y + *xc + carry;
				carry = z >> 32;
				*xc++ = (ULong)(z & FFFFFFFF);
			} while (x < xae);
			*xc = (ULong)carry;
		}
	}
	for (xc0 = c->x, xc = xc0 + wc; wc > 0 && !*--xc; --wc);
	c->wds = wc;
	return c;
}


static Bigint*
pow5mult(Bigint* b, int k)
{
	Bigint* b1, * p5, ** p5s;
	int i;
	static const int p05[3] = { 5, 25, 125 };

	if ((i = k & 3)) {
		b = multadd(b, p05[i - 1], 0);
		if (b == NULL)
			return NULL;
	}

	if (!(k >>= 2))
		return b;
	AlifInterpreter* interp = _alifInterpreter_get();
	p5s = interp->dtoa.p5s;
	for (;;) {
		p5 = *p5s;
		p5s++;
		if (k & 1) {
			b1 = mult(b, p5);
			Bfree(b);
			b = b1;
			if (b == NULL)
				return NULL;
		}
		if (!(k >>= 1))
			break;
	}
	return b;
}


static Bigint*
lshift(Bigint* b, int k)
{
	int i, k1, n, n1;
	Bigint* b1;
	ULong* x, * x1, * xe, z;

	if (!k || (!b->x[0] && b->wds == 1))
		return b;

	n = k >> 5;
	k1 = b->k;
	n1 = n + b->wds + 1;
	for (i = b->maxwds; n1 > i; i <<= 1)
		k1++;
	b1 = Balloc(k1);
	if (b1 == NULL) {
		Bfree(b);
		return NULL;
	}
	x1 = b1->x;
	for (i = 0; i < n; i++)
		*x1++ = 0;
	x = b->x;
	xe = x + b->wds;
	if (k &= 0x1f) {
		k1 = 32 - k;
		z = 0;
		do {
			*x1++ = *x << k | z;
			z = *x++ >> k1;
		} while (x < xe);
		if ((*x1 = z))
			++n1;
	}
	else do
		*x1++ = *x++;
	while (x < xe);
	b1->wds = n1 - 1;
	Bfree(b);
	return b1;
}

/* Do a three-way compare of a and b, returning -1 if a < b, 0 if a == b and
   1 if a > b.  Ignores signs of a and b. */

static int
cmp(Bigint* a, Bigint* b)
{
	ULong* xa, * xa0, * xb, * xb0;
	int i, j;

	i = a->wds;
	j = b->wds;
	if (i -= j)
		return i;
	xa0 = a->x;
	xa = xa0 + j;
	xb0 = b->x;
	xb = xb0 + j;
	for (;;) {
		if (*--xa != *--xb)
			return *xa < *xb ? -1 : 1;
		if (xa <= xa0)
			break;
	}
	return 0;
}

/* Take the difference of Bigints a and b, returning a new Bigint.  Returns
   NULL on failure.  The signs of a and b are ignored, but the sign of the
   result is set appropriately. */

static Bigint*
diff(Bigint* a, Bigint* b)
{
	Bigint* c;
	int i, wa, wb;
	ULong* xa, * xae, * xb, * xbe, * xc;
	ULLong borrow, y;

	i = cmp(a, b);
	if (!i) {
		c = Balloc(0);
		if (c == NULL)
			return NULL;
		c->wds = 1;
		c->x[0] = 0;
		return c;
	}
	if (i < 0) {
		c = a;
		a = b;
		b = c;
		i = 1;
	}
	else
		i = 0;
	c = Balloc(a->k);
	if (c == NULL)
		return NULL;
	c->sign = i;
	wa = a->wds;
	xa = a->x;
	xae = xa + wa;
	wb = b->wds;
	xb = b->x;
	xbe = xb + wb;
	xc = c->x;
	borrow = 0;
	do {
		y = (ULLong)*xa++ - *xb++ - borrow;
		borrow = y >> 32 & (ULong)1;
		*xc++ = (ULong)(y & FFFFFFFF);
	} while (xb < xbe);
	while (xa < xae) {
		y = *xa++ - borrow;
		borrow = y >> 32 & (ULong)1;
		*xc++ = (ULong)(y & FFFFFFFF);
	}
	while (!*--xc)
		wa--;
	c->wds = wa;
	return c;
}


static double ulp(U* x)
{
	Long L;
	U u;

	L = (word0(x) & Exp_mask) - (P - 1) * Exp_msk1;
	word0(&u) = L;
	word1(&u) = 0;
	return dval(&u);
}

/* Convert a Bigint to a double plus an exponent */

static double
b2d(Bigint* a, int* e)
{
	ULong* xa, * xa0, w, y, z;
	int k;
	U d;

	xa0 = a->x;
	xa = xa0 + a->wds;
	y = *--xa;
#ifdef DEBUG
	if (!y) Bug("zero y in b2d");
#endif
	k = hi0bits(y);
	*e = 32 - k;
	if (k < Ebits) {
		word0(&d) = Exp_1 | y >> (Ebits - k);
		w = xa > xa0 ? *--xa : 0;
		word1(&d) = y << ((32 - Ebits) + k) | w >> (Ebits - k);
		goto ret_d;
	}
	z = xa > xa0 ? *--xa : 0;
	if (k -= Ebits) {
		word0(&d) = Exp_1 | y << k | z >> (32 - k);
		y = xa > xa0 ? *--xa : 0;
		word1(&d) = z << k | y >> (32 - k);
	}
	else {
		word0(&d) = Exp_1 | y;
		word1(&d) = z;
	}
ret_d:
	return dval(&d);
}


static Bigint*
sd2b(U* d, int scale, int* e)
{
	Bigint* b;

	b = Balloc(1);
	if (b == NULL)
		return NULL;

	/* First construct b and e assuming that scale == 0. */
	b->wds = 2;
	b->x[0] = word1(d);
	b->x[1] = word0(d) & Frac_mask;
	*e = Etiny - 1 + (int)((word0(d) & Exp_mask) >> Exp_shift);
	if (*e < Etiny)
		*e = Etiny;
	else
		b->x[1] |= Exp_msk1;

	if (scale && (b->x[0] || b->x[1])) {
		*e -= scale;
		if (*e < Etiny) {
			scale = Etiny - *e;
			*e = Etiny;
			if (scale >= 32) {
				b->x[0] = b->x[1];
				b->x[1] = 0;
				scale -= 32;
			}
			if (scale) {
				/* The bits shifted out should all be zero. */
				b->x[0] = (b->x[0] >> scale) | (b->x[1] << (32 - scale));
				b->x[1] >>= scale;
			}
		}
	}
	if (!b->x[1])
		b->wds = 1;

	return b;
}

/* Convert a double to a Bigint plus an exponent.  Return NULL on failure.

   Given a finite nonzero double d, return an odd Bigint b and exponent *e
   such that fabs(d) = b * 2**e.  On return, *bbits gives the number of
   significant bits of b; that is, 2**(*bbits-1) <= b < 2**(*bbits).

   If d is zero, then b == 0, *e == -1010, *bbits = 0.
 */

static Bigint*
d2b(U* d, int* e, int* bits)
{
	Bigint* b;
	int de, k;
	ULong* x, y, z;
	int i;

	b = Balloc(1);
	if (b == NULL)
		return NULL;
	x = b->x;

	z = word0(d) & Frac_mask;
	word0(d) &= 0x7fffffff;   /* clear sign bit, which we ignore */
	if ((de = (int)(word0(d) >> Exp_shift)))
		z |= Exp_msk1;
	if ((y = word1(d))) {
		if ((k = lo0bits(&y))) {
			x[0] = y | z << (32 - k);
			z >>= k;
		}
		else
			x[0] = y;
		i =
			b->wds = (x[1] = z) ? 2 : 1;
	}
	else {
		k = lo0bits(&z);
		x[0] = z;
		i =
			b->wds = 1;
		k += 32;
	}
	if (de) {
		*e = de - Bias - (P - 1) + k;
		*bits = P - k;
	}
	else {
		*e = de - Bias - (P - 1) + 1 + k;
		*bits = 32 * i - hi0bits(x[i - 1]);
	}
	return b;
}

/* Compute the ratio of two Bigints, as a double.  The result may have an
   error of up to 2.5 ulps. */

static double
ratio(Bigint* a, Bigint* b)
{
	U da, db;
	int k, ka, kb;

	dval(&da) = b2d(a, &ka);
	dval(&db) = b2d(b, &kb);
	k = ka - kb + 32 * (a->wds - b->wds);
	if (k > 0)
		word0(&da) += k * Exp_msk1;
	else {
		k = -k;
		word0(&db) += k * Exp_msk1;
	}
	return dval(&da) / dval(&db);
}

static const double
tens[] = {
	1e0, 1e1, 1e2, 1e3, 1e4, 1e5, 1e6, 1e7, 1e8, 1e9,
	1e10, 1e11, 1e12, 1e13, 1e14, 1e15, 1e16, 1e17, 1e18, 1e19,
	1e20, 1e21, 1e22
};

static const double
bigtens[] = { 1e16, 1e32, 1e64, 1e128, 1e256 };
static const double tinytens[] = { 1e-16, 1e-32, 1e-64, 1e-128,
								   9007199254740992. * 9007199254740992.e-256
	/* = 2^106 * 1e-256 */
};
/* The factor of 2^53 in tinytens[4] helps us avoid setting the underflow */
/* flag unnecessarily.  It leads to a song and dance at the end of strtod. */
#define Scale_Bit 0x10
#define n_bigtens 5

#define ULbits 32
#define kshift 5
#define kmask 31


static int
dshift(Bigint* b, int p2)
{
	int rv = hi0bits(b->x[b->wds - 1]) - 4;
	if (p2 > 0)
		rv -= p2;
	return rv & kmask;
}

/* special case of Bigint division.  The quotient is always in the range 0 <=
   quotient < 10, and on entry the divisor S is normalized so that its top 4
   bits (28--31) are zero and bit 27 is set. */

static int
quorem(Bigint* b, Bigint* S)
{
	int n;
	ULong* bx, * bxe, q, * sx, * sxe;
	ULLong borrow, carry, y, ys;

	n = S->wds;
#ifdef DEBUG
	/*debug*/ if (b->wds > n)
		/*debug*/       Bug("oversize b in quorem");
#endif
	if (b->wds < n)
		return 0;
	sx = S->x;
	sxe = sx + --n;
	bx = b->x;
	bxe = bx + n;
	q = *bxe / (*sxe + 1);      /* ensure q <= true quotient */
#ifdef DEBUG
	/*debug*/ if (q > 9)
		/*debug*/       Bug("oversized quotient in quorem");
#endif
	if (q) {
		borrow = 0;
		carry = 0;
		do {
			ys = *sx++ * (ULLong)q + carry;
			carry = ys >> 32;
			y = *bx - (ys & FFFFFFFF) - borrow;
			borrow = y >> 32 & (ULong)1;
			*bx++ = (ULong)(y & FFFFFFFF);
		} while (sx <= sxe);
		if (!*bxe) {
			bx = b->x;
			while (--bxe > bx && !*bxe)
				--n;
			b->wds = n;
		}
	}
	if (cmp(b, S) >= 0) {
		q++;
		borrow = 0;
		carry = 0;
		bx = b->x;
		sx = S->x;
		do {
			ys = *sx++ + carry;
			carry = ys >> 32;
			y = *bx - (ys & FFFFFFFF) - borrow;
			borrow = y >> 32 & (ULong)1;
			*bx++ = (ULong)(y & FFFFFFFF);
		} while (sx <= sxe);
		bx = b->x;
		bxe = bx + n;
		if (!*bxe) {
			while (--bxe > bx && !*bxe)
				--n;
			b->wds = n;
		}
	}
	return q;
}


static double
sulp(U* x, BCinfo* bc)
{
	U u;

	if (bc->scale && 2 * P + 1 > (int)((word0(x) & Exp_mask) >> Exp_shift)) {
		/* rv/2^bc->scale is subnormal */
		word0(&u) = (P + 2) * Exp_msk1;
		word1(&u) = 0;
		return u.d;
	}
	else {
		return ulp(x);
	}
}


static int
bigcomp(U* rv, const char* s0, BCinfo* bc)
{
	Bigint* b, * d;
	int b2, d2, dd, i, nd, nd0, odd, p2, p5;

	nd = bc->nd;
	nd0 = bc->nd0;
	p5 = nd + bc->e0;
	b = sd2b(rv, bc->scale, &p2);
	if (b == NULL)
		return -1;

	/* record whether the lsb of rv/2^(bc->scale) is odd:  in the exact halfway
	   case, this is used for round to even. */
	odd = b->x[0] & 1;

	/* left shift b by 1 bit and or a 1 into the least significant bit;
	   this gives us b * 2**p2 = rv/2^(bc->scale) + 0.5 ulp. */
	b = lshift(b, 1);
	if (b == NULL)
		return -1;
	b->x[0] |= 1;
	p2--;

	p2 -= p5;
	d = i2b(1);
	if (d == NULL) {
		Bfree(b);
		return -1;
	}
	/* Arrange for convenient computation of quotients:
	 * shift left if necessary so divisor has 4 leading 0 bits.
	 */
	if (p5 > 0) {
		d = pow5mult(d, p5);
		if (d == NULL) {
			Bfree(b);
			return -1;
		}
	}
	else if (p5 < 0) {
		b = pow5mult(b, -p5);
		if (b == NULL) {
			Bfree(d);
			return -1;
		}
	}
	if (p2 > 0) {
		b2 = p2;
		d2 = 0;
	}
	else {
		b2 = 0;
		d2 = -p2;
	}
	i = dshift(d, d2);
	if ((b2 += i) > 0) {
		b = lshift(b, b2);
		if (b == NULL) {
			Bfree(d);
			return -1;
		}
	}
	if ((d2 += i) > 0) {
		d = lshift(d, d2);
		if (d == NULL) {
			Bfree(b);
			return -1;
		}
	}

	/* Compare s0 with b/d: set dd to -1, 0, or 1 according as s0 < b/d, s0 ==
	 * b/d, or s0 > b/d.  Here the digits of s0 are thought of as representing
	 * a number in the range [0.1, 1). */
	if (cmp(b, d) >= 0)
		/* b/d >= 1 */
		dd = -1;
	else {
		i = 0;
		for (;;) {
			b = multadd(b, 10, 0);
			if (b == NULL) {
				Bfree(d);
				return -1;
			}
			dd = s0[i < nd0 ? i : i + 1] - '0' - quorem(b, d);
			i++;

			if (dd)
				break;
			if (!b->x[0] && b->wds == 1) {
				/* b/d == 0 */
				dd = i < nd;
				break;
			}
			if (!(i < nd)) {
				/* b/d != 0, but digits of s0 exhausted */
				dd = -1;
				break;
			}
		}
	}
	Bfree(b);
	Bfree(d);
	if (dd > 0 || (dd == 0 && odd))
		dval(rv) += sulp(rv, bc);
	return 0;
}




double _alif_dgStrToDouble(const char* s00, char** se) { // 1383
	int bb2, bb5, bbe, bd2, bd5, bs2, c, dsign, e, e1, error;
	int esign, i, j, k, lz, nd, nd0, odd, sign;
	const char* s, * s0, * s1;
	double aadj, aadj1;
	U aadj2, adj, rv, rv0;
	ULong y, z, abs_exp;
	Long L;
	BCinfo bc;
	Bigint* bb = NULL, * bd = NULL, * bd0 = NULL, * bs = NULL, * delta = NULL;
	size_t ndigits, fraclen;
	double result;

	dval(&rv) = 0.;

	/* Start parsing. */
	c = *(s = s00);

	/* Parse optional sign, if present. */
	sign = 0;
	switch (c) {
	case '-':
		sign = 1;
	case '+':
		c = *++s;
	}

	/* Skip leading zeros: lz is true iff there were leading zeros. */
	s1 = s;
	while (c == '0')
		c = *++s;
	lz = s != s1;

	/* Point s0 at the first nonzero digit (if any).  fraclen will be the
	   number of digits between the decimal point and the end of the
	   digit string.  ndigits will be the total number of digits ignoring
	   leading zeros. */
	s0 = s1 = s;
	while ('0' <= c && c <= '9')
		c = *++s;
	ndigits = s - s1;
	fraclen = 0;

	/* Parse decimal point and following digits. */
	if (c == '.') {
		c = *++s;
		if (!ndigits) {
			s1 = s;
			while (c == '0')
				c = *++s;
			lz = lz || s != s1;
			fraclen += (s - s1);
			s0 = s;
		}
		s1 = s;
		while ('0' <= c && c <= '9')
			c = *++s;
		ndigits += s - s1;
		fraclen += s - s1;
	}

	/* Now lz is true if and only if there were leading zero digits, and
	   ndigits gives the total number of digits ignoring leading zeros.  A
	   valid input must have at least one digit. */
	if (!ndigits && !lz) {
		if (se)
			*se = (char*)s00;
		goto parse_error;
	}

	/* Range check ndigits and fraclen to make sure that they, and values
	   computed with them, can safely fit in an int. */
	if (ndigits > MAX_DIGITS || fraclen > MAX_DIGITS) {
		if (se)
			*se = (char*)s00;
		goto parse_error;
	}
	nd = (int)ndigits;
	nd0 = (int)ndigits - (int)fraclen;

	/* Parse exponent. */
	e = 0;
	if (c == 'e' || c == 'E') {
		s00 = s;
		c = *++s;

		/* Exponent sign. */
		esign = 0;
		switch (c) {
		case '-':
			esign = 1;
		case '+':
			c = *++s;
		}

		/* Skip zeros.  lz is true iff there are leading zeros. */
		s1 = s;
		while (c == '0')
			c = *++s;
		lz = s != s1;

		/* Get absolute value of the exponent. */
		s1 = s;
		abs_exp = 0;
		while ('0' <= c && c <= '9') {
			abs_exp = 10 * abs_exp + (c - '0');
			c = *++s;
		}

		/* abs_exp will be correct modulo 2**32.  But 10**9 < 2**32, so if
		   there are at most 9 significant exponent digits then overflow is
		   impossible. */
		if (s - s1 > 9 || abs_exp > MAX_ABS_EXP)
			e = (int)MAX_ABS_EXP;
		else
			e = (int)abs_exp;
		if (esign)
			e = -e;

		/* A valid exponent must have at least one digit. */
		if (s == s1 && !lz)
			s = s00;
	}

	/* Adjust exponent to take into account position of the point. */
	e -= nd - nd0;
	if (nd0 <= 0)
		nd0 = nd;

	/* Finished parsing.  Set se to indicate how far we parsed */
	if (se)
		*se = (char*)s;

	/* If all digits were zero, exit with return value +-0.0.  Otherwise,
	   strip trailing zeros: scan back until we hit a nonzero digit. */
	if (!nd)
		goto ret;
	for (i = nd; i > 0; ) {
		--i;
		if (s0[i < nd0 ? i : i + 1] != '0') {
			++i;
			break;
		}
	}
	e += nd - i;
	nd = i;
	if (nd0 > nd)
		nd0 = nd;

	/* Summary of parsing results.  After parsing, and dealing with zero
	 * inputs, we have values s0, nd0, nd, e, sign, where:
	 *
	 *  - s0 points to the first significant digit of the input string
	 *
	 *  - nd is the total number of significant digits (here, and
	 *    below, 'significant digits' means the set of digits of the
	 *    significand of the input that remain after ignoring leading
	 *    and trailing zeros).
	 *
	 *  - nd0 indicates the position of the decimal point, if present; it
	 *    satisfies 1 <= nd0 <= nd.  The nd significant digits are in
	 *    s0[0:nd0] and s0[nd0+1:nd+1] using the usual Python half-open slice
	 *    notation.  (If nd0 < nd, then s0[nd0] contains a '.'  character; if
	 *    nd0 == nd, then s0[nd0] could be any non-digit character.)
	 *
	 *  - e is the adjusted exponent: the absolute value of the number
	 *    represented by the original input string is n * 10**e, where
	 *    n is the integer represented by the concatenation of
	 *    s0[0:nd0] and s0[nd0+1:nd+1]
	 *
	 *  - sign gives the sign of the input:  1 for negative, 0 for positive
	 *
	 *  - the first and last significant digits are nonzero
	 */

	 /* put first DBL_DIG+1 digits into integer y and z.
	  *
	  *  - y contains the value represented by the first min(9, nd)
	  *    significant digits
	  *
	  *  - if nd > 9, z contains the value represented by significant digits
	  *    with indices in [9, min(16, nd)).  So y * 10**(min(16, nd) - 9) + z
	  *    gives the value represented by the first min(16, nd) sig. digits.
	  */

	bc.e0 = e1 = e;
	y = z = 0;
	for (i = 0; i < nd; i++) {
		if (i < 9)
			y = 10 * y + s0[i < nd0 ? i : i + 1] - '0';
		else if (i < DBL_DIG + 1)
			z = 10 * z + s0[i < nd0 ? i : i + 1] - '0';
		else
			break;
	}

	k = nd < DBL_DIG + 1 ? nd : DBL_DIG + 1;
	dval(&rv) = y;
	if (k > 9) {
		dval(&rv) = tens[k - 9] * dval(&rv) + z;
	}
	if (nd <= DBL_DIG
		&& Flt_Rounds == 1
		) {
		if (!e)
			goto ret;
		if (e > 0) {
			if (e <= Ten_pmax) {
				dval(&rv) *= tens[e];
				goto ret;
			}
			i = DBL_DIG - nd;
			if (e <= Ten_pmax + i) {
				/* A fancier test would sometimes let us do
				 * this for larger i values.
				 */
				e -= i;
				dval(&rv) *= tens[i];
				dval(&rv) *= tens[e];
				goto ret;
			}
		}
		else if (e >= -Ten_pmax) {
			dval(&rv) /= tens[-e];
			goto ret;
		}
	}
	e1 += nd - k;

	bc.scale = 0;

	/* Get starting approximation = rv * 10**e1 */

	if (e1 > 0) {
		if ((i = e1 & 15))
			dval(&rv) *= tens[i];
		if (e1 &= ~15) {
			if (e1 > DBL_MAX_10_EXP)
				goto ovfl;
			e1 >>= 4;
			for (j = 0; e1 > 1; j++, e1 >>= 1)
				if (e1 & 1)
					dval(&rv) *= bigtens[j];
			/* The last multiplication could overflow. */
			word0(&rv) -= P * Exp_msk1;
			dval(&rv) *= bigtens[j];
			if ((z = word0(&rv) & Exp_mask)
	> Exp_msk1 * (DBL_MAX_EXP + Bias - P))
				goto ovfl;
			if (z > Exp_msk1 * (DBL_MAX_EXP + Bias - 1 - P)) {
				/* set to largest number */
				/* (Can't trust DBL_MAX) */
				word0(&rv) = Big0;
				word1(&rv) = Big1;
			}
			else
				word0(&rv) += P * Exp_msk1;
		}
	}
	else if (e1 < 0) {
		/* The input decimal value lies in [10**e1, 10**(e1+16)).

		   If e1 <= -512, underflow immediately.
		   If e1 <= -256, set bc.scale to 2*P.

		   So for input value < 1e-256, bc.scale is always set;
		   for input value >= 1e-240, bc.scale is never set.
		   For input values in [1e-256, 1e-240), bc.scale may or may
		   not be set. */

		e1 = -e1;
		if ((i = e1 & 15))
			dval(&rv) /= tens[i];
		if (e1 >>= 4) {
			if (e1 >= 1 << n_bigtens)
				goto undfl;
			if (e1 & Scale_Bit)
				bc.scale = 2 * P;
			for (j = 0; e1 > 0; j++, e1 >>= 1)
				if (e1 & 1)
					dval(&rv) *= tinytens[j];
			if (bc.scale && (j = 2 * P + 1 - ((word0(&rv) & Exp_mask)
				>> Exp_shift)) > 0) {
				/* scaled rv is denormal; clear j low bits */
				if (j >= 32) {
					word1(&rv) = 0;
					if (j >= 53)
						word0(&rv) = (P + 2) * Exp_msk1;
					else
						word0(&rv) &= 0xffffffff << (j - 32);
				}
				else
					word1(&rv) &= 0xffffffff << j;
			}
			if (!dval(&rv))
				goto undfl;
		}
	}

	/* Now the hard part -- adjusting rv to the correct value.*/

	/* Put digits into bd: true value = bd * 10^e */

	bc.nd = nd;
	bc.nd0 = nd0;       /* Only needed if nd > STRTOD_DIGLIM, but done here */
	/* to silence an erroneous warning about bc.nd0 */
	/* possibly not being initialized. */
	if (nd > STRTOD_DIGLIM) {
		/* ASSERT(STRTOD_DIGLIM >= 18); 18 == one more than the */
		/* minimum number of decimal digits to distinguish double values */
		/* in IEEE arithmetic. */

		/* Truncate input to 18 significant digits, then discard any trailing
		   zeros on the result by updating nd, nd0, e and y suitably. (There's
		   no need to update z; it's not reused beyond this point.) */
		for (i = 18; i > 0; ) {
			/* scan back until we hit a nonzero digit.  significant digit 'i'
			is s0[i] if i < nd0, s0[i+1] if i >= nd0. */
			--i;
			if (s0[i < nd0 ? i : i + 1] != '0') {
				++i;
				break;
			}
		}
		e += nd - i;
		nd = i;
		if (nd0 > nd)
			nd0 = nd;
		if (nd < 9) { /* must recompute y */
			y = 0;
			for (i = 0; i < nd0; ++i)
				y = 10 * y + s0[i] - '0';
			for (; i < nd; ++i)
				y = 10 * y + s0[i + 1] - '0';
		}
	}
	bd0 = s2b(s0, nd0, nd, y);
	if (bd0 == NULL)
		goto failed_malloc;

	/* Notation for the comments below.  Write:

		 - dv for the absolute value of the number represented by the original
		   decimal input string.

		 - if we've truncated dv, write tdv for the truncated value.
		   Otherwise, set tdv == dv.

		 - srv for the quantity rv/2^bc.scale; so srv is the current binary
		   approximation to tdv (and dv).  It should be exactly representable
		   in an IEEE 754 double.
	*/

	for (;;) {

		/* This is the main correction loop for _Py_dg_strtod.

		   We've got a decimal value tdv, and a floating-point approximation
		   srv=rv/2^bc.scale to tdv.  The aim is to determine whether srv is
		   close enough (i.e., within 0.5 ulps) to tdv, and to compute a new
		   approximation if not.

		   To determine whether srv is close enough to tdv, compute integers
		   bd, bb and bs proportional to tdv, srv and 0.5 ulp(srv)
		   respectively, and then use integer arithmetic to determine whether
		   |tdv - srv| is less than, equal to, or greater than 0.5 ulp(srv).
		*/

		bd = Balloc(bd0->k);
		if (bd == NULL) {
			goto failed_malloc;
		}
		Bcopy(bd, bd0);
		bb = sd2b(&rv, bc.scale, &bbe);   /* srv = bb * 2^bbe */
		if (bb == NULL) {
			goto failed_malloc;
		}
		/* Record whether lsb of bb is odd, in case we need this
		   for the round-to-even step later. */
		odd = bb->x[0] & 1;

		/* tdv = bd * 10**e;  srv = bb * 2**bbe */
		bs = i2b(1);
		if (bs == NULL) {
			goto failed_malloc;
		}

		if (e >= 0) {
			bb2 = bb5 = 0;
			bd2 = bd5 = e;
		}
		else {
			bb2 = bb5 = -e;
			bd2 = bd5 = 0;
		}
		if (bbe >= 0)
			bb2 += bbe;
		else
			bd2 -= bbe;
		bs2 = bb2;
		bb2++;
		bd2++;

		/* At this stage bd5 - bb5 == e == bd2 - bb2 + bbe, bb2 - bs2 == 1,
		   and bs == 1, so:

			  tdv == bd * 10**e = bd * 2**(bbe - bb2 + bd2) * 5**(bd5 - bb5)
			  srv == bb * 2**bbe = bb * 2**(bbe - bb2 + bb2)
			  0.5 ulp(srv) == 2**(bbe-1) = bs * 2**(bbe - bb2 + bs2)

		   It follows that:

			  M * tdv = bd * 2**bd2 * 5**bd5
			  M * srv = bb * 2**bb2 * 5**bb5
			  M * 0.5 ulp(srv) = bs * 2**bs2 * 5**bb5

		   for some constant M.  (Actually, M == 2**(bb2 - bbe) * 5**bb5, but
		   this fact is not needed below.)
		*/

		/* Remove factor of 2**i, where i = min(bb2, bd2, bs2). */
		i = bb2 < bd2 ? bb2 : bd2;
		if (i > bs2)
			i = bs2;
		if (i > 0) {
			bb2 -= i;
			bd2 -= i;
			bs2 -= i;
		}

		/* Scale bb, bd, bs by the appropriate powers of 2 and 5. */
		if (bb5 > 0) {
			bs = pow5mult(bs, bb5);
			if (bs == NULL) {
				goto failed_malloc;
			}
			Bigint* bb1 = mult(bs, bb);
			Bfree(bb);
			bb = bb1;
			if (bb == NULL) {
				goto failed_malloc;
			}
		}
		if (bb2 > 0) {
			bb = lshift(bb, bb2);
			if (bb == NULL) {
				goto failed_malloc;
			}
		}
		if (bd5 > 0) {
			bd = pow5mult(bd, bd5);
			if (bd == NULL) {
				goto failed_malloc;
			}
		}
		if (bd2 > 0) {
			bd = lshift(bd, bd2);
			if (bd == NULL) {
				goto failed_malloc;
			}
		}
		if (bs2 > 0) {
			bs = lshift(bs, bs2);
			if (bs == NULL) {
				goto failed_malloc;
			}
		}

		/* Now bd, bb and bs are scaled versions of tdv, srv and 0.5 ulp(srv),
		   respectively.  Compute the difference |tdv - srv|, and compare
		   with 0.5 ulp(srv). */

		delta = diff(bb, bd);
		if (delta == NULL) {
			goto failed_malloc;
		}
		dsign = delta->sign;
		delta->sign = 0;
		i = cmp(delta, bs);
		if (bc.nd > nd && i <= 0) {
			if (dsign)
				break;  /* Must use bigcomp(). */

			/* Here rv overestimates the truncated decimal value by at most
			   0.5 ulp(rv).  Hence rv either overestimates the true decimal
			   value by <= 0.5 ulp(rv), or underestimates it by some small
			   amount (< 0.1 ulp(rv)); either way, rv is within 0.5 ulps of
			   the true decimal value, so it's possible to exit.

			   Exception: if scaled rv is a normal exact power of 2, but not
			   DBL_MIN, then rv - 0.5 ulp(rv) takes us all the way down to the
			   next double, so the correctly rounded result is either rv - 0.5
			   ulp(rv) or rv; in this case, use bigcomp to distinguish. */

			if (!word1(&rv) && !(word0(&rv) & Bndry_mask)) {
				/* rv can't be 0, since it's an overestimate for some
				   nonzero value.  So rv is a normal power of 2. */
				j = (int)(word0(&rv) & Exp_mask) >> Exp_shift;
				/* rv / 2^bc.scale = 2^(j - 1023 - bc.scale); use bigcomp if
				   rv / 2^bc.scale >= 2^-1021. */
				if (j - bc.scale >= 2) {
					dval(&rv) -= 0.5 * sulp(&rv, &bc);
					break; /* Use bigcomp. */
				}
			}

			{
				bc.nd = nd;
				i = -1; /* Discarded digits make delta smaller. */
			}
		}

		if (i < 0) {
			/* Error is less than half an ulp -- check for
			 * special case of mantissa a power of two.
			 */
			if (dsign || word1(&rv) || word0(&rv) & Bndry_mask
				|| (word0(&rv) & Exp_mask) <= (2 * P + 1) * Exp_msk1
				) {
				break;
			}
			if (!delta->x[0] && delta->wds <= 1) {
				/* exact result */
				break;
			}
			delta = lshift(delta, Log2P);
			if (delta == NULL) {
				goto failed_malloc;
			}
			if (cmp(delta, bs) > 0)
				goto drop_down;
			break;
		}
		if (i == 0) {
			/* exactly half-way between */
			if (dsign) {
				if ((word0(&rv) & Bndry_mask1) == Bndry_mask1
					&& word1(&rv) == (
						(bc.scale &&
							(y = word0(&rv) & Exp_mask) <= 2 * P * Exp_msk1) ?
						(0xffffffff & (0xffffffff << (2 * P + 1 - (y >> Exp_shift)))) :
						0xffffffff)) {
					/*boundary case -- increment exponent*/
					word0(&rv) = (word0(&rv) & Exp_mask)
						+ Exp_msk1
						;
					word1(&rv) = 0;
					/* dsign = 0; */
					break;
				}
			}
			else if (!(word0(&rv) & Bndry_mask) && !word1(&rv)) {
			drop_down:
				/* boundary case -- decrement exponent */
				if (bc.scale) {
					L = word0(&rv) & Exp_mask;
					if (L <= (2 * P + 1) * Exp_msk1) {
						if (L > (P + 2) * Exp_msk1)
							/* round even ==> */
							/* accept rv */
							break;
						/* rv = smallest denormal */
						if (bc.nd > nd)
							break;
						goto undfl;
					}
				}
				L = (word0(&rv) & Exp_mask) - Exp_msk1;
				word0(&rv) = L | Bndry_mask1;
				word1(&rv) = 0xffffffff;
				break;
			}
			if (!odd)
				break;
			if (dsign)
				dval(&rv) += sulp(&rv, &bc);
			else {
				dval(&rv) -= sulp(&rv, &bc);
				if (!dval(&rv)) {
					if (bc.nd > nd)
						break;
					goto undfl;
				}
			}
			/* dsign = 1 - dsign; */
			break;
		}
		if ((aadj = ratio(delta, bs)) <= 2.) {
			if (dsign)
				aadj = aadj1 = 1.;
			else if (word1(&rv) || word0(&rv) & Bndry_mask) {
				if (word1(&rv) == Tiny1 && !word0(&rv)) {
					if (bc.nd > nd)
						break;
					goto undfl;
				}
				aadj = 1.;
				aadj1 = -1.;
			}
			else {
				/* special case -- power of FLT_RADIX to be */
				/* rounded down... */

				if (aadj < 2. / FLT_RADIX)
					aadj = 1. / FLT_RADIX;
				else
					aadj *= 0.5;
				aadj1 = -aadj;
			}
		}
		else {
			aadj *= 0.5;
			aadj1 = dsign ? aadj : -aadj;
			if (Flt_Rounds == 0)
				aadj1 += 0.5;
		}
		y = word0(&rv) & Exp_mask;

		/* Check for overflow */

		if (y == Exp_msk1 * (DBL_MAX_EXP + Bias - 1)) {
			dval(&rv0) = dval(&rv);
			word0(&rv) -= P * Exp_msk1;
			adj.d = aadj1 * ulp(&rv);
			dval(&rv) += adj.d;
			if ((word0(&rv) & Exp_mask) >=
				Exp_msk1 * (DBL_MAX_EXP + Bias - P)) {
				if (word0(&rv0) == Big0 && word1(&rv0) == Big1) {
					goto ovfl;
				}
				word0(&rv) = Big0;
				word1(&rv) = Big1;
				goto cont;
			}
			else
				word0(&rv) += P * Exp_msk1;
		}
		else {
			if (bc.scale && y <= 2 * P * Exp_msk1) {
				if (aadj <= 0x7fffffff) {
					if ((z = (ULong)aadj) <= 0)
						z = 1;
					aadj = z;
					aadj1 = dsign ? aadj : -aadj;
				}
				dval(&aadj2) = aadj1;
				word0(&aadj2) += (2 * P + 1) * Exp_msk1 - y;
				aadj1 = dval(&aadj2);
			}
			adj.d = aadj1 * ulp(&rv);
			dval(&rv) += adj.d;
		}
		z = word0(&rv) & Exp_mask;
		if (bc.nd == nd) {
			if (!bc.scale)
				if (y == z) {
					/* Can we stop now? */
					L = (Long)aadj;
					aadj -= L;
					/* The tolerances below are conservative. */
					if (dsign || word1(&rv) || word0(&rv) & Bndry_mask) {
						if (aadj < .4999999 || aadj > .5000001)
							break;
					}
					else if (aadj < .4999999 / FLT_RADIX)
						break;
				}
		}
	cont:
		Bfree(bb); bb = NULL;
		Bfree(bd); bd = NULL;
		Bfree(bs); bs = NULL;
		Bfree(delta); delta = NULL;
	}
	if (bc.nd > nd) {
		error = bigcomp(&rv, s0, &bc);
		if (error)
			goto failed_malloc;
	}

	if (bc.scale) {
		word0(&rv0) = Exp_1 - 2 * P * Exp_msk1;
		word1(&rv0) = 0;
		dval(&rv) *= dval(&rv0);
	}

ret:
	result = sign ? -dval(&rv) : dval(&rv);
	goto done;

parse_error:
	result = 0.0;
	goto done;

failed_malloc:
	errno = ENOMEM;
	result = -1.0;
	goto done;

undfl:
	result = sign ? -0.0 : 0.0;
	goto done;

ovfl:
	errno = ERANGE;
	/* Can't trust HUGE_VAL */
	word0(&rv) = Exp_mask;
	word1(&rv) = 0;
	result = sign ? -dval(&rv) : dval(&rv);
	goto done;

done:
	Bfree(bb);
	Bfree(bd);
	Bfree(bs);
	Bfree(bd0);
	Bfree(delta);
	return result;
}
