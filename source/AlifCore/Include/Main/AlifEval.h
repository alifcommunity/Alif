#pragma once




AlifObject* alifEval_evalCode(AlifObject*, AlifObject*, AlifObject*); // 10

AlifObject* alifEval_getBuiltins(); // 20
AlifObject* alifEval_getGlobals(); // 21

AlifIntT alif_makePendingCalls(); // 30


AlifThread* alifEval_saveThread(); // 111
void alifEval_restoreThread(AlifThread*); // 112

void alifEval_acquireThread(AlifThread*); // 116
void alifEval_releaseThread(AlifThread*); // 117

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





/* ----------------------------------------------------------------------------------------------------- */







AlifObject* alifEval_evalFrameDefault(AlifThread*, class AlifInterpreterFrame*, AlifIntT); // 15
