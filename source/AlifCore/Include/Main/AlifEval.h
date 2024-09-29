#pragma once







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



/* ----------------------------------------------------------------------------------------------------- */
