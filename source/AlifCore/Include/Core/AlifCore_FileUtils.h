#pragma once




enum AlifErrorHandler_ { // 26
	Alif_Error_Unknown = 0,
	Alif_Error_Strict,
	Alif_Error_SurrogateEscape,
	Alif_Error_Replace,
	Alif_Error_Ignore,
	Alif_Error_BackSlashReplace,
	Alif_Error_SurrogatePass,
	Alif_Error_XMLCharRefReplace,
	Alif_Error_Other
};


AlifErrorHandler_ alif_getErrorHandler(const char*); // 39


AlifIntT alif_decodeLocaleEx(const char*, wchar_t**, AlifUSizeT*,
	const char**, AlifIntT, AlifErrorHandler_); // 42


AlifIntT _alif_encodeLocaleEx(const wchar_t*, char**,
	AlifUSizeT*, const char**, AlifIntT, AlifErrorHandler_); // 51

extern char* _alif_encodeLocaleRaw(const wchar_t*, AlifUSizeT*); // 59


// 78
#ifdef _WINDOWS
class AlifStatStruct {
public:
	uint64_t dev{};
	uint64_t ino{};
	unsigned short mode{};
	AlifIntT nlink{};
	AlifIntT uid{};
	AlifIntT gid{};
	unsigned long rdev{};
	__int64 size{};
	time_t atime{};
	AlifIntT atimeNSec{};
	time_t mtime{};
	AlifIntT mtimeNSec{};
	time_t ctime{};
	AlifIntT ctimeNSec{};
	time_t birthtime{};
	AlifIntT birthtimeNSec{};
	unsigned long fileAttributes{};
	unsigned long reparseTag{};
	uint64_t inoHigh{};
};
#else
	#define AlifStatStruct stat
#endif



AlifIntT _alifFStat_noraise(AlifIntT, class AlifStatStruct*); // 110



#ifdef HAVE_READLINK
extern int alif_wReadLink(const wchar_t*, wchar_t*, AlifUSizeT); // 151
#endif

#ifdef HAVE_REALPATH
extern wchar_t* alif_wRealPath(const wchar_t*, wchar_t*, AlifUSizeT); // 160
#endif

extern wchar_t* alif_wGetCWD(wchar_t* , AlifUSizeT ); // 168

AlifIntT _alif_dup(AlifIntT); // 185

#ifdef _WINDOWS
extern void* _alifGet_osfHandleNoRaise(AlifIntT); // 192

#endif  /* _WINDOWS */

extern AlifIntT alif_decodeUTF8Ex(const char*, AlifSizeT,
	wchar_t**, AlifUSizeT*, const char**, AlifErrorHandler_); // 210

extern AlifIntT _alif_wStat(const wchar_t*, struct stat*); // 231

//wchar_t* alifUniversal_newLineFGetsWithSize(wchar_t*, int, FILE*, AlifSizeT*);


extern AlifIntT _alif_isAbs(const wchar_t*); // 267
extern AlifIntT _alif_absPath(const wchar_t*, wchar_t**); // 268
#ifdef _WINDOWS
extern AlifIntT alifOS_getFullPathName(const wchar_t*, wchar_t**); // 270
#endif

extern AlifIntT _alif_addRelfile(wchar_t*, const wchar_t*, AlifUSizeT); // 274

wchar_t* _alif_normPath(wchar_t*, AlifSizeT); // 280

 // 302
#if defined _MSC_VER && _MSC_VER >= 1900

	#include <stdlib.h>   // _set_thread_local_invalid_parameter_handler()

	extern _invalid_parameter_handler _alifSilentInvalidParameterHandler_;
	#define ALIF_BEGIN_SUPPRESS_IPH \
    { _invalid_parameter_handler _alifOldHandler_ = \
      _set_thread_local_invalid_parameter_handler(_alifSilentInvalidParameterHandler_);
	#define ALIF_END_SUPPRESS_IPH \
    _set_thread_local_invalid_parameter_handler(_alifOldHandler_); }
#else
	#define ALIF_BEGIN_SUPPRESS_IPH
	#define ALIF_END_SUPPRESS_IPH
#endif /* _MSC_VER >= 1900 */



char* alifUniversal_newLineFGetsWithSize(char*, AlifIntT, FILE*, AlifObject*, AlifUSizeT*); // 321



AlifIntT _alif_isValidFD(AlifIntT); // 330
