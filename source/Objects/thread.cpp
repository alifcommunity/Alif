






#include "alif.h"
#include "alifCore_alifState.h"      
//#include "alifCore_structSeq.h"     
#include "alifCore_alifThread.h"

#ifndef DONT_HAVESTDIO_H
#include <stdio.h>
#endif

#include <stdlib.h>


static void alifThread__init_thread(); 

#define INITIALIZED alifRuntime.threads.initialized

void alifThread_initThread()
{
	if (INITIALIZED) {
		return;
	}
	INITIALIZED = 1;
	alifThread__init_thread();
}
