#include "alif.h"

#include "AlifCore_Frame.h"
#include "AlifCore_Function.h"
#include "AlifCore_GlobalObjects.h"
#include "AlifCore_Compile.h"
#include "AlifCore_Intrinsics.h"
#include "AlifCore_Errors.h"
#include "AlifCore_Runtime.h"
#include "AlifCore_SysModule.h"






static AlifObject* no_intrinsic1(AlifThread* _thread, AlifObject* _unused) { // 18
	_alifErr_setString(_thread, _alifExcSystemError_, "خطأ في وظيفة جوهرية");
	return nullptr;
}







static AlifObject* list_toTuple(AlifThread* unused, AlifObject* v) { // 191
	return _alifTuple_fromArray(((AlifListObject*)v)->item, ALIF_SIZE(v));
}






const IntrinsicFunc1Info _alifIntrinsicsUnaryFunctions_[] = { // 209
	{ no_intrinsic1, "INTRINSIC_1_INVALID" },
	{nullptr},//{print_expr, "INTRINSIC_PRINT"},
	{nullptr},//{import_star, "INTRINSIC_IMPORT_STAR"},
	{nullptr},//{stopIteration_error, "INTRINSIC_STOPITERATION_ERROR"},
	{nullptr},//{_alif_asyncGenValueWrapperNew, "INTRINSIC_ASYNC_GEN_WRAP"},
	{nullptr},//{unary_pos, "INTRINSIC_UNARY_POSITIVE"},
	{list_toTuple, "INTRINSIC_LIST_TO_TUPLE"},
	//{make_typeVar, "INTRINSIC_TYPEVAR"},
	//{_alif_makeParamSpec, "INTRINSIC_PARAMSPEC"},
	//{_alif_makeTypeVarTuple, "INTRINSIC_TYPEVARTUPLE"},
	//{_alif_subScriptGeneric, "INTRINSIC_SUBSCRIPT_GENERIC"},
	//{ _alif_makeTypeAlias, "INTRINSIC_TYPEALIAS" },
};
