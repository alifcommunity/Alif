#include "alif.h"

#include "AlifCore_AST.h"
#include "AlifCore_Symtable.h"
#include "AlifCore_UStrObject.h"

#define UNDEFINED_FUTURE_FEATURE "future feature %.100s is not defined"

static AlifIntT future_checkFeatures(AlifFutureFeatures* ff, StmtTy s, AlifObject* filename) { // 8
	AlifSizeT i{};

	ASDLAliasSeq* names = s->V.importFrom.names;
	for (i = 0; i < ASDL_SEQ_LEN(names); i++) {
		AliasTy name = (AliasTy)ASDL_SEQ_GET(names, i);
		const char* feature = alifUStr_asUTF8(name->name);
		if (!feature)
			return 0;
		if (strcmp(feature, FUTURE_NESTED_SCOPES) == 0) {
			continue;
		}
		else if (strcmp(feature, FUTURE_GENERATORS) == 0) {
			continue;
		}
		else if (strcmp(feature, FUTURE_DIVISION) == 0) {
			continue;
		}
		else if (strcmp(feature, FUTURE_ABSOLUTE_IMPORT) == 0) {
			continue;
		}
		else if (strcmp(feature, FUTURE_WITH_STATEMENT) == 0) {
			continue;
		}
		else if (strcmp(feature, FUTURE_PRINT_FUNCTION) == 0) {
			continue;
		}
		else if (strcmp(feature, FUTURE_UNICODE_LITERALS) == 0) {
			continue;
		}
		else if (strcmp(feature, FUTURE_BARRY_AS_BDFL) == 0) {
			ff->features |= CO_FUTURE_BARRY_AS_BDFL;
		}
		else if (strcmp(feature, FUTURE_GENERATOR_STOP) == 0) {
			continue;
		}
		else if (strcmp(feature, FUTURE_ANNOTATIONS) == 0) {
			ff->features |= CO_FUTURE_ANNOTATIONS;
		}
		else if (strcmp(feature, "braces") == 0) {
			//alifErr_setString(_alifExcSyntaxError_,
			//	"not a chance");
			//alifErr_syntaxLocationObject(filename, s->lineNo, s->colOffset + 1);
			return 0;
		}
		else {
			//alifErr_format(_alifExcSyntaxError_,
			//	UNDEFINED_FUTURE_FEATURE, feature);
			//alifErr_syntaxLocationObject(filename, s->lineNo, s->colOffset + 1);
			return 0;
		}
	}
	return 1;
}


static AlifIntT future_parse(AlifFutureFeatures* ff, ModuleTy mod, AlifObject* filename) { // 56
	if (!(mod->type == ModK_::ModuleK or mod->type == ModK_::InteractiveK)) {
		return 1;
	}

	AlifSizeT n = ASDL_SEQ_LEN(mod->V.module.body);
	if (n == 0) {
		return 1;
	}

	AlifSizeT i = 0;
	if (alifAST_getDocString(mod->V.module.body) != nullptr) {
		i++;
	}

	for (; i < n; i++) {
		StmtTy s = (StmtTy)ASDL_SEQ_GET(mod->V.module.body, i);

		/* The only things that can precede a future statement
		 *  are another future statement and a doc string.
		 */

		if (s->type == StmtK_::ImportFromK and s->V.importFrom.level == 0) {
			Identifier modname = s->V.importFrom.module;
			if (modname and
				alifUStr_equalToASCIIString(modname, "__future__")) {
				if (!future_checkFeatures(ff, s, filename)) {
					return 0;
				}
				ff->location = SRC_LOCATION_FROM_AST(s);
			}
			else {
				return 1;
			}
		}
		else {
			return 1;
		}
	}
	return 1;
}


AlifIntT alifFuture_fromAST(ModuleTy _mod, AlifObject* _filename,
	AlifFutureFeatures* _ff) { // 101
	_ff->features = 0;
	_ff->location = { -1, -1, -1, -1 };

	if (!future_parse(_ff, _mod, _filename)) {
		return 0;
	}
	return 1;
}
