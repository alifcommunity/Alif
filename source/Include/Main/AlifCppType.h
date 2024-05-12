#pragma once






#define ALIF_CPPTF_LOWER  0x01
#define ALIF_CPPTF_UPPER  0x02
#define ALIF_CPPTF_ALPHA  (ALIF_CPPTF_LOWER|ALIF_CPPTF_UPPER)
#define ALIF_CPPTF_DIGIT  0x04
#define ALIF_CPPTF_ALNUM  (ALIF_CPPTF_ALPHA|ALIF_CPPTF_DIGIT)
#define ALIF_CPPTF_SPACE  0x08
#define ALIF_CPPTF_XDIGIT 0x10

extern const unsigned int alifCppTypeTable[256];



#define ALIF_ISLOWER(wcs)  (alifCppTypeTable[ALIF_WCHARMASK(wcs)] & ALIF_CPPTF_LOWER)
#define ALIF_ISUPPER(wcs)  (alifCppTypeTable[ALIF_WCHARMASK(wcs)] & ALIF_CPPTF_UPPER)
#define ALIF_ISALPHA(wcs)  (alifCppTypeTable[ALIF_WCHARMASK(wcs)] & ALIF_CPPTF_ALPHA)
#define ALIF_ISDIGIT(wcs) (alifCppTypeTable[ALIF_WCHARMASK(wcs)] & ALIF_CPPTF_DIGIT)
#define ALIF_ISXDIGIT(wcs) (alifCppTypeTable[ALIF_WCHARMASK(wcs)] & ALIF_CPPTF_XDIGIT)
#define ALIF_ISALNUM(wcs)  (alifCppTypeTable[ALIF_WCHARMASK(wcs)] & ALIF_CPPTF_ALNUM)
#define ALIF_ISSPACE(wcs)  (alifCppTypeTable[ALIF_WCHARMASK(wcs)] & ALIF_CPPTF_SPACE)


extern const unsigned char alifCTypeToLower[256];
extern const unsigned char alifCTypeToUpper[256];

#define ALIF_TOLOWER(wcs) (alifCTypeToLower[ALIF_WCHARMASK(wcs)])
#define ALIF_TOUPPER(wcs) (alifCTypeToUpper[ALIF_WCHARMASK(wcs)])

