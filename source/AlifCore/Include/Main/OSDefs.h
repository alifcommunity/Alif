#pragma once



#ifdef MS_WINDOWS
#  define SEP L'\\'
#  define ALTSEP L'/'
#  define MAXPATHLEN 256
#  define DELIM L';'
#endif

#ifndef SEP
#  define SEP L'/'
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
