#include "alif.h"
#include "AlifCore_Interpreter.h"
#include "AlifCore_State.h"
#include "AlifCore_ImportDL.h"

#include <sys/types.h>
#include <sys/stat.h>

#if defined(__NetBSD__)
#include <sys/param.h>
#if (NetBSD < 199712)
#include <nlist.h>
#include <link.h>
#define dlerror() "error in dynamic linking"
#endif
#endif /* NetBSD */

#ifdef HAVE_DLFCN_H
#include <dlfcn.h>
#endif

#if (defined(__OpenBSD__) || defined(__NetBSD__)) && !defined(__ELF__)
#define LEAD_UNDERSCORE "_"
#else
#define LEAD_UNDERSCORE ""
#endif


const char *_alifImportDynLoadFiletab_[] = {
#ifdef __CYGWIN__
	".dll",
#else  /* !__CYGWIN__ */
	"." SOABI ".so",
#ifdef ALT_SOABI
	"." ALT_SOABI ".so",
#endif
	".abi" ALIF_ABI_STRING ".so",
	".so",
#endif  /* __CYGWIN__ */
	nullptr,
};


DLFuncPtr _alifImport_findSharedFuncptr(const char *prefix,
	const char *shortname, const char *pathname, FILE *fp) {
	DLFuncPtr p{};
	void *handle{};
	char funcname[258]{};
	char pathbuf[260]{};
	AlifIntT dlopenflags=0;

	if (strchr(pathname, '/') == nullptr) {
		alifOS_snprintf(pathbuf, sizeof(pathbuf), "./%-.255s", pathname);
		pathname = pathbuf;
	}

	alifOS_snprintf(funcname, sizeof(funcname),
		LEAD_UNDERSCORE "%.20s_%.200s", prefix, shortname);

	if (fp != nullptr) {
		class AlifStatStruct status{};
		if (_alif_fStat(fileno(fp), &status) == -1)
			return nullptr;
	}

	dlopenflags = _alifImport_getDLOpenFlags(_alifInterpreter_get());

	handle = dlopen(pathname, dlopenflags);

	if (handle == nullptr) {
		AlifObject* modName{};
		AlifObject* path{};
		AlifObject* error_ob{};
		const char *error = dlerror();
		if (error == nullptr)
			error = "unknown dlopen() error";
		error_ob = alifUStr_decodeLocale(error, "surrogateescape");
		if (error_ob == nullptr)
			return nullptr;
		modName = alifUStr_fromString(shortname);
		if (modName == nullptr) {
			ALIF_DECREF(error_ob);
			return nullptr;
		}
		path = alifUStr_decodeFSDefault(pathname);
		if (path == nullptr) {
			ALIF_DECREF(error_ob);
			ALIF_DECREF(modName);
			return nullptr;
		}
		alifErr_setImportError(error_ob, modName, path);
		ALIF_DECREF(error_ob);
		ALIF_DECREF(modName);
		ALIF_DECREF(path);
		return nullptr;
	}
	p = (DLFuncPtr)dlsym(handle, funcname);
	return p;
}
