#pragma once






#define ALIF_CPPTF_LOWER  0x01
#define ALIF_CPPTF_UPPER  0x02
#define ALIF_CPPTF_ALPHA  (ALIF_CPPTF_LOWER|ALIF_CPPTF_UPPER)
#define ALIF_CPPTF_DIGIT  0x04
#define ALIF_CPPTF_ALNUM  (ALIF_CPPTF_ALPHA|ALIF_CPPTF_DIGIT)
#define ALIF_CPPTF_SPACE  0x08
#define ALIF_CPPTF_XDIGIT 0x10

extern const unsigned int _alifCppTypeTable_[256];



#define ALIF_ISLOWER(_c)  (_alifCppTypeTable_[ALIF_CHARMASK(_c)] & ALIF_CPPTF_LOWER)
#define ALIF_ISUPPER(_c)  (_alifCppTypeTable_[ALIF_CHARMASK(_c)] & ALIF_CPPTF_UPPER)
#define ALIF_ISALPHA(_c)  (_alifCppTypeTable_[ALIF_CHARMASK(_c)] & ALIF_CPPTF_ALPHA)
#define ALIF_ISDIGIT(_c) (_alifCppTypeTable_[ALIF_CHARMASK(_c)] & ALIF_CPPTF_DIGIT)
#define ALIF_ISXDIGIT(_c) (_alifCppTypeTable_[ALIF_CHARMASK(_c)] & ALIF_CPPTF_XDIGIT)
#define ALIF_ISALNUM(_c)  (_alifCppTypeTable_[ALIF_CHARMASK(_c)] & ALIF_CPPTF_ALNUM)
#define ALIF_ISSPACE(_c)  (_alifCppTypeTable_[ALIF_CHARMASK(_c)] & ALIF_CPPTF_SPACE)


// لا حاجة بها ويجب خذفها
//extern const unsigned char _alifCppTypeToLower_[256];
//extern const unsigned char _alifCppTypeToUpper_[256];
//
//#define ALIF_TOLOWER(_c) (_alifCppTypeToLower_[ALIF_CHARMASK(_c)])
//#define ALIF_TOUPPER(_c) (_alifCppTypeToUpper_[ALIF_CHARMASK(_c)])

