#include "alif.h"

#include "AlifCore_DoubleToASCII.h"
#include "AlifCore_State.h"








typedef int32_t Long; // 161
typedef uint64_t ULLong;


union U { double d; ULong L[2]; }; // 175

#define MAX_DIGITS 1000000000U // 202


 // 279
typedef struct BCinfo BCinfo;
struct
	BCinfo {
	int e0, nd, nd0, scale;
};

#define FFFFFFFF 0xffffffffUL









static double sulp(U* x, BCinfo* bc) { // 1209
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
