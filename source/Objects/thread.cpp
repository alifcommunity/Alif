#include "alif.h"

#include "alifCore_alifThread.h"

#ifndef DONT_HAVE_STDIO_H
#include <stdio.h>
#endif

#include <stdlib.h>

void alifThread__init_thread(void); /* Forward */


#define INITIALIZED alifRuntime.threads.initialized

void alifThread_initThread() {

	if (INITIALIZED) {
		return;
	}

	INITIALIZED = 1;
	alifThread__init_thread();

}
