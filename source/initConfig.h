#pragma once

#include<iostream>

class ArgvArgc {
public:
    int argc;
    int useBytesArgv;
    char* const* bytesArgv;
    wchar_t* const* argv;
};