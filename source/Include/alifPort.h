#pragma once

#include "alifConfig.h"


#if defined(__CYGWIN__)
#       define HAVE_DECLSPEC_DLL
#endif

#include "exports.h"

/* only get special linkage if built as shared or platform is Cygwin */
#if defined(ALIF_ENABLE_SHARED) || defined(__CYGWIN__)
#       if defined(HAVE_DECLSPEC_DLL)
#               if defined(ALIF_BUILD_CORE) && !defined(ALIF_BUILD_CORE_MODULE)
#                       define ALIFAPI_FUNC(RTYPE) ALIF_EXPORTED_SYMBOL RTYPE
#                       define ALIFAPI_DATA(RTYPE) extern ALIF_EXPORTED_SYMBOL RTYPE
		/* module init functions inside the core need no external linkage */
		/* except for Cygwin to handle embedding */
#                       if defined(__CYGWIN__)
#                               define ALIFMODINIT_FUNC ALIF_EXPORTED_SYMBOL AlifObject*
#                       else /* __CYGWIN__ */
#                               define ALIFMODINIT_FUNC AlifObject*
#                       endif /* __CYGWIN__ */
#               else /* ALIF_BUILD_CORE */
#                       if !defined(__CYGWIN__)
#                               define ALIFAPI_FUNC(RTYPE) ALIF_IMPORTED_SYMBOL RTYPE
#                       endif /* !__CYGWIN__ */
#                       define ALIFAPI_DATA(RTYPE) extern ALIF_IMPORTED_SYMBOL RTYPE
		/* module init functions outside the core must be exported */
#                       define ALIFMODINIT_FUNC /* extern "C" */ ALIF_EXPORTED_SYMBOL AlifObject*
#               endif /* ALIF_BUILD_CORE */
#       endif /* HAVE_DECLSPEC_DLL */
#endif /* ALIF_ENABLE_SHARED */
