#pragma once



double alifOS_stringToDouble(const char*, char**, AlifObject*); // 9





char* alifOS_doubleToString(double, char, AlifIntT, AlifIntT, AlifIntT*); // 15




 // 22
#define ALIF_DTSF_SIGN      0x01 /* always add the sign */
#define ALIF_DTSF_ADD_DOT_0 0x02 /* if the result is an integer add ".0" */
#define ALIF_DTSF_ALT       0x04 /* "alternate" formatting. it's format_code
								  specific */
#define ALIF_DTSF_NO_NEG_0  0x08 /* negative zero result is coerced to 0 */

								  /* alifOS_doubleToString's "type", if non-NULL, will be set to one of: */
#define ALIF_DTST_FINITE 0
#define ALIF_DTST_INFINITE 1
#define ALIF_DTST_NAN 2
