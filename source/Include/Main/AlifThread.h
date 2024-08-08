#pragma once


typedef void* AlifThreadLock;

enum AlifLockStatus {
        Alif_Lock_Failure = 0,
        Alif_Lock_Acquired = 1,
        Alif_Lock_Intr
} ;

typedef class AlifTSST AlifTSST;


#define NATIVE_TSS_KEY_T     unsigned long

class AlifTSST {
public:
    int isInitialized{};
    NATIVE_TSS_KEY_T key_{};
};


#define ALIFTSS_NEEDS_INIT   {0}



