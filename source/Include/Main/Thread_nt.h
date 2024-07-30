//#pragma once
//
//#include "AlifCore_Time.h"
//
//AlifThreadIdentT alifThread_get_thread_ident_ex(void)
//{
//    //if (!initialized)
//        //alifThread_init_thread();
//
//    return GetCurrentThreadId();
//}
//
//unsigned long alifThread_get_thread_ident(void)
//{
//    return (unsigned long)alifThread_get_thread_ident_ex();
//}
//
//
///*
// * Return the native Thread ID (TID) of the calling thread.
// * The native ID of a thread is valid and guaranteed to be unique system-wide
// * from the time the thread is created until the thread has been terminated.
// */
//unsigned long alifThread_get_thread_native_id(void)
//{
//    //if (!initialized) {
//        //alifThread_init_thread();
//    //}
//
//    DWORD nativeID;
//    nativeID = GetCurrentThreadId();
//    return (unsigned long)nativeID;
//}
//
//int alifThread_tss_create(AlifTSST* _key)
//{
//    if (_key->isInitialized) {
//        return 0;
//    }
//
//    DWORD result_ = TlsAlloc();
//    if (result_ == TLS_OUT_OF_INDEXES) {
//        return -1;
//    }
//    _key->key_ = result_;
//    _key->isInitialized = 1;
//    return 0;
//}
//
//void alifThread_tss_delete(AlifTSST* _key)
//{
//    if (!_key->isInitialized) {
//        return;
//    }
//
//    TlsFree(_key->key_);
//    _key->key_ = TLS_OUT_OF_INDEXES;
//    _key->isInitialized = 0;
//}
//
//int alifThread_tss_set(AlifTSST* _key, void* _value)
//{
//    BOOL ok_ = TlsSetValue(_key->key_, _value);
//    return ok_ ? 0 : -1;
//}
//
//void* alifThread_tss_get(AlifTSST* _key)
//{
//    int err_ = GetLastError();
//    void* r_ = TlsGetValue(_key->key_);
//    if (r_ || !GetLastError()) {
//        SetLastError(err_);
//    }
//    return r_;
//}
