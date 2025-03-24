#pragma once



// 18
#ifndef S_IFLNK
	#define S_IFLNK 0120000
#endif

#ifndef S_ISDIR
#  define S_ISDIR(x) (((x) & S_IFMT) == S_IFDIR)
#endif


wchar_t* alif_decodeLocale(const char*, AlifUSizeT*); // 44







/* --------------------------------------------------------------------------------------------- */






FILE* alif_fOpenObj(AlifObject*, const char*);
