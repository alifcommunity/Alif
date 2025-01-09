#include "alif.h"

#include "AlifCore_AST.h"
#include "AlifCore_Call.h"
#include "AlifCore_FileUtils.h"
#include "AlifCore_Interpreter.h"
#include "AlifCore_Parser.h"
#include "AlifCore_State.h"
#include "AlifCore_SysModule.h"
#include "AlifCore_Traceback.h"









AlifTypeObject _alifTraceBackType_ = { // 209
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "تتبع",
	.basicSize = sizeof(AlifTracebackObject),
	//.dealloc = (Destructor)tb_dealloc,
	.getAttro = alifObject_genericGetAttr,
	.flags = ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_HAVE_GC,
	//.traverse = (TraverseProc)tb_traverse,
	//(Inquiry)tb_clear,
	//.methods = tb_methods,
	//.members = tb_memberlist,
	//.getSet = tb_getsetters,
	//.new_ = tb_new,
};
