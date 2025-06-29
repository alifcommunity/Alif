#include "alif.h"

#include "AlifCore_Call.h"


#include "AlifCore_ImportDL.h"



static const char* const _asciiOnlyPrefix_ = "alifInit"; // 38
static const char* const _nonasciiPrefix_ = "alifInitU"; // 39







void _alifExtModule_loaderInfoClear(AlifExtModuleLoaderInfo* info) { // 103
	ALIF_CLEAR(info->filename);
#ifndef _WINDOWS
	ALIF_CLEAR(info->filenameEncoded);
#endif
	ALIF_CLEAR(info->name);
	ALIF_CLEAR(info->nameEncoded);
}




AlifIntT _alifExtModule_loaderInfoInitForBuiltin(AlifExtModuleLoaderInfo* _info,
	AlifObject* _name) { // 172
	AlifObject* name_encoded = alifUStr_asEncodedString(_name, "utf8", nullptr); //* alif
	if (name_encoded == nullptr) {
		return -1;
	}

	*_info = {
		.name = ALIF_NEWREF(_name),
		.nameEncoded = name_encoded,
		/* We won't need filename. */
		.path = _name,
		.origin = AlifExtModuleOrigin::Alif_Ext_Module_Origin_BUILTIN,
		.hookPrefix = _asciiOnlyPrefix_,
		.newcontext = nullptr,
	};
	return 0;
}







void _alifExtModule_loaderResultClear(AlifExtModuleLoaderResult* res) { // 239
	*res = {0};
}









AlifIntT _alifImport_runModInitFunc(AlifModInitFunction p0,
	AlifExtModuleLoaderInfo* info, AlifExtModuleLoaderResult* p_res) { // 416
	AlifExtModuleLoaderResult res = {
		.kind = AlifExtModuleKind::Alif_Ext_Module_Kind_UNKNOWN,
	};

	/* Call the module init function. */

	const char* oldcontext = _alifImport_swapPackageContext(info->newcontext);
	AlifObject* m = p0();
	_alifImport_swapPackageContext(oldcontext);

	/* Validate the result (and populate "res". */

	if (m == nullptr) {
		res.kind = AlifExtModuleKind::Alif_Ext_Module_Kind_SINGLEPHASE;
		if (alifErr_occurred()) {
			//alifExtModuleLoader_resultSetError(
			//	&res, Alif_Ext_Module_Loader_Result_EXCEPTION);
		}
		else {
			//alifExtModuleLoader_resultSetError(
			//	&res, Alif_Ext_Module_Loader_Result_ERR_MISSING);
		}
		goto error;
	}
	else if (alifErr_occurred()) {
		res.kind = AlifExtModuleKind::Alif_Ext_Module_Kind_SINGLEPHASE;
		//alifExtModuleLoader_resultSetError(
		//	&res, Alif_Ext_Module_Loader_Result_ERR_UNREPORTED_EXC);
		m = nullptr;
		goto error;
	}

	if (ALIF_IS_TYPE(m, nullptr)) {
		//alifExtModuleLoader_resultSetError(
		//	&res, Alif_Ext_Module_Loader_Result_ERR_UNINITIALIZED);
		m = nullptr; /* prevent segfault in DECREF */
		goto error;
	}

	if (ALIFOBJECT_TYPECHECK(m, &_alifModuleDefType_)) {
		res.kind = AlifExtModuleKind::Alif_Ext_Module_Kind_MULTIPHASE;
		res.def = (AlifModuleDef*)m;
	}
	else if (info->hookPrefix == _nonasciiPrefix_) {
		res.kind = AlifExtModuleKind::Alif_Ext_Module_Kind_MULTIPHASE;
		//alifExtModuleLoader_resultSetError(
		//	&res, Alif_Ext_Module_Loader_Result_ERR_NONASCII_NOT_MULTIPHASE);
		goto error;
	}
	else {
		res.kind = AlifExtModuleKind::Alif_Ext_Module_Kind_SINGLEPHASE;
		res.module = m;

		if (!ALIFMODULE_CHECK(m)) {
			//alifExtModuleLoader_resultSetError(
			//	&res, Alif_Ext_Module_Loader_Result_ERR_NOT_MODULE);
			goto error;
		}

		res.def = _alifModule_getDef(m);
		if (res.def == nullptr) {
			//alifErr_clear();
			//alifExtModuleLoader_resultSetError(
			//	&res, Alif_Ext_Module_Loader_Result_ERR_MISSING_DEF);
			goto error;
		}
	}

	*p_res = res;
	return 0;

error:
	ALIF_CLEAR(res.module);
	res.def = nullptr;
	*p_res = res;
	p_res->err = &p_res->err_;
	return -1;
}
