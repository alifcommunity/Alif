#pragma once


// 14
#ifndef S_IFMT
#  define S_IFMT 0170000
#endif
#ifndef S_IFLNK
#  define S_IFLNK 0120000
#endif
#ifndef S_ISREG
#  define S_ISREG(x) (((x) & S_IFMT) == S_IFREG)
#endif
#ifndef S_ISDIR
#  define S_ISDIR(x) (((x) & S_IFMT) == S_IFDIR)
#endif
#ifndef S_ISCHR
#  define S_ISCHR(x) (((x) & S_IFMT) == S_IFCHR)
#endif
#ifndef S_ISLNK
#  define S_ISLNK(x) (((x) & S_IFMT) == S_IFLNK)
#endif



wchar_t* alif_decodeLocale(const char*, AlifUSizeT*); // 44







/* --------------------------------------------------------------------------------------------- */






FILE* alif_fOpenObj(AlifObject*, const char*);
