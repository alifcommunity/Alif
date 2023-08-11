#pragma once



#if !defined(ALIF_LIMITED_API) || ALIF_LIMITED_API+0 >= 0x03070000
ALIFAPI_DATA(int) alifUTF8Mode;
#endif


#ifndef ALIF_LIMITED_API
#  define ALIF_CPPALIF_FILEOBJECT_H

typedef AlifObject* (*AlifOpenCodeHookFunction)(AlifObject*, void*);

#  undef ALIF_CPPALIF_FILEOBJECT_H
#endif
