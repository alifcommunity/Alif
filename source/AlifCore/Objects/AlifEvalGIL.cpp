#include "alif.h"










void alifEval_releaseThread(AlifThread* _thread) { // 598
	alifThread_detach(_thread);
}
