#include "alif.h"

#include "AlifCore_Code.h"
#include "AlifCore_Compile.h"
#include "AlifCore_InstructionSequence.h"
#include "AlifCore_OpcodeUtils.h"
//#include "AlifCore_OpcodeMetadata.h"
#include "AlifCore_SymTable.h"




 // 16
#undef SUCCESS
#undef ERROR
#define SUCCESS 0
#define ERROR -1

#define RETURN_IF_ERROR(X)  \
    if ((X) < 0) {          \
        return ERROR;       \
    }









class AlifAssembler { // 51
public:
	AlifObject* bytecode{};
	AlifIntT offset{};         
	AlifObject* exceptTable{}; 
	AlifIntT exceptTableOff{};
	/* Location Info */
	AlifIntT lineno{};
	AlifObject* lineTable{};
	AlifIntT locationOff{};
};


























































AlifCodeObject* alifAssemble_makeCodeObject(AlifCompileCodeUnitMetadata* _umd,
	AlifObject* _constCache, AlifObject* _consts, AlifIntT _maxDepth, InstrSequence* _instrs,
	AlifIntT _nLocalsPlus, AlifIntT _codeFlags, AlifObject* _filename) { // 735
	if (alifInstructionSequence_applyLabelMap(_instrs) < 0) {
		return nullptr;
	}
	if (resolve_unconditionalJumps(_instrs) < 0) {
		return nullptr;
	}
	if (resolve_jumpOffsets(_instrs) < 0) {
		return nullptr;
	}
	AlifCodeObject* co = nullptr;

	AlifAssembler a{};
	AlifIntT res = assemble_emit(&a, _instrs, _umd->firstLineno, _constCache);
	if (res == SUCCESS) {
		co = makecode(_umd, &a, _constCache, _consts, _maxDepth, _nLocalsPlus,
			_codeFlags, _filename);
	}
	assemble_free(&a);
	return co;
}
