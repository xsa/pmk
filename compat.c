/* $Id$ */

/*
 * Copyright (c) 2003-2005 Damien Couderc
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *    - Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *    - Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following
 *      disclaimer in the documentation and/or other materials provided
 *      with the distribution.
 *    - Neither the name of the copyright holder(s) nor the names of its
 *      contributors may be used to endorse or promote products derived
 *      from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */


#include <sys/param.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "compat.h"
#include "compat/config.h"

#include "compat/pmk_ctype.h"
#include "compat/pmk_libgen.h"
#include "compat/pmk_stdbool.h"
#include "compat/pmk_string.h"
#include "compat/pmk_sys_types.h"
#include "compat/pmk_unistd.h"


/**********
 * macros *
 ***********************************************************************/

/* macros for structure */
#define	SET_FLAG(f)		data.flags = data.flags | f
#define	UNSET_FLAG(f)	data.flags = data.flags & (FLAG_ALL ^ f);
#define	F_IS_SET(f)		(data.flags & f) != 0
#define	F_IS_UNSET(f)	(data.flags & f) == 0

/* macros for pointer of structure */
#define	SET_PFLAG(f)	pdt->flags = pdt->flags | f
#define	UNSET_PFLAG(f)	pdt->flags = pdt->flags & (FLAG_ALL ^ f);
#define	PF_IS_SET(f)	(pdt->flags & f) != 0
#define	PF_IS_UNSET(f)	(pdt->flags & f) == 0


/*************
 * functions *
 ***********************************************************************/

/*****************
 * fill_buffer() *
 ***********************************************************************
 DESCR
	Append a character in the buffer if it does not override the
	buffer's length

 IN
 	pdt :		vsnprintf common data structure
	chr :		char to append

 OUT
	NONE
 ***********************************************************************/

static void fill_buffer(vsnp_t *pdt, char chr) {
#ifdef DEBUG_VSNPRINTF
	printf("cursor = %d, char = '%c'\n", pdt->cur, chr);
#endif

	/* if buffer is not full */
	if (pdt->cur < pdt->len) {
		/* insert character */
		pdt->buf[pdt->cur] = chr;
	}

	/* increment cursor position */
	pdt->cur++;
}


/******************
 * build_number() *
 ***********************************************************************
 DESCR
	Convert an unsigned integer into a string. The resulting string is
	in the reverse order without terminating null character.

 IN
	buffer :	buffer location
	len :		buffer size
	value :		unsigned integer to convert
 	pdt :		vsnprintf common data structure

 OUT
	size of the resulting string
 ***********************************************************************/

static size_t build_number(char *buf, size_t len, unsigned_t value,
													base_t base, flag_t flags) {
	char	*basestr;
	size_t	 vlen = 0;

	/* if value is equal to zero */
	if (value == 0) {
		buf[0] = '0';
		return(1);
	}

	/* select reference string relative to the case */
	if ((flags & FLAG_UPPERCASE) != 0) {
		basestr = UPPER_BASE;
	} else {
		basestr = LOWER_BASE;
	}

	/* create number string in reverse order */
	while ((value > 0) && (vlen < len)) {
		buf[vlen] = basestr[value % base];
		value = value / base;
		vlen++;
	}

	return(vlen);
}


/*****************
 * convert_int() *
 ***********************************************************************
 DESCR
	Convert an integer value

 IN
 	pdt :		vsnprintf common data structure
	value :		unsigned integer to convert

 OUT
	NONE
 ***********************************************************************/

static void convert_int(vsnp_t *pdt, unsigned_t value) {
	char	nbuf[MAXINTLEN+1],
			sign = ' ';
	int		vlen,
			zplen,
			splen,
			have_sign = 0,
			have_hex_prefix = 0;

	if (PF_IS_SET(FLAG_NEGATIVE_VALUE)) {
		/* on negative value set the sign */
		have_sign = 1;
		sign = '-';
	} else {
		/* else if + flag is set display the positive sign */
		if (PF_IS_SET(FLAG_SIGNED)) {
			have_sign = 1;
			sign = '+';
		}
	}

	vlen = build_number(nbuf, sizeof(nbuf), value, pdt->base, pdt->flags);

	/* compute zero padding */
	zplen = pdt->prec - vlen;
	if (zplen < 0) {
		zplen = 0;
	}

	/* compute space padding */
	if (pdt->prec > vlen) {
		splen = pdt->fwidth - pdt->prec;
	} else {
		splen = pdt->fwidth - vlen;
	}
	if (have_sign != 0) {
		splen--;
	}
	if (PF_IS_SET(FLAG_ALTERNATIVE_FORM)) {
		switch (pdt->base) {
			case BASE_OCT :
				/* octal alternative form */
				if ((value == 0) && (pdt->prec == 0)) {
					pdt->prec = 1;
				}
				break;

			case BASE_HEX :
				/* hex alternative form */
				if (value > 0) {
					have_hex_prefix = 1;
					splen = splen - 2; /* length of '0x' */
				}
				break;
		}
	}
	if (splen < 0) {
		splen = 0;
	}

	/* right justify space padding */
	if (PF_IS_UNSET(FLAG_LEFT_JUSTIFIED)) {
		while (splen > 0) {
			fill_buffer(pdt, ' ');
			splen--;
		}
	}

	/* if we have a sign */
	if (have_sign != 0) {
		fill_buffer(pdt, sign);
	}

	/* hex alternative form */
	if (have_hex_prefix != 0) {
		fill_buffer(pdt, '0');
		if (PF_IS_UNSET(FLAG_UPPERCASE)) {
			fill_buffer(pdt, 'x');
		} else {
			fill_buffer(pdt, 'X');
		}
	}

	/* zero padding */
	while (zplen > 0) {
		fill_buffer(pdt, '0');
		zplen--;
	}

	/* converted integer */
	while (vlen > 0) {
		fill_buffer(pdt, nbuf[vlen - 1]);
		vlen--;
	}

	/* left justify space padding */
	if (PF_IS_SET(FLAG_LEFT_JUSTIFIED)) {
		while (splen > 0) {
			fill_buffer(pdt, ' ');
			splen--;
		}
	}
}


/******************
 * convert_sint() *
 ***********************************************************************
 DESCR
 	Convert a signed integer

 IN
 	pdt :		vsnprintf common data structure
	value :		unsigned integer to convert

 OUT
	NONE
 ***********************************************************************/

static void convert_sint(vsnp_t *pdt, signed_t value) {

	/* check if value is negative */
	if (value < 0) {
		/* if true set flag */
		SET_PFLAG(FLAG_NEGATIVE_VALUE);
		value = -value;
	}

	/* call generic conversion procedure */
	convert_int(pdt, (unsigned_t) value);
}


/*****************
 * convert_str() *
 ***********************************************************************
 DESCR
	Convert the string. It means copying the string in the place of the
	conversion identifier.

 IN
 	pdt :		vsnprintf common data structure
	ptr :		string to convert

 OUT
	NONE
 ***********************************************************************/

static void convert_str(vsnp_t *pdt, char *ptr) {
	while ((pdt->cur < pdt->len) && (pdt->prec > 0) && (*ptr != '\0')) {
		fill_buffer(pdt, *ptr);
		ptr++;
		pdt->prec--;
	}
}


/*****************
 * conv_to_exp() *
 ***********************************************************************
 DESCR
	Convert to exponent form

 IN
 	pdt :		vsnprintf common data structure
	expnt :		exponent value

 OUT
	exponent presence flag
 ***********************************************************************/

static int conv_to_exp(vsnp_t *pdt, flt_t *pval, int *pexp) {
	flt_t	uval;
	int		expnt,
			have_exp = 1;


	/* work on local variable to save value in case of g conversion */
	uval = *pval;

	/* init exponent value */
	expnt = 0;

	/* process value to get only one digit before decimal point */
	if (uval >= 1) {
		while (uval > pdt->base) {
			uval = uval / pdt->base;
			expnt++;
		}
	} else {
		while (uval < 1) {
			uval = uval * pdt->base;
			expnt--;
		}
	}

	/* check if exponent is needed for g conversion */
	if (PF_IS_SET(FLAG_G_CONVERSION)) {
		if ((expnt >= -4) && (expnt < pdt->prec)) { /* XXX use define for ISOC99 lower limit */
			/* unset exponent flag */
			have_exp = 0;

			/* get back to original value */
			uval = *pval;
		}
	}

	/* set values */
	*pval = uval;
	*pexp = expnt;

	return(have_exp);
}


/***************
 * print_exp() *
 ***********************************************************************
 DESCR
	process exponent

 IN
 	pdt :		vsnprintf common data structure
	expnt :		exponent value

 OUT
	NONE
 ***********************************************************************/

static void print_exp(vsnp_t *pdt, int expnt) {
	char	ebuf[MAXINTLEN+1],
			sign= ' ';

	/* exponent sign */
	if (expnt < 0) {
		sign = '-';
	} else {
		sign = '+';
	}

	/* exponent type may vary */
	ebuf[1] = '0'; /* init */
	switch (pdt->base) {
		case BASE_DEC :
			/* g or G conversion */
			if (PF_IS_SET(FLAG_UPPERCASE)) {
				fill_buffer(pdt, 'E');
			} else {
				fill_buffer(pdt, 'e');
			}

			fill_buffer(pdt, sign);

			/* exponent 2 digits */
			build_number(ebuf, 2, expnt, BASE_DEC, FLAG_NONE);
			fill_buffer(pdt, ebuf[1]);
			fill_buffer(pdt, ebuf[0]);
			break;

		case BASE_HEX :
			/* a or A conversion */
			if (PF_IS_SET(FLAG_UPPERCASE)) {
				fill_buffer(pdt, 'P');
			} else {
				fill_buffer(pdt, 'p');
			}

			fill_buffer(pdt, sign);

			/* exponent 1 digit */
			build_number(ebuf, 1, expnt, BASE_DEC, FLAG_NONE);
			fill_buffer(pdt, ebuf[0]);
			break;
	}
}


/*******************
 * convert_float() *
 ***********************************************************************
 DESCR
	Convert a floating point number

 IN
 	pdt :		vsnprintf common data structure
	value :		unsigned integer to convert

 OUT
	NONE

 TODO
 	-inf
 	-NaN
 ***********************************************************************/

static void convert_float(vsnp_t *pdt, flt_t value) {
	char	ibuf[MAXINTLEN+1],
			fbuf[MAXINTLEN+1],
			sign= ' ';
	flt_t	frac,
			uval;
	int		expnt = 0,
			t,
			explen = 0,
			fplen,
			fzplen,
			izplen,
			splen,
			have_dot = 0,
			have_sign = 0,
			have_exp = 0,
			have_g_conv,
			have_alt_form,
			skip_zero = 0;
	long	intval,
			fracval,
			lt;
	size_t	ilen,
			flen;

	have_g_conv = (int) (pdt->flags & FLAG_G_CONVERSION);
	have_alt_form = (int) (pdt->flags & FLAG_ALTERNATIVE_FORM);

	/* for g conversion without alternative form */
	if ((have_g_conv != 0) && (have_alt_form == 0)) {
		/* we skip ending zeros */
		skip_zero = 1;
	}

	if (value < 0) {
		/* on negative value set the sign */
		have_sign = 1;
		sign = '-';
		uval = -value;
	} else {
		/* else if + flag is set display the positive sign */
		if (PF_IS_SET(FLAG_SIGNED)) {
			have_sign = 1;
			sign = '+';
		}

		uval = value;
	}

	if ((PF_IS_SET(FLAG_EXPONENT)) || (have_g_conv != 0)) {
		/* process eventual exponent */
		have_exp = conv_to_exp(pdt, &uval, &expnt);
	}

	/* get integral part of the value */
	intval = (long) uval;

	/* build integral part number */
	ilen = build_number(ibuf, sizeof(ibuf), intval, pdt->base, pdt->flags);

	if (have_g_conv != 0) {
		pdt->prec = pdt->prec - ilen;
		if (pdt->prec < 0) {
			pdt->prec = 0;
		}
	}

	if (pdt->prec > 0) {
		/* get the fractional part */
		frac = uval - (flt_t) intval;

		/* shift as digits as precision specify */
		t = pdt->prec;

		/* init round limit */
		lt = 1;

		/* multiply successively by base to obtain the desired precision */
		while (t > 0) {
			frac = frac * pdt->base;
			lt = lt * pdt->base;
			t--;
		}

		/* get integral part of the result */
		fracval = (long) frac;

		/* rount value if needed */
		if ((frac - fracval) > 0.5) {
			fracval++;

			/* if the rounded value add a digit (eg 999 -> 1000) */
			if (fracval == lt) {
				/* then adjust fractional value */
				fracval = fracval / pdt->base;
			}
		}

		/* unset signed flag for fractional part */
		UNSET_PFLAG(FLAG_SIGNED);

		/* build fractional part number */
		flen = build_number(fbuf, sizeof(fbuf), fracval, pdt->base, pdt->flags);
	} else {
		flen = 0;
		fracval = 0;
	}

	/* need a dot ? */
	if ((have_alt_form != 0) || (pdt->prec > 0)) {
		have_dot = 1;
	}

	/* specific treatment for g conversion */
	if ((skip_zero != 0) && (fracval == 0)) {
		have_dot = 0;
		flen = 0;
		pdt->prec = 0;
	}

	/* fractal part len */
	if (flen < (size_t) pdt->prec) {
		fplen = pdt->prec;
		fzplen = pdt->prec - flen;
	} else {
		fplen = flen;
		fzplen = 0;
	}

	/* exponent len */
	if (have_exp != 0) {
		switch (pdt->base) {
			case BASE_DEC :
				explen = 4;
				break;

			case BASE_HEX :
				explen = 3;
				break;
		}
	} else {
		explen = 0;
	}

	/* total string len */
	t = ilen + fplen + explen;
	/* sign */
	if (have_sign != 0) {
		t++;
	}
	/* decimal point */
	if (have_dot != 0) {
		t++;
	}
	/* a or A conversion => hex prefix */
	if (pdt->base == BASE_HEX) {
		t= t + 2;
	}

	/* compute space or zero (integer part) padding */
	if (PF_IS_SET(FLAG_ZERO_PADDING)) {
		izplen = pdt->fwidth - t;
		if (izplen < 0) {
			izplen = 0;
		}
		splen = 0;
	} else {
		splen = pdt->fwidth - t;
		if (splen < 0) {
			splen = 0;
		}
		izplen = 0;
	}

	/* right justify space padding */
	if (PF_IS_UNSET(FLAG_LEFT_JUSTIFIED)) {
		while (splen > 0) {
			fill_buffer(pdt, ' ');
			splen--;
		}
	}

	/* if we have a sign */
	if (have_sign != 0) {
		fill_buffer(pdt, sign);
	}

	/* hex alternative form */
	if (pdt->base == BASE_HEX) {
		fill_buffer(pdt, '0');
		if (PF_IS_UNSET(FLAG_UPPERCASE)) {
			fill_buffer(pdt, 'x');
		} else {
			fill_buffer(pdt, 'X');
		}
	}

	/* zero padding */
	while (izplen > 0) {
		fill_buffer(pdt, '0');
		izplen--;
	}

	/* integral part */
	while (ilen > 0) {
		fill_buffer(pdt, ibuf[ilen - 1]);
		ilen--;
	}

	if (have_dot) {
		fill_buffer(pdt, '.');

		if (pdt->prec > 0) {
			/* fractional part */
			while (flen > 0) {
				fill_buffer(pdt, fbuf[flen - 1]);
				flen--;
			}

			if (skip_zero == 0) {
				/* zero padding */
				while (fzplen > 0) {
					fill_buffer(pdt, '0');
					fzplen--;
				}
			}
		}
	}

	if (have_exp != 0) {
		/* exponent part */
		print_exp(pdt, expnt);
	}
}


/*******************
 * pmk_vsnprintf() *
 ***********************************************************************
 DESCR
	String formatting function with fixed bufer size and va_list
	argument type

 IN
	buf :		buffer location
	len :		buffer size
	fmt :		format string
	args :		list of arguments

 OUT
	number of characters that would have been written if the buffer had
	not been size limited
 ***********************************************************************/

int pmk_vsnprintf(char *buf, size_t len, const char *fmt, va_list args) {
	char				*pstr,
						*pc = NULL;
	flt_t				 flt_val;
	int					*pi = NULL,
						 state = PARSE_CHAR,
						 modifier = 0,
						 n_modsave = MDFR_NORMAL,
						 have_n_conv = 0;
#ifdef HAVE_INTMAX_T
	intmax_t			*pimt = NULL;
#endif /* HAVE_INTMAX_T */
	long				*pl = NULL;
#ifdef HAVE_LONG_LONG
	long long			*pll = NULL;
#endif /* HAVE_LONG_LONG */
#ifdef HAVE_PTRDIFF_T
	ptrdiff_t			*ppd = NULL;
#endif /* HAVE_PTRDIFF_T */
	short				*ps = NULL;
	signed_t			 int_val;
	size_t				*pst = NULL;
	unsigned_t			 uint_val;
	void				*ptr;
	vsnp_t				 data;
#ifdef HAVE_WCHAR_T
	wchar_t				*wc;
#endif /* HAVE_WCHAR_T */
#ifdef HAVE_WINT_T
	wint_t				 wi;
#endif /* HAVE_WINT_T */

	/* init common data */
	data.buf = buf;
	data.len = len;
	data.cur = 0;

	/* loop until the end of the format string is reached */
	while (*fmt != '\0') {
		switch (state) {
			case PARSE_CHAR :
				/*
					check if we have a conversion sequence
				*/

#ifdef DEBUG_VSNPRINTF
printf("Enter state PARSE_CHAR\n");
#endif

				if (*fmt == '%') {
					data.flags = FLAG_NONE;
					data.fwidth = 0; /* -1 ? */
					data.prec = -1;
					modifier = MDFR_NORMAL;
					state = PARSE_FLAGS;
				} else {
					/* else copy the character in the buffer */
					fill_buffer(&data, *fmt);
				}

				/* next character */
				fmt++;
				break;

			case PARSE_FLAGS :
				/*
					parse the conversion flags
				*/

#ifdef DEBUG_VSNPRINTF
printf("Enter state PARSE_FLAGS\n");
#endif

				switch (*fmt) {
					case '+' :
						SET_FLAG(FLAG_SIGNED);
						fmt++;
						break;

					case '-' :
						SET_FLAG(FLAG_LEFT_JUSTIFIED);
						fmt++;
						break;

					case ' ' :
						SET_FLAG(FLAG_SPACE_PREFIX);
						fmt++;
						break;

					case '#' :
						SET_FLAG(FLAG_ALTERNATIVE_FORM);
						fmt++;
						break;

					case '0' :
						SET_FLAG(FLAG_ZERO_PADDING);
						fmt++;
						break;

					default :
						state = PARSE_FLD_WIDTH;
				}
				break;

			case PARSE_FLD_WIDTH :
				/*
					parse the field width
				*/

#ifdef DEBUG_VSNPRINTF
printf("Enter state PARSE_FLD_WIDTH\n");
#endif

				/* if we got an asterisk */
				if (*fmt == '*') {
					/* then get the field width from arguments */
					data.fwidth = va_arg(args, int);
					/* if field width is negative */
					if (data.fwidth < 0) {
						/* take absolute value for the width */
						data.fwidth = data.fwidth * (-1);
						/* and set left justify flag */
						SET_FLAG(FLAG_LEFT_JUSTIFIED);
					}
					fmt++;
				} else {
					/* else take the width from format string */
					while ((*fmt >= '0') && (*fmt <= '9')) {
						data.fwidth = (data.fwidth * 10) + (int) (*fmt - '0');
						fmt++;
					}
				}

				/* ignore 0 flag if - flag is provided */
				if (F_IS_SET(FLAG_LEFT_JUSTIFIED)) {
					UNSET_FLAG(FLAG_ZERO_PADDING);
				}

				state = PARSE_DOT;
				break;

			case PARSE_DOT :
				/*
					check if the dot of precision field is given
				*/

#ifdef DEBUG_VSNPRINTF
printf("Enter state PARSE_DOT\n");
#endif

				if (*fmt == '.') {
					/* if yes parse the precision field */
					fmt++;
					data.prec = 0;
					state = PARSE_PRECISION;
					/*flags = flags & (FLAG_ALL ^ FLAG_ZERO_PADDING);*//* XXX meant for something ??? */
				} else {
					/* else go parse the modifier */
					state = PARSE_LEN_MDFR;
				}
				break;

			case PARSE_PRECISION :
				/*
					parse precision field
				*/

				/* if we got an asterisk */
				if (*fmt == '*') {
					/* then get the precision from arguments */
					data.prec = va_arg(args, int);
					fmt++;
				} else {
					/* else take the precision from format string */
					while ((*fmt >= '0') && (*fmt <= '9')) {
						data.prec = (data.prec * 10) + (int) (*fmt - '0');
						fmt++;
					}
				}

				/* go parse the modifier */
				state = PARSE_LEN_MDFR;
				break;

			case PARSE_LEN_MDFR :
				/*
					parse modifier
				*/

#ifdef DEBUG_VSNPRINTF
printf("Enter state PARSE_LEN_MDFR\n");
#endif

				switch (*fmt) {
					case 'h' :
						switch (modifier) {
							case MDFR_NORMAL :
								modifier = MDFR_SHORT;
								fmt++;
								break;

							case MDFR_SHORT :
								modifier = MDFR_CHAR;
								state = PARSE_CONV_SPEC;
								fmt++;
								break;

							default :
								state = PARSE_CONV_SPEC;
						}
						break;

					case 'l' :
						switch (modifier) {
							case MDFR_NORMAL :
								modifier = MDFR_LONG;
								fmt++;
								break;

							case MDFR_LONG :
#if defined(HAVE_LONG_LONG) || defined(HAVE_UNSIGNED_LONG_LONG)
								modifier = MDFR_LONG_LONG;
#endif /* HAVE_LONG_LONG || HAVE_UNSIGNED_LONG_LONG */
								state = PARSE_CONV_SPEC;
								fmt++;
								break;
							default :
								state = PARSE_CONV_SPEC;
						}
						break;

					case 'j' :
						modifier = MDFR_INTMAX;
						state = PARSE_CONV_SPEC;
						fmt++;
						break;

					case 'z' :
						modifier = MDFR_SIZET;
						state = PARSE_CONV_SPEC;
						fmt++;
						break;

					case 't' :
						modifier = MDFR_PTRDIFF;
						state = PARSE_CONV_SPEC;
						fmt++;
						break;

					case 'L' :
						modifier = MDFR_LONG_DBL;
						state = PARSE_CONV_SPEC;
						fmt++;
						break;

					default :
						state = PARSE_CONV_SPEC;
				}
				break;

			case PARSE_CONV_SPEC :
				/*
					parse conversion specifier
				*/

#ifdef DEBUG_VSNPRINTF
printf("Enter state PARSE_CONV_SPEC\n");
#endif

				switch(*fmt) {
					case 'd' :
					case 'i' :
						/*
							signed decimal
						*/

						/* default precision */
						if (data.prec < 0) {
							data.prec = 1;
						}

						/* process modifier */
						switch (modifier) {
							case MDFR_LONG :
								int_val = (signed_t) va_arg(args, long);
								break;

#ifdef HAVE_LONG_LONG
							case MDFR_LONG_LONG :
								int_val = (signed_t) va_arg(args, long long);
								break;
#endif /* HAVE_LONG_LONG */

							case MDFR_SHORT :
							case MDFR_CHAR :
								/* short and char are promoted to int throught ... */
								int_val = (signed_t) va_arg(args, int);
								break;

#ifdef HAVE_INTMAX_T
							case MDFR_INTMAX :
								int_val = (signed_t) va_arg(args, intmax_t);
								break;
#endif /* HAVE_INTMAX_T */

							case MDFR_SIZET :
								int_val = (signed_t) va_arg(args, size_t);
								break;

#ifdef HAVE_PTRDIFF_T
							case MDFR_PTRDIFF :
								int_val = (signed_t) va_arg(args, ptrdiff_t);
								break;
#endif /* HAVE_PTRDIFF_T */
							default :
								int_val = (signed_t) va_arg(args, int);
						}

						data.base = 10;

						convert_sint(&data, int_val);
						break;

					case 'o' :
					case 'u' :
					case 'x' :
					case 'X' :
						/*
							unsigned conversion
						*/

						/* default precision */
						if (data.prec < 0) {
							data.prec = 1;
						}

						/* ignore - flag */
						UNSET_FLAG(FLAG_SIGNED);

						/* process modifier */
						switch (modifier) {
							case MDFR_LONG :
								uint_val = (unsigned_t) va_arg(args, unsigned long);
								break;

#ifdef HAVE_UNSIGNED_LONG_LONG
							case MDFR_LONG_LONG :
								uint_val = (unsigned_t) va_arg(args, unsigned long long);
								break;
#endif /* HAVE_UNSIGNED_LONG_LONG */

							case MDFR_SHORT :
							case MDFR_CHAR :
								/* short and char are promoted to int throught ... */
								uint_val = (unsigned_t) va_arg(args, unsigned int);
								break;

#ifdef HAVE_UINTMAX_T
							case MDFR_INTMAX :
								uint_val = (unsigned_t) va_arg(args, uintmax_t);
								break;
#endif /* HAVE_UINTMAX_T */

							case MDFR_SIZET :
								uint_val = (unsigned_t) va_arg(args, size_t);
								break;

#ifdef HAVE_PTRDIFF_T
							case MDFR_PTRDIFF :
								uint_val = (unsigned_t) va_arg(args, ptrdiff_t);
								break;
#endif /* HAVE_PTRDIFF_T */
							default :
								uint_val = (unsigned_t) va_arg(args, unsigned int);
						}

						/* set base */
						switch (*fmt) {
							case 'o' :
								data.base = BASE_OCT;
								break;

							case 'u' :
								data.base = BASE_DEC;
								break;

							case 'X' :
								SET_FLAG(FLAG_UPPERCASE);
								/* no break */

							case 'x' :
								data.base = BASE_HEX;
								break;
						}

						convert_int(&data, uint_val);
						break;

					case 'F' :
					case 'E' :
					case 'G' :
					case 'A' :
						/*
							float conversion (uppercase)
						*/

						SET_FLAG(FLAG_UPPERCASE);
						/* no break */

					case 'f' :
					case 'e' :
					case 'g' :
					case 'a' :
						/*
							float conversion (common)
						*/

						switch (*fmt) {
							case 'F' :
							case 'f' :
								data.base = BASE_DEC;

								/* default precision */
								if (data.prec < 0) {
									data.prec = 6;
								}
								break;

							case 'E' :
							case 'e' :
								data.base = BASE_DEC;

								SET_FLAG(FLAG_EXPONENT);

								/* default precision */
								if (data.prec < 0) {
									data.prec = 6;
								}
								break;

							case 'G' :
							case 'g' :
								data.base = BASE_DEC;

								SET_FLAG(FLAG_G_CONVERSION);

								/* default precision */
								if (data.prec <= 0) {
									data.prec = 6; /* XXX ISO C99 = 1 */
								}
								break;

							case 'A' :
							case 'a' :
								data.base = BASE_HEX;

								SET_FLAG(FLAG_EXPONENT);

								/* default precision */
								if (data.prec < 0) {
									data.prec = 4; /* XXX real default value is ? */
								}
								break;
						}

#ifdef HAVE_LONG_DOUBLE
						if (modifier = MDFR_LONG_DBL) {
							flt_val = (flt_t) va_arg(args, long double);
						} else {
#endif /* HAVE_LONG_DOUBLE */
							flt_val = (flt_t) va_arg(args, double);
#ifdef HAVE_LONG_DOUBLE
						}
#endif /* HAVE_LONG_DOUBLE */

						convert_float(&data, flt_val);
						break;

					case 'c' :
						/*
							character conversion
						*/

#ifdef HAVE_WINT_T
						if (modifier == MDFR_LONG) {
							/* wide char */
							wi = va_arg(args, wint_t);

							/* XXX TODO wide char support */
						} else {
#endif /* HAVE_WINT_T */
							/* char is promoted to int throught ... */
							int_val = (signed_t) va_arg(args, int);

#ifdef HAVE_WINT_T
							fill_buffer(&data, (char) int_val);
						}
#endif /* HAVE_WINT_T */

						break;

					case 's' :
						/*
							string conversion
						*/

#ifdef HAVE_WCHAR_T
						if (modifier == MDFR_LONG) {
							/* wide char */
							wc = va_arg(args, wchar_t *);

							/* XXX TODO wide char support */
						} else {
#endif /* HAVE_WCHAR_T */
							pstr = va_arg(args, char *);

							if (data.prec <= 0) {
								data.prec = len;
							}

							convert_str(&data, pstr);
#ifdef HAVE_WCHAR_T
						}
#endif /* HAVE_WCHAR_T */

						break;

					case 'p' :
						/*
							pointer conversion
						*/
						ptr = va_arg(args, void *);

						data.base = BASE_HEX;

/* XXX ?
						SET_FLAG(FLAG_ALTERNATIVE_FORM);
*/

						convert_int(&data, (unsigned_t) ptr);
						break;

					case 'n' :
						/*
							pointer to the variable that will receive string size
						*/

						/* set flag */
						have_n_conv = 1;

						/* save modifier */
						n_modsave = modifier;

						/* process modifier */
						switch (modifier) {
							case MDFR_LONG :
								pl = va_arg(args, long *);
								break;

#ifdef HAVE_LONG_LONG
							case MDFR_LONG_LONG :
								pll = va_arg(args, long long *);
								break;
#endif /* HAVE_LONG_LONG */

							case MDFR_SHORT :
								ps = va_arg(args, short *);
								break;

							case MDFR_CHAR :
								pc = va_arg(args, char *);
								break;

#ifdef HAVE_INTMAX_T
							case MDFR_INTMAX :
								pimt = va_arg(args, intmax_t *);
								break;
#endif /* HAVE_INTMAX_T */

							case MDFR_SIZET :
								pst = va_arg(args, size_t *);
								break;

#ifdef HAVE_PTRDIFF_T
							case MDFR_PTRDIFF :
								ppd = va_arg(args, ptrdiff_t *);
								break;
#endif /* HAVE_PTRDIFF_T */
							default :
								pi = va_arg(args, int *);
						}
						break;

					case '%' :
						/*
							"escaped" conversion character
						*/

						/* XXX we allow complete specification, != ISO C99 ? */

						fill_buffer(&data, *fmt);
						break;
				}

				/* next char */
				fmt++;

				state = PARSE_CHAR;
				break;
		}
	}

	/* NULL terminate the string */
	if (len > data.cur) {
		buf[data.cur] = '\0';
	} else {
		buf[len - 1] = '\0';
	}

	/* if %n provided, write lenght into the pointed area */
	if (have_n_conv != 0) {
		/* process modifier */
		switch (n_modsave) {
			case MDFR_LONG :
				*pl = (long) data.cur;
				break;

#ifdef HAVE_LONG_LONG
			case MDFR_LONG_LONG :
				*pll = (long long) data.cur;
				break;
#endif /* HAVE_LONG_LONG */

			case MDFR_SHORT :
				*ps = (short) data.cur;
				break;

			case MDFR_CHAR :
				*pc = (char) data.cur;
				break;

#ifdef HAVE_INTMAX_T
			case MDFR_INTMAX :
				*pimt = (intmax_t) data.cur;
				break;
#endif /* HAVE_INTMAX_T */

			case MDFR_SIZET :
				*pst = data.cur;
				break;

#ifdef HAVE_PTRDIFF_T
			case MDFR_PTRDIFF :
				*ppd = (ptrdiff_t) data.cur;
				break;
#endif /* HAVE_PTRDIFF_T */
			default :
				*pi = (int) data.cur;
		}
	}

	/* return what should have been the string lenght if not limited */
	return(data.cur);
}


/*****************
 * pmk_strlcpy() *
 ***********************************************************************
 DESCR
	This function duplicate a string.

 IN
	ostr :	char pointer of source string

 OUT
	pointer to duplicated string or NULL
 ***********************************************************************/

char *pmk_strdup(const char *ostr) {
	char	*pstr;
	size_t	 len;

	len = strlen(ostr) + 1;

	pstr = (char *) malloc(len);
	if (pstr != NULL) {
		memcpy(pstr, ostr, len);
	}

	return(pstr);
}


/*****************
 * pmk_strlcpy() *
 ***********************************************************************
 DESCR
	This function copy a given number of characters from a source
	string to a destination buffer. It also grants that this buffer
	will be null terminated.

 IN
	dst :	char pointer of destination buffer
	src :	char pointer of source string
	s :		number of characters to copy

 OUT
	size of the source string
 ***********************************************************************/

size_t pmk_strlcpy(char *dst, const char *src, size_t s) {
	size_t	len = 0;

	/* loop until we reach the end of the src string */
	while (*src != '\0') {
		/* if buffer is not full */
		if (s > 0) {
			if (s == 1) {
				/* last character, null terminate */
				*dst = '\0';
			} else {
				/* copy character */
				*dst = *src;
			}

			/* adjust remaining size */
			s--;

			/* update src string length */
			len++;
			/* and dst pointer */
			dst++;
		}

		/* increment src pointer */
		src++;
	}

	/* if the end of the buffer has not been reached */
	if (s > 0) {
		/* last character, null terminate */
		*dst = '\0';
	}

	return(len);
}


/*****************
 * pmk_strlcat() *
 ***********************************************************************
 DESCR
	This function append a given number of characters from a source
	string to the end of a destination buffer. It also grants that
	this buffer will be null terminated.

 IN
	dst :	char pointer of destination buffer
	src :	char pointer of source string
	s :		number of characters to copy

 OUT
	size of the source string
 ***********************************************************************/

size_t pmk_strlcat(char *dst, const char *src, size_t s) {
	size_t	len;

	/* get size of actual string */
	len = strlen(dst);

	/* start right after the last character */
	dst = dst + len;

	/* update size */
	s = s -len;

	/* update len with the result of the string copy */
	len = len + pmk_strlcpy(dst, src, s);

	return(len);
}


/***************************
 * find_parent_separator() *
 ***********************************************************************
 DESCR
	try to find the last separator between directory name and file name

 IN
	path :	path string
	psep :	separator location
	plen :	path string len

 OUT
	NONE
 ***********************************************************************/

static void find_parent_separator(char *path, char **psep, size_t *plen) {
	char		*psave = NULL;
	int			 have_sep = 0;

	/* initialize length */
	*plen = 0;

	/* initialize separator position */
	*psep = NULL;

	while (*path != '\0') {
		if (*path == '/') {
			/* set flag */
			have_sep = 1;

			/* save position */
			psave = path;
		} else {
			if (have_sep != 0) {
				/* had a separator before -> save last position */
				*psep = psave;
			}
		}

		/* next character */
		path++;
		(*plen)++;
	}
}


/*****************
 * pmk_dirname() *
 ***********************************************************************
 DESCR
	XXX

 IN
	XXX

 OUT
	XXX
 ***********************************************************************/

char *pmk_dirname(char *path) {
	static char	 buffer[MAXPATHLEN];
	char		*pstr,
				*psep;
	size_t		 len;

	/* start at the begining of the buffer */
	pstr = buffer;

	if ((path == NULL) || (*path == '\0')) {
		*pstr = '.';
		pstr++;
	} else {
		/* look for base/dir separator */
		find_parent_separator(path, &psep, &len);

		if (psep == NULL) {
			/* no valid separator found */
			if (*path == '/') {
				*pstr = '/';
			} else {
				*pstr = '.';
			}
			pstr++;
		} else {
			while ((psep > path) && (*psep == '/')) {
				/* skip trailing slashes */
				psep--;
			}

			/* take care of buffer overflow */
			if ((size_t) (psep - path) > sizeof(buffer)) {
				return(NULL);
			}

			/* copy characters until separator position */
			while (path <= psep) {
				*pstr = *path;
				pstr++;
				path++;
			}
		}
	}

	/* NULL terminate the buffer */
	*pstr = '\0';

	return(buffer);
}


/******************
 * pmk_basename() *
 ***********************************************************************
 DESCR
	XXX

 IN
	XXX

 OUT
	XXX
 ***********************************************************************/

char *pmk_basename(char *path) {
	static char	 buffer[MAXPATHLEN];
	char		*pstr,
				*psep,
				*pbeg,
				*pend;
	size_t		 len;

	/* start at the begining of the buffer */
	pstr = buffer;

	/* if path is a null pointer or empty string */
	if ((path == NULL) || (*path == '\0')) {
		*pstr = '.';
		pstr++;
	} else {
		/* else look for base/dir separator */
		find_parent_separator(path, &psep, &len);

		/* if no valid separator is found */
		if (psep == NULL) {
			/* return the whole string */
			pbeg = path;
		} else {
			/* else return the string starting right after separator */
			pbeg = psep + 1;
		}

		/* compute end of string pointer */
		pend = path + len - 1;

		while ((pend > pbeg) && (*pend == '/')) {
			/* skip trailing slashes */
			pend--;
		}

		/* take care of buffer overflow */
		if ((size_t) (pend - pbeg) > sizeof(buffer)) {
			return(NULL);
		}

		/* copy characters until separator position */
		while (pbeg <= pend) {
			*pstr = *pbeg;
			pstr++;
			pbeg++;
		}
	}

	/* NULL terminate the buffer */
	*pstr = '\0';

	return(buffer);
}


/*****************
 * pmk_isblank() *
 ***********************************************************************
 DESCR
	XXX

 IN
	XXX

 OUT
	XXX
 ***********************************************************************/

int pmk_isblank(int c) {
	if (c == ' ' || c == '\t')
		return (1);
	else
		return(0);
}


/******************
 * pmk_mkstemps() *
 ***********************************************************************
 DESCR
	XXX

 IN
	XXX

 OUT
	XXX
 ***********************************************************************/

int pmk_mkstemps(char *template, int suffixlen) {
	struct timeval	 tv;
	char			 subst[] = "aZbY0cXdWe1VfUgT2hSiRj3QkPlO4mNnMo5LpKqJ6rIsHt7GuFvE8wDxCy9BzA",
					*start,
					*end,
					*p;
	int				 fd,
					 len,
					 i;

	/* forwarding to end of template */
	for (p = template ; *p != '\0' ; p++);

	/* increment len to also count end of file character */
	suffixlen++;

	/* compute (supposed) position of last character to replace */
	p = p - suffixlen;

	/* check it baby ;) */
	if (p < template)
		return(-1);

	/* set last character position */
	end = p;

	/* step back until we found the starting character */
	for ( ; *p == MKSTEMPS_REPLACE_CHAR && p > template; p--);

	/* set fisrt character position */
	start = ++p;

	/* intialise random() */
	len = strlen(subst);
	gettimeofday(&tv, NULL);
	srandom(tv.tv_sec * tv.tv_usec);

	/* lets go replacing the stuff */
	for (p = start ; p <= end ; p++) {
		/* get random value */
		i = (int) random() % len;
		/* replace */
		*p = subst[i];
	}

	/* open file */
	fd = open(template, O_CREAT|O_EXCL|O_RDWR, S_IRUSR | S_IWUSR);

	return(fd);
}


/************************
 *	compatibility stuff *
 ***********************************************************************/

/********************
 *	pmk_stdio.h	*
 ********************/

#ifndef HAVE_VSNPRINTF
int vsnprintf(char *buf, size_t len, const char *fmt, va_list args) {
	return(pmk_vsnprintf(buf, len, fmt, args));
}

#endif /* HAVE_VSNPRINTF */


#ifndef HAVE_SNPRINTF
int snprintf(char *str, size_t size, const char *format, ...) {
	int		rslt;
	va_list	args;

	va_start(args, format);
	rslt = vsnprintf(str, size, format, args);
	va_end(args);

	return(rslt);
}
#endif /* HAVE_SNPRINTF */


/********************
 *	pmk_string.h	*
 ********************/

#ifndef HAVE_STRDUP
char *strdup(const char *str) {
	return(pmk_strdup(str));
}
#endif /* HAVE_STRDUP */


#ifndef HAVE_STRLCAT
size_t strlcat(char *dst, const char *src, size_t s) {
	return(pmk_strlcat(dst, src, s));
}
#endif /* HAVE_STRLCAT */


#ifndef HAVE_STRLCPY
size_t strlcpy(char *dst, const char *src, size_t s) {
	return(pmk_strlcpy(dst, src, s));
}
#endif /* HAVE_STRLCPY */


/********************
 *	pmk_libgen.h	*
 ********************/

#ifndef HAVE_LIBGEN_H
char *dirname(char *path) {
	return(pmk_dirname(path));
}

char *basename(char *path) {
	return(pmk_basename(path));
}
#endif /* HAVE_LIBGEN_H */


/****************
 *	pmk_ctype.h	*
 ****************/

#ifndef HAVE_ISBLANK
int isblank(int c) {
	return(pmk_isblank(c));
}
#endif /* HAVE_ISBLANK */


/********************
 *	pmk_unistd.h	*
 ********************/

#ifndef HAVE_MKSTEMPS
int mkstemps(char *template, int suffixlen) {
	return(pmk_mkstemps(template, suffixlen));
}
#endif /* HAVE_MKSTEMPS */


/********************************
 *	boolean string functions	*
 ***********************************************************************/

/****************
 * snprintf_b() *
 ***********************************************************************
 DESCR
	boolean snprintf

 IN
	buf :	buffer location
	siz :	buffer size
	fmt :	format string
	... :	argument list

 OUT
	boolean value
 ***********************************************************************/

bool snprintf_b(char *buf, size_t siz, const char *fmt, ...) {
	bool	rslt;
	va_list	ap;

	va_start(ap, fmt);

	if (vsnprintf(buf, siz, fmt, ap) >= (int) siz)
		rslt = false;
	else
		rslt = true;

	va_end(ap);

	return(rslt);
}


/***************
 * strlcat_b() *
 ***********************************************************************
 DESCR
	boolean strlcat

 IN
	dst :	destination buffer
	src :	source string
	siz :	size of destination buffer

 OUT
	boolean
 ***********************************************************************/

bool strlcat_b(char *dst, const char *src, size_t siz) {
	if (strlcat(dst, src, siz) >= siz)
		return(false);
	else
		return(true);
}


/***************
 * strlcpy_b() *
 ***********************************************************************
 DESCR
	boolean strlcpy

 IN
	dst :	destination buffer
	src :	source string
	siz :	size of destination buffer

 OUT
	boolean
 ***********************************************************************/

bool strlcpy_b(char *dst, const char *src, size_t siz) {
	if (strlcpy(dst, src, siz) >= siz)
		return(false);
	else
		return(true);
}
