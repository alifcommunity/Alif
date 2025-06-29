#pragma once



#define ALIF_SINGLE_INPUT 256
#define ALIF_FILE_INPUT 257
#define ALIF_EVAL_INPUT 258
#define ALIF_FUNC_TYPE_INPUT 345




/* ------------------------------------------------------------------------------------------ */

 // 6
#define ALIFCF_MASK (CO_FUTURE_DIVISION | CO_FUTURE_ABSOLUTE_IMPORT | \
                   CO_FUTURE_WITH_STATEMENT | CO_FUTURE_PRINT_FUNCTION | \
                   CO_FUTURE_UNICODE_LITERALS | CO_FUTURE_BARRY_AS_BDFL | \
                   CO_FUTURE_GENERATOR_STOP | CO_FUTURE_ANNOTATIONS)
#define ALIFCF_MASK_OBSOLETE (CO_NESTED)

#define ALIFCF_SOURCE_IS_UTF8  0x0100
#define ALIFCF_DONT_IMPLY_DEDENT 0x0200
#define ALIFCF_ONLY_AST 0x0400
#define ALIFCF_IGNORE_COOKIE 0x0800
#define ALIFCF_TYPE_COMMENTS 0x1000
#define ALIFCF_ALLOW_TOP_LEVEL_AWAIT 0x2000
#define ALIFCF_ALLOW_INCOMPLETE_INPUT 0x4000
#define ALIFCF_OPTIMIZED_AST (0x8000 | ALIFCF_ONLY_AST)
#define ALIFCF_COMPILE_MASK (ALIFCF_ONLY_AST | ALIFCF_ALLOW_TOP_LEVEL_AWAIT | \
                           ALIFCF_TYPE_COMMENTS | ALIFCF_DONT_IMPLY_DEDENT | \
                           ALIFCF_ALLOW_INCOMPLETE_INPUT | ALIFCF_OPTIMIZED_AST)



class AlifCompilerFlags { // 27
public:
	AlifIntT flags{};  /* bitmask of CO_xxx flags relevant to future */
	AlifIntT featureVersion{};  /* minor Alif version */
};

// 32
#define ALIFCOMPILERFLAGS_INIT {.flags = 0, .featureVersion = ALIF_MINOR_VERSION}


// 37
#define FUTURE_NESTED_SCOPES "nested_scopes"
#define FUTURE_GENERATORS "generators"
#define FUTURE_DIVISION "division"
#define FUTURE_ABSOLUTE_IMPORT "absolute_import"
#define FUTURE_WITH_STATEMENT "with_statement"
#define FUTURE_PRINT_FUNCTION "print_function"
#define FUTURE_UNICODE_LITERALS "unicode_literals"
#define FUTURE_BARRY_AS_BDFL "barry_as_FLUFL"
#define FUTURE_GENERATOR_STOP "generator_stop"
#define FUTURE_ANNOTATIONS "annotations"

#define ALIF_INVALID_STACK_EFFECT INT_MAX // 48

AlifIntT alifCompile_opcodeStackEffect(AlifIntT, AlifIntT); // 49
