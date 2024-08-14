#pragma once





#ifndef POSIX_THREADS // 8
#  define POSIX_THREADS 1
#endif






typedef class { unsigned __attr; } pthread_condattr_t; // 50
typedef unsigned pthread_key_t; // 53







#ifndef PTHREAD_KEYS_MAX // 95
#  define PTHREAD_KEYS_MAX 128
#endif








