#pragma once





#define INTRINSIC_1_INVALID                      0
#define INTRINSIC_PRINT                          1
#define INTRINSIC_IMPORT_STAR                    2
#define INTRINSIC_STOPITERATION_ERROR            3
#define INTRINSIC_ASYNC_GEN_WRAP                 4
#define INTRINSIC_UNARY_POSITIVE                 5
#define INTRINSIC_LIST_TO_TUPLE                  6
#define INTRINSIC_TYPEVAR                        7
#define INTRINSIC_PARAMSPEC                      8
#define INTRINSIC_TYPEVARTUPLE                   9
#define INTRINSIC_SUBSCRIPT_GENERIC             10




/* Binary Functions: */
#define INTRINSIC_2_INVALID                      0
#define INTRINSIC_PREP_RERAISE_STAR              1
#define INTRINSIC_TYPEVAR_WITH_BOUND             2
#define INTRINSIC_TYPEVAR_WITH_CONSTRAINTS       3
#define INTRINSIC_SET_FUNCTION_TYPE_PARAMS       4
#define INTRINSIC_SET_TYPEPARAM_DEFAULT          5


typedef AlifObject* (*IntrinsicFunc1)(AlifThread*, AlifObject*);
typedef AlifObject* (*IntrinsicFunc2)(AlifThread*, AlifObject*, AlifObject*);

class IntrinsicFunc1Info {
public:
	IntrinsicFunc1 func{};
	const char* name{};
};






extern const IntrinsicFunc1Info _alifIntrinsicsUnaryFunctions_[];
