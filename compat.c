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
	buffer :	buffer location
	len :		buffer size
	cursor		cursor position
	chr :		char to append

 OUT
	NONE
 ***********************************************************************/

static void fill_buffer(char *buffer, size_t len, size_t *cursor, char chr) {
#ifdef DEBUG_VSNPRINTF
printf("cursor = %d, char = '%c'\n", *cursor, chr);
#endif
	/* if buffer is not full */
	if (*cursor < len) {
		/* insert character */
		buffer[*cursor] = chr;
	}

	/* increment cursor position */
	(*cursor)++;
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
	base :		base used for conversion
	flags :		used to know the case for conversion

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
	buffer :	buffer location
	len :		buffer size
	cur :		cursor position
	value :		unsigned integer to convert
	base :		base used for conversion
	fwidth :	field width
	prec :		precision
	flags :		used to know the case for conversion
	isneg :		negative value flag

 OUT
	NONE
 ***********************************************************************/

static void convert_int(char *buf, size_t len, size_t *cur,
							unsigned_t value, base_t base,
							int fwidth,	int prec, flag_t flags, int isneg) {
	char	nbuf[MAXINTLEN+1],
			sign = ' ';
	int		vlen,
			zplen,
			splen,
			have_sign = 0,
			have_hex_prefix = 0;

	if (isneg != 0) {
		/* on negative value set the sign */
		have_sign = 1;
		sign = '-';
	} else {
		/* else if + flag is set display the positive sign */
		if ((flags & FLAG_SIGNED) != 0) {
			have_sign = 1;
			sign = '+';
		}
	}

	vlen = build_number(nbuf, sizeof(nbuf), value, base, flags);

	/* compute zero padding */
	zplen = prec - vlen;
	if (zplen < 0) {
		zplen = 0;
	}

	/* compute space padding */
	if (prec > vlen) {
		splen = fwidth - prec;
	} else {
		splen = fwidth - vlen;
	}
	if (have_sign != 0) {
		splen--;
	}
	if ((flags & FLAG_ALTERNATIVE_FORM) != 0) {
		switch (base) {
			case BASE_OCT :
				/* octal alternative form */
				if ((value == 0) && (prec == 0)) {
					prec = 1;
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
	if ((flags & FLAG_LEFT_JUSTIFIED) == 0) {
		while (splen > 0) {
			fill_buffer(buf, len, cur, ' ');
			splen--;
		}
	}

	/* if we have a sign */
	if (have_sign != 0) {
		fill_buffer(buf, len, cur, sign);
	}

	/* hex alternative form */
	if (have_hex_prefix != 0) {
		fill_buffer(buf, len, cur, '0');
		if ((flags & FLAG_UPPERCASE) == 0) {
			fill_buffer(buf, len, cur, 'x');
		} else {
			fill_buffer(buf, len, cur, 'X');
		}
	}

	/* zero padding */
	while (zplen > 0) {
		fill_buffer(buf, len, cur, '0');
		zplen--;
	}

	/* converted integer */
	while (vlen > 0) {
		fill_buffer(buf, len, cur, nbuf[vlen - 1]);
		vlen--;
	}

	/* left justify space padding */
	if ((flags & FLAG_LEFT_JUSTIFIED) != 0) {
		while (splen > 0) {
			fill_buffer(buf, len, cur, ' ');
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
	buffer :	buffer location
	len :		buffer size
	cur :		cursor position
	value :		unsigned integer to convert
	base :		base used for conversion
	fwidth :	field width
	prec :		precision
	flags :		used to know the case for conversion

 OUT
	NONE
 ***********************************************************************/

static void convert_sint(char *buf, size_t len, size_t *cur, signed_t value,
						base_t base, int fwidth, int prec, flag_t flags) {
	int	isneg = 0;

	/* check if value is negative */
	if (value < 0) {
		/* if true set flag */
		isneg = 1;
		value = -value;
	}

	/* call generic conversion procedure */
	convert_int(buf, len, cur, (unsigned_t) value, base, fwidth, prec,
														flags, isneg);
}


/******************
 * convert_uint() *
 ***********************************************************************
 DESCR
	Convert an unsigned integer

 IN
	buffer :	buffer location
	len :		buffer size
	cur :		cursor position
	value :		unsigned integer to convert
	base :		base used for conversion
	fwidth :	field width
	prec :		precision
	flags :		used to know the case for conversion

 OUT
	NONE
 ***********************************************************************/

static void convert_uint(char *buf, size_t len, size_t *cur, unsigned_t value,
						base_t base, int fwidth, int prec, flag_t flags) {

	/* call generic conversion procedure */
	convert_int(buf, len, cur, value, base, fwidth, prec, flags, 0);
}


/*****************
 * convert_str() *
 ***********************************************************************
 DESCR
	Convert the string. It means copying the string in the place of the
	conversion identifier.

 IN
	buffer :	buffer location
	len :		buffer size
	cur :		cursor position
	ptr :		string to convert
	prec :		precision AKA max number of characters to copy
	flags :		used to know the case for conversion

 OUT
	NONE
 ***********************************************************************/

static void convert_str(char *buf, size_t len, size_t *cur, char *ptr,
											int prec, flag_t flags) {
	while ((*cur < len) && (prec > 0) && (*ptr != '\0')) {
		fill_buffer(buf, len, cur, *ptr);
		ptr++;
		prec--;
	}
}


/*******************
 * convert_float() *
 ***********************************************************************
 DESCR
	Convert a floating point number

 IN
	buffer :	buffer location
	len :		buffer size
	cur :		cursor position
	value :		unsigned integer to convert
	base :		base used for conversion
	fwidth :	field width
	prec :		precision
	flags :		used to know the case for conversion

 OUT
	NONE

 TODO
 	-inf
 	-NaN
 ***********************************************************************/

static void convert_float(char *buf, size_t len, size_t *cur, float_t value,
						base_t base, int fwidth, int prec, flag_t flags) {
	char	ibuf[MAXINTLEN+1],
			fbuf[MAXINTLEN+1],
			sign= ' ';
	float_t	uval,
			frac;
	int		expnt = 0,
			t,
			explen = 0,
			fplen,
			fzplen,
			izplen,
			splen,
			have_sign = 0,
			have_dot = 0,
			have_exp = 0,
			have_g_conv,
			have_alt_form,
			skip_zero = 0;
	long	intval,
			fracval,
			lt;
	size_t	ilen,
			flen;

	have_g_conv = (int) (flags & FLAG_G_CONVERSION);
	have_alt_form = (int) (flags & FLAG_ALTERNATIVE_FORM);

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
		if ((flags & FLAG_SIGNED) != 0) {
			have_sign = 1;
			sign = '+';
		}

		uval = value;
	}

	/* process eventual exponent */
	if (((flags & FLAG_EXPONENT) != 0) || (have_g_conv != 0)) {
		/* save value in frac for g conversion */
		frac = uval;

		/* set exponent flag */
		have_exp = 1;
		expnt = 0;

		/* process value to get only one digit before decimal point */
		if (uval >= 0) {
			while (uval > base) {
				uval = uval / base;
				expnt++;
			}
		} else {
			while (uval < 0) {
				uval = uval * base;
				expnt--;
			}
		}

		/* check if exponent is needed for g conversion */
		if (have_g_conv != 0) {
			if ((expnt >= -4) && (expnt < prec)) {
				/* unset exponent flag */
				have_exp = 0;

				/* get back to saved value */
				uval = frac;
			}
		}
	}

	/* get integral part of the value */
	intval = (long) uval;

	/* build integral part number */
	ilen = build_number(ibuf, sizeof(ibuf), intval, base, flags);

	if (have_g_conv != 0) {
		prec = prec - ilen;
		if (prec < 0) {
			prec = 0;
		}
	}

	if (prec > 0) {
		/* get the fractional part */
		frac = uval - (float_t) intval;

		/* shift as digits as precision specify */
		t = prec;

		/* init round limit */
		lt = 1;

		/* multiply successively by base to obtain the desired precision */
		while (t > 0) {
			frac = frac * base;
			lt = lt * base;
			t--;
		}

		/* get integral part */
		fracval = (long) frac;

		/* rount value if needed */
		if ((frac - fracval) > 0.5) {
			fracval++;

			/* if the rounded value add a digit (eg 999 -> 1000) */
			if (fracval == lt) {
				/* then adjust fractional value */
				fracval = fracval / base;
			}
		}

		/* unset signed flag for fractional part */
		flags = flags & (FLAG_ALL ^ FLAG_SIGNED);

		/* build fractional part number */
		flen = build_number(fbuf, sizeof(fbuf), fracval, base, flags);
	} else {
		flen = 0;
		fracval = 0;
	}

	/* need a dot ? */
	if ((have_alt_form != 0) || (prec > 0)) {
		have_dot = 1;
	}

	/* specific treatment for g conversion */
	if ((skip_zero != 0) && (fracval == 0)) {
		have_dot = 0;
		flen = 0;
		prec = 0;
	}

	/* fractal part len */
	if (flen < (size_t) prec) {
		fplen = prec;
		fzplen = prec - flen;
	} else {
		fplen = flen;
		fzplen = 0;
	}

	/* exponent len */
	if (have_exp != 0) {
		switch (base) {
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
	if (base == BASE_HEX) {
		t= t + 2;
	}

	/* compute space or zero (integer part) padding */
	if ((flags & FLAG_ZERO_PADDING) != 0) {
		izplen = fwidth - t;
		if (izplen < 0) {
			izplen = 0;
		}
		splen = 0;
	} else {
		splen = fwidth - t;
		if (splen < 0) {
			splen = 0;
		}
		izplen = 0;
	}

	/* right justify space padding */
	if ((flags & FLAG_LEFT_JUSTIFIED) == 0) {
		while (splen > 0) {
			fill_buffer(buf, len, cur, ' ');
			splen--;
		}
	}

	/* if we have a sign */
	if (have_sign != 0) {
		fill_buffer(buf, len, cur, sign);
	}

	/* hex alternative form */
	if (base == BASE_HEX) {
		fill_buffer(buf, len, cur, '0');
		if ((flags & FLAG_UPPERCASE) == 0) {
			fill_buffer(buf, len, cur, 'x');
		} else {
			fill_buffer(buf, len, cur, 'X');
		}
	}

	/* zero padding */
	while (izplen > 0) {
		fill_buffer(buf, len, cur, '0');
		izplen--;
	}

	/* integral part */
	while (ilen > 0) {
		fill_buffer(buf, len, cur, ibuf[ilen - 1]);
		ilen--;
	}

	if (have_dot) {
		fill_buffer(buf, len, cur, '.');

		if (prec > 0) {
			/* fractional part */
			while (flen > 0) {
				fill_buffer(buf, len, cur, fbuf[flen - 1]);
				flen--;
			}

			if (skip_zero == 0) {
				/* zero padding */
				while (fzplen > 0) {
					fill_buffer(buf, len, cur, '0');
					fzplen--;
				}
			}
		}
	}

	if (have_exp != 0) {
		/* exponent part */

		/* exponenet sign */
		if (expnt < 0) {
			sign = '-';
		} else {
			sign = '+';
		}

		/* exponent type may vary */
		ibuf[1] = '0'; /* init */
		switch (base) {
			case BASE_DEC :
				/* g or G conversion */
				if ((flags & FLAG_UPPERCASE) != 0) {
					fill_buffer(buf, len, cur, 'E');
				} else {
					fill_buffer(buf, len, cur, 'e');
				}

				fill_buffer(buf, len, cur, sign);

				/* exponent 2 digits */
				build_number(ibuf, 2, expnt, BASE_DEC, FLAG_NONE);
				fill_buffer(buf, len, cur, ibuf[1]);
				fill_buffer(buf, len, cur, ibuf[0]);
				break;

			case BASE_HEX :
				/* a or A conversion */
				if ((flags & FLAG_UPPERCASE) != 0) {
					fill_buffer(buf, len, cur, 'P');
				} else {
					fill_buffer(buf, len, cur, 'p');
				}

				fill_buffer(buf, len, cur, sign);

				/* exponent 1 digit */
				build_number(ibuf, 1, expnt, BASE_DEC, FLAG_NONE);
				fill_buffer(buf, len, cur, ibuf[0]);
				break;
		}
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
	base_t				 base = 0;
	char				*pstr,
						*pc = NULL;
	flag_t				 flags = FLAG_NONE;
	float_t				 flt_val;
	int					*pi = NULL,
						 state = PARSE_CHAR,
						 fwidth = 0,
						 precision = 0,
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
	size_t				 cursor = 0,
						*pst = NULL;
	unsigned_t			 uint_val;
	void				*ptr;
#ifdef HAVE_WCHAR_T
	wchar_t				*wc;
#endif /* HAVE_WCHAR_T */
#ifdef HAVE_WINT_T
	wint_t				 wi;
#endif /* HAVE_WINT_T */


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
					flags = FLAG_NONE;
					fwidth = 0; /* -1 ? */
					precision = -1;
					modifier = MDFR_NORMAL;
					state = PARSE_FLAGS;
				} else {
					/* else copy the character in the buffer */
					fill_buffer(buf, len, &cursor, *fmt);
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
						flags = flags | FLAG_SIGNED;
						fmt++;
						break;

					case '-' :
						flags = flags | FLAG_LEFT_JUSTIFIED;
						fmt++;
						break;

					case ' ' :
						flags = flags | FLAG_SPACE_PREFIX;
						fmt++;
						break;

					case '#' :
						flags = flags | FLAG_ALTERNATIVE_FORM;
						fmt++;
						break;

					case '0' :
						flags = flags | FLAG_ZERO_PADDING;
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
					fwidth = va_arg(args, int);
					/* if field width is negative */
					if (fwidth < 0) {
						/* take absolute value for the width */
						fwidth = fwidth * (-1);
						/* and set left justify flag */
						flags = flags | FLAG_LEFT_JUSTIFIED;
					}
					fmt++;
				} else {
					/* else take the width from format string */
					while ((*fmt >= '0') && (*fmt <= '9')) {
						fwidth = (fwidth * 10) + (int) (*fmt - '0');
						fmt++;
					}
				}

				/* ignore 0 flag if - flag is provided */
				if ((flags & FLAG_LEFT_JUSTIFIED) != 0) {
					flags = flags & (FLAG_ALL ^ FLAG_ZERO_PADDING);
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
					precision = 0;
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
					precision = va_arg(args, int);
					fmt++;
				} else {
					/* else take the precision from format string */
					while ((*fmt >= '0') && (*fmt <= '9')) {
						precision = (precision * 10) + (int) (*fmt - '0');
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
						if (precision < 0) {
							precision = 1;
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

						base = 10;

						convert_sint(buf, len, &cursor, int_val, base, fwidth, precision, flags);
						break;

					case 'o' :
					case 'u' :
					case 'x' :
					case 'X' :
						/*
							unsigned conversion
						*/

						/* default precision */
						if (precision < 0) {
							precision = 1;
						}

						/* ignore - flag */
						flags = flags & (FLAG_ALL ^ FLAG_SIGNED);

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
								base = BASE_OCT;
								break;

							case 'u' :
								base = BASE_DEC;
								break;

							case 'X' :
								flags = flags | FLAG_UPPERCASE;
								/* no break */

							case 'x' :
								base = BASE_HEX;
								break;
						}

						convert_uint(buf, len, &cursor, uint_val, base, fwidth, precision, flags);
						break;

					case 'F' :
					case 'E' :
					case 'G' :
					case 'A' :
						/*
							float conversion (uppercase)
						*/

						flags = flags | FLAG_UPPERCASE;
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
								base = BASE_DEC;

								/* default precision */
								if (precision < 0) {
									precision = 6;
								}
								break;

							case 'E' :
							case 'e' :
								base = BASE_DEC;

								flags = flags | FLAG_EXPONENT;

								/* default precision */
								if (precision < 0) {
									precision = 6;
								}
								break;

							case 'G' :
							case 'g' :
								base = BASE_DEC;

								flags = flags | FLAG_G_CONVERSION;

								/* default precision */
								if (precision <= 0) {
									precision = 6; /* XXX ISO C99 = 1 */
								}
								break;

							case 'A' :
							case 'a' :
								base = BASE_HEX;

								flags = flags | FLAG_EXPONENT;

								/* default precision */
								if (precision < 0) {
									precision = 4; /* XXX real default value is ? */
								}
								break;
						}

#ifdef HAVE_LONG_DOUBLE
						if (modifier = MDFR_LONG_DBL) {
							flt_val = (float_t) va_arg(args, long double);
						} else {
#endif /* HAVE_LONG_DOUBLE */
							flt_val = (float_t) va_arg(args, double);
#ifdef HAVE_LONG_DOUBLE
						}
#endif /* HAVE_LONG_DOUBLE */

						convert_float(buf, len, &cursor, flt_val, base, fwidth, precision, flags);
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
							fill_buffer(buf, len, &cursor, (char) int_val);
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

							if (precision <= 0) {
								precision = len;
							}

							convert_str(buf, len, &cursor, pstr, precision, flags);
#ifdef HAVE_WCHAR_T
						}
#endif /* HAVE_WCHAR_T */

						break;

					case 'p' :
						/*
							pointer conversion
						*/
						ptr = va_arg(args, void *);

						base = BASE_HEX;

/* XXX ?
						flags = flags | FLAG_ALTERNATIVE_FORM;
*/

						convert_uint(buf, len, &cursor, (unsigned_t) ptr, base, fwidth, precision, flags);
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

						fill_buffer(buf, len, &cursor, *fmt);
						break;
				}

				/* next char */
				fmt++;

				state = PARSE_CHAR;
				break;
		}
	}

	/* NULL terminate the string */
	if (len > cursor) {
		buf[cursor] = '\0';
	} else {
		buf[len - 1] = '\0';
	}

	/* if %n provided, write lenght into the pointed area */
	if (have_n_conv != 0) {
		/* process modifier */
		switch (n_modsave) {
			case MDFR_LONG :
				*pl = (long) cursor;
				break;

#ifdef HAVE_LONG_LONG
			case MDFR_LONG_LONG :
				*pll = (long long) cursor;
				break;
#endif /* HAVE_LONG_LONG */

			case MDFR_SHORT :
				*ps = (short) cursor;
				break;

			case MDFR_CHAR :
				*pc = (char) cursor;
				break;

#ifdef HAVE_INTMAX_T
			case MDFR_INTMAX :
				*pimt = (intmax_t) cursor;
				break;
#endif /* HAVE_INTMAX_T */

			case MDFR_SIZET :
				*pst = cursor;
				break;

#ifdef HAVE_PTRDIFF_T
			case MDFR_PTRDIFF :
				*ppd = (ptrdiff_t) cursor;
				break;
#endif /* HAVE_PTRDIFF_T */
			default :
				*pi = (int) cursor;
		}
	}

	/* return what should have been the string lenght if not limited */
	return(cursor);
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
 *	pmk_string.h	*
 ********************/

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
