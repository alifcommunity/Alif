#pragma once



#ifdef _WINDOWS
#  define SEP L'\\'
#  define ALTSEP L'/'
#  define MAXPATHLEN 256
#  define DELIM L';'
#endif

/* Filename separator */
#ifndef SEP
#  define SEP L'/'
#endif

/* Max pathname length */
#ifdef __hpux
#  include <sys/param.h>
#  include <limits.h>
#  ifndef PATH_MAX
#    define PATH_MAX MAXPATHLEN
#  endif
#endif

#ifndef MAXPATHLEN
#  if defined(PATH_MAX) && PATH_MAX > 1024
#    define MAXPATHLEN PATH_MAX
#  else
#    define MAXPATHLEN 1024
#  endif
#endif


#ifndef DELIM
#  define DELIM L':'
#endif
