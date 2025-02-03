#include "alif.h"

#include "AlifCore_InitConfig.h"
#include "AlifCore_ModSupport.h"
#include "AlifCore_Object.h"
#include "AlifCore_Tuple.h"



static const char _visibleLengthKey_[] = "NSequenceFields";
static const char _realLengthKey_[] = "NFields";
static const char _unnamedFieldsKey_[] = "NUnnamedFields";
static const char _matchArgsKey_[] = "__matchArgs__";


const char* const _alifStructSequenceUnnamedField_ = "unnamed field"; // 24


static AlifSizeT getTypeAttr_asSize(AlifTypeObject* _tp, AlifObject* _name) { // 26
	AlifObject* v = alifDict_getItemWithError(alifType_getDict(_tp), _name);
	if (v == nullptr and !alifErr_occurred()) {
		//alifErr_format(_alifExcTypeError_,
		//	"Missed attribute '%U' of type %s",
		//	_name, _tp->name);
		return -1;
	}
	return alifLong_asSizeT(v);
}


 // 40
#define VISIBLE_SIZE_TP(tp) \
    getTypeAttr_asSize(tp, &ALIF_ID(NSequenceFields))
#define REAL_SIZE_TP(tp) \
    getTypeAttr_asSize(tp, &ALIF_ID(NFields))


AlifObject* alifStructSequence_new(AlifTypeObject* _type) { // 59
	AlifStructSequence* obj{};
	AlifSizeT size = REAL_SIZE_TP(_type), i;
	if (size < 0) {
		return nullptr;
	}
	AlifSizeT vsize = VISIBLE_SIZE_TP(_type);
	if (vsize < 0) {
		return nullptr;
	}

	obj = ALIFOBJECT_GC_NEWVAR(AlifStructSequence, _type, size);
	if (obj == nullptr)
		return nullptr;
	ALIF_SET_SIZE(obj, vsize);
	for (i = 0; i < size; i++)
		obj->item[i] = nullptr;

	return (AlifObject*)obj;
}



void alifStructSequence_setItem(AlifObject* _op,
	AlifSizeT _index, AlifObject* _value) { // 84
	AlifTupleObject* tuple = ALIFTUPLE_CAST(_op);
	tuple->item[_index] = _value;
}

AlifObject* alifStructSequence_getItem(AlifObject* _op, AlifSizeT _index) { // 97
	return ALIFTUPLE_GET_ITEM(_op, _index);
}




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

static AlifIntT initialize_structSeqDict(AlifStructSequenceDesc* _desc, AlifObject* _dict,
	AlifSizeT _nMembers, AlifSizeT _nUnnamedMembers) { // 486
	AlifObject* v{};

#define SET_DICT_FROM_SIZE(key, value)                                         \
    do {                                                                       \
        v = alifLong_fromSizeT(value);                                         \
        if (v == nullptr) {                                                       \
            return -1;                                                         \
        }                                                                      \
        if (alifDict_setItemString(_dict, key, v) < 0) {                          \
            ALIF_DECREF(v);                                                      \
            return -1;                                                         \
        }                                                                      \
        ALIF_DECREF(v);                                                          \
    } while (0)

	SET_DICT_FROM_SIZE(_visibleLengthKey_, _desc->nInSequence);
	SET_DICT_FROM_SIZE(_realLengthKey_, _nMembers);
	SET_DICT_FROM_SIZE(_unnamedFieldsKey_, _nUnnamedMembers);

	// Prepare and set __matchArgs__
	AlifSizeT i{}, k{};
	AlifObject* keys = alifTuple_new(_desc->nInSequence);
	if (keys == nullptr) {
		return -1;
	}

	for (i = k = 0; i < _desc->nInSequence; ++i) {
		if (_desc->fields[i].name == _alifStructSequenceUnnamedField_) {
			continue;
		}
		AlifObject* new_member = alifUStr_fromString(_desc->fields[i].name);
		if (new_member == nullptr) {
			goto error;
		}
		ALIFTUPLE_SET_ITEM(keys, k, new_member);
		k++;
	}

	if (alifTuple_resize(&keys, k) == -1) {
		goto error;
	}

	if (alifDict_setItemString(_dict, _matchArgsKey_, keys) < 0) {
		goto error;
	}

	ALIF_DECREF(keys);
	return 0;

error:
	ALIF_DECREF(keys);
	return -1;
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




AlifIntT _alifStructSequence_initBuiltinWithFlags(AlifInterpreter* _interp,
	AlifTypeObject* _type, AlifStructSequenceDesc* _desc, unsigned long _tpFlags) { // 617
	if (ALIF_TYPE(_type) == nullptr) {
		ALIF_SET_TYPE(_type, &_alifTypeType_);
	}
	AlifSizeT nUnnamedMembers{};
	AlifSizeT nMembers = count_members(_desc, &nUnnamedMembers);
	AlifMemberDef* members = nullptr;

	if ((_type->flags & ALIF_TPFLAGS_READY) == 0) {

		members = initialize_members(_desc, nMembers, nUnnamedMembers);
		if (members == nullptr) {
			goto error;
		}
		initialize_staticFields(_type, _desc, members, nMembers, _tpFlags);

		alif_setImmortal((AlifObject*)_type);
	}

	if (alifStaticType_initBuiltin(_interp, _type) < 0) {
		//alifErr_format(_alifExcRuntimeError_,
		//	"Can't initialize builtin type %s",
		//	desc->name);
		goto error;
	}

	if (initialize_structSeqDict(
		_desc, alifType_getDict(_type), nMembers, nUnnamedMembers) < 0) {
		goto error;
	}

	return 0;

error:
	if (members != nullptr) {
		alifMem_dataFree(members);
	}
	return -1;
}
