#pragma once





class AlifCompilerFlags { // 27
public:
	AlifIntT flags{};  /* bitmask of CO_xxx flags relevant to future */
	AlifIntT featureVersion{};  /* minor Alif version */
};

 // 32
#define ALIFCOMPILERFLAGS_INIT {.flags = 0, .featureVersion = ALIF_MINOR_VERSION}
