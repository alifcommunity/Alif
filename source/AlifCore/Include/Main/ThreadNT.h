#pragma once



AlifSizeT alifThread_getThreadID() {

    //if (!initialized) alifThread_initThread();

    return GetCurrentThreadId();
}


