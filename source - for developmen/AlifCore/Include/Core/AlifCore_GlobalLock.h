#pragma once

#include "AlifCore_CondVar.h"

class GilRuntimeState {
#ifdef ALIF_GIL_DISABLED

    int enabled_;
#endif
    unsigned long interval_;

    //AlifThreadState* lastHolder; // سيتم اضافته لاحقا لان هذا الجزء متعلق ب المفسر

    int locked_;
    unsigned long switchNumber;

    AlifCondT cond_;
    AlifMutexT mutex_;
};