#include "alif.h"

#include "AlifCore_InitConfig.h"
#include "AlifCore_ModSupport.h"
#include "AlifCore_Object.h"
#include "AlifCore_Tuple.h"






const char* const _alifStructSequenceUnnamedField_ = "unnamed field";







static AlifSizeT count_members(AlifStructSequenceDesc* desc,
	AlifSizeT* n_unnamed_members) { // 473
	AlifSizeT i{};

	*n_unnamed_members = 0;
	for (i = 0; desc->fields[i].name != nullptr; ++i) {
		if (desc->fields[i].name == _alifStructSequenceUnnamedField_) {
			(*n_unnamed_members)++;
		}
	}
	return i;
}


static AlifMemberDef* initialize_members(AlifStructSequenceDesc* desc,
	AlifSizeT n_members, AlifSizeT n_unnamed_members) { // 543
	AlifMemberDef* members{};

	members = (((AlifUSizeT)(n_members - n_unnamed_members + 1) > ALIF_SIZET_MAX / sizeof(AlifMemberDef)) ? nullptr : \
		((AlifMemberDef*)alifMem_dataAlloc((n_members - n_unnamed_members + 1) * sizeof(AlifMemberDef))));
	if (members == nullptr) {
		//alifErr_noMemory();
		return nullptr;
	}

	AlifSizeT i{}, k{};
	for (i = k = 0; i < n_members; ++i) {
		if (desc->fields[i].name == _alifStructSequenceUnnamedField_) {
			continue;
		}

		/* The names and docstrings in these MemberDefs are statically */
		/* allocated so it is expected that they'll outlive the MemberDef */
		members[k].name = desc->fields[i].name;
		members[k].type = ALIF_T_OBJECT;
		members[k].offset = offsetof(AlifStructSequence, item)
			+ i * sizeof(AlifObject*);
		members[k].flags = ALIF_READONLY;
		//members[k].doc = desc->fields[i].doc;
		k++;
	}
	members[k].name = nullptr;

	return members;
}




static void initialize_staticFields(AlifTypeObject* type, AlifStructSequenceDesc* desc,
	AlifMemberDef* tp_members, AlifSizeT n_members,
	unsigned long tp_flags) { // 577
	type->name = desc->name;

	AlifSizeT n_hidden = n_members - desc->nInSequence;
	type->basicSize = sizeof(AlifStructSequence) + (n_hidden - 1) * sizeof(AlifObject*);
	type->itemSize = sizeof(AlifObject*);
	//type->dealloc = (Destructor)structSeq_dealloc;
	//type->repr = (ReprFunc)structSeq_repr;
	//type->doc = desc->doc;
	type->base = &_alifTupleType_;
	//type->methods = structSeq_methods;
	//type->new_ = structSeq_new;
	type->flags = ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_HAVE_GC | tp_flags;
	//type->traverse = (TraverseProc)structSeq_traverse;
	//type->members = tp_members;
}




AlifIntT _alifStructSequence_initBuiltinWithFlags(AlifInterpreter* interp,
	AlifTypeObject* type, AlifStructSequenceDesc* desc, unsigned long tp_flags) { // 617
	if (ALIF_TYPE(type) == nullptr) {
		ALIF_SET_TYPE(type, &_alifTypeType_);
	}
	AlifSizeT n_unnamed_members{};
	//AlifSizeT n_members = count_members(desc, &n_unnamed_members);
	AlifMemberDef* members = nullptr;

	if ((type->flags & ALIF_TPFLAGS_READY) == 0) {

		//members = initialize_members(desc, n_members, n_unnamed_members);
		//if (members == nullptr) {
		//	goto error;
		//}
		//initialize_staticFields(type, desc, members, n_members, tp_flags);

		alif_setImmortal((AlifObject*)type);
	}

	if (alifStaticType_initBuiltin(interp, type) < 0) {
		//alifErr_format(_alifExcRuntimeError_,
		//	"Can't initialize builtin type %s",
		//	desc->name);
		goto error;
	}

	//if (initialize_structSeqDict(
	//	desc, alifType_getDict(type), n_members, n_unnamed_members) < 0) {
	//	goto error;
	//}

	return 0;

error:
	if (members != nullptr) {
		alifMem_dataFree(members);
	}
	return -1;
}
