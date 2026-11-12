#pragma once




ALIFAPI_FUNC(AlifObject*) alifEval_evalCode(AlifObject*, AlifObject*, AlifObject*); // 10

ALIFAPI_FUNC(AlifObject*) alifEval_evalCodeEx(AlifObject*, AlifObject*, AlifObject*,
	AlifObject* const*, AlifIntT, AlifObject* const*, AlifIntT,
	AlifObject* const*, AlifIntT, AlifObject*, AlifObject*);

ALIFAPI_FUNC(AlifObject*) alifEval_getBuiltins(); // 20
ALIFAPI_FUNC(AlifObject*) alifEval_getGlobals(); // 21

ALIFAPI_FUNC(AlifIntT) alif_makePendingCalls(); // 30







ALIFAPI_FUNC(AlifThread*) alifEval_saveThread(); // 111
ALIFAPI_FUNC(void) alifEval_restoreThread(AlifThread*); // 112

ALIFAPI_FUNC(void) alifEval_acquireThread(AlifThread*); // 116
ALIFAPI_FUNC(void) alifEval_releaseThread(AlifThread*); // 117

 // 119
#define ALIF_BEGIN_ALLOW_THREADS { \
                        AlifThread *_save; \
                        _save = alifEval_saveThread();
#define ALIF_BLOCK_THREADS        alifEval_restoreThread(_save);
#define ALIF_UNBLOCK_THREADS      _save = alifEval_saveThread();
#define ALIF_END_ALLOW_THREADS    alifEval_restoreThread(_save); \
                 }



 // 127
/* Masks and values used by FORMAT_VALUE opcode. */
#define FVC_MASK      0x3
#define FVC_NONE      0x0
#define FVC_STR       0x1
#define FVC_REPR      0x2
#define FVC_ASCII     0x3
#define FVS_MASK      0x4
#define FVS_HAVE_SPEC 0x4


/* Special methods used by LOAD_SPECIAL */
#define SPECIAL___ENTER__   0
#define SPECIAL___EXIT__    1
#define SPECIAL___AENTER__  2
#define SPECIAL___AEXIT__   3
#define SPECIAL_MAX   3


/* ----------------------------------------------------------------------------------------------------- */





ALIFAPI_FUNC(AlifIntT) alifEval_mergeCompilerFlags(AlifCompilerFlags*); // 13

ALIFAPI_FUNC(AlifObject*) alifEval_evalFrameDefault(AlifThread*, class AlifInterpreterFrame*, AlifIntT); // 15



ALIFAPI_FUNC(AlifIntT) _alifEval_sliceIndex(AlifObject*, AlifSizeT*); // 24
