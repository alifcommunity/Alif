#pragma once



#define ALIF_SINGLE_INPUT 256
#define ALIF_FILE_INPUT 257
#define ALIF_EVAL_INPUT 258
#define ALIF_FUNC_TYPE_INPUT 345




/* ------------------------------------------------------------------------------------------ */


class AlifCompilerFlags { // 27
public:
	AlifIntT flags{};  /* bitmask of CO_xxx flags relevant to future */
	AlifIntT featureVersion{};  /* minor Alif version */
};

 // 32
#define ALIFCOMPILERFLAGS_INIT {.flags = 0, .featureVersion = ALIF_MINOR_VERSION}
