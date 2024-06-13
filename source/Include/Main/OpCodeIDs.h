#pragma once









#define NOP		                    30
#define POP_TOP                     32

#define RETURN_VALUE                36
#define TO_BOOL                     40
#define BINARY_OP					45

#define CALL_INTRINSIC_1			55
#define CALL_INTRINSIC_2			56
#define COPY                        61
#define EXTENDED_ARG                71
#define IS_OP                       76
#define JUMP_BACKWARD               77
#define JUMP_BACKWARD_NO_INTERRUPT  78
#define JUMP_FORWARD                79

#define LOAD_CONST                  83

#define POP_JUMPIF_FALSE			97
#define POP_JUMPIF_NONE				98
#define POP_JUMPIF_NOTNONE			99
#define POP_JUMPIF_TRUE				100
#define RAISE_VARARGS               101
#define RERAISE                     102
#define RETURN_CONST                103

#define RESUME						149

#define JUMP                        256
#define JUMP_NO_INTERRUPT           257

#define SETUP_CLEANUP               264
#define SETUP_FINALLY               265
#define SETUP_WITH                  266