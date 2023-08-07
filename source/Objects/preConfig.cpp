#include "Alif.h"
#include "alifCore_initConfig.h"    // AlifArgv
#include "alifCore_alifCycle.h"
#include "alifCore_alifMem.h"


AlifStatus alifArgv_asWstrList(const AlifArgv* _args, AlifWideStringList* _list)
{
	AlifWideStringList wArgv = { .length = 0, .items = nullptr };
	if (_args->useCharArgv) {

	}
	else {
		wArgv.length = _args->argc;
		wArgv.items = (wchar_t**)_args->wcharArgv;

		*_list = wArgv;
	}


	return ALIFSTATUS_OK();
}

/* ___________ AlifPreConfig ___________ */

/* يجب مراجعة الدالتين التاليتين
	وذلك بسبب أنه قد تكوم استخدمت للتوافقية بين النسخ
	القديمة والجديدة فقط وهذا ما لا نحتاجه */

void alifPreConfig_initCompatConfig(AlifPreConfig* _config)
{
	_config->configInit = 1; // INIT_COMPAT = 1, INIT_ALIF = 2, INIT_ISOLATED = 3
	_config->parseArgv = 0;
	_config->isolated = -1;
	_config->useEnvironment = -1;
	_config->configureLocale = 1;
	_config->utf8Mode = 0;
	_config->cppLocale = 0;
	_config->allocator = ALIFMEM_ALLOCATOR_NOT_SET;
#ifdef MS_WINDOWS
	_config->EncodingLegacyWindowsFS = -1;
#endif
}

void alifPreConfig_initAlifConfig(AlifPreConfig* _config)
{
	alifPreConfig_initCompatConfig(_config);

	_config->configInit = 2; // INIT_COMPAT = 1, INIT_ALIF = 2, INIT_ISOLATED = 3
	_config->isolated = 0;
	_config->parseArgv = 1;
	_config->useEnvironment = 1;
	_config->cppLocale = -1;
	_config->utf8Mode = -1;
#ifdef MS_WINDOWS
	_config->EncodingLegacyWindowsFS = 0;
#endif
}


AlifStatus alifPreConfig_initFromPreConfig(AlifPreConfig* _config)
{
	alifPreConfig_initAlifConfig(_config);
	return ALIFSTATUS_OK();
}


static void preConfig_getGlobalVars(AlifPreConfig* _config)
{
	if (_config->configInit != 1) // AlifConfig_INIT_COMPAT = 1
	{
		/* Alif and Isolated configuration ignore global variables */
		return;
	}

#define COPY_FLAG(ATTR, VALUE) \
    if (_config->ATTR < 0) { \
        _config->ATTR = VALUE; \
    }
#define COPY_NOT_FLAG(ATTR, VALUE) \
    if (_config->ATTR < 0) { \
        _config->ATTR = !(VALUE); \
    }

	ALIF_COMP_DIAG_PUSH
	ALIF_COMP_DIAG_IGNORE_DEPR_DECLS
	COPY_FLAG(isolated, alifIsolatedFlag);
	COPY_NOT_FLAG(useEnvironment, alifIgnoreEnvironmentFlag);
	if (alifUTF8Mode > 0) {
		_config->utf8Mode = alifUTF8Mode;
	}
#ifdef MS_WINDOWS
	//COPY_FLAG(legacyWindowsFsEncoding, alifLegacyWindowsFSEncodingFlag);
#endif
	ALIF_COMP_DIAG_POP

#undef COPY_FLAG
#undef COPY_NOT_FLAG
}


/* Read the configuration from:

   - command line arguments
   - environment variables
   - Alif_xxx global configuration variables
   - the LC_CTYPE locale
*/
AlifStatus alifPreConfig_read(AlifPreConfig* _config, const AlifArgv* _args)
{
	AlifStatus status{};

	//status = alifRuntime_initialize();
	//if (ALIFSTATUS_EXCEPTION(status)) {
	//	return status;
	//}

	preConfig_getGlobalVars(_config);

	/* Copy LC_CTYPE locale, since it's modified later */
	const char* loc = std::setlocale(LC_CTYPE, nullptr);
	if (loc == nullptr) {
		return ALIFSTATUS_ERR("فشلت عملية تهيئة LC_CTYPE");
	}
	char* initCtypeLocale = alifMem_rawStrDup(loc); // يوجد مشكلة في الذاكرة ويجب التحقق ما إذا تم حلها
	if (initCtypeLocale == nullptr) {
		return ALIFSTATUS_NO_MEMORY();
	}

	/* Save the config to be able to restore it if encodings change */
	AlifPreConfig saveConfig{};

	status = alifPreConfig_initFromPreConfig(&saveConfig);
	if (ALIFSTATUS_EXCEPTION(status)) {
		return status;
	}

	/* Set LC_CTYPE to the user preferred locale */
	if (_config->configureLocale) {
		alif_setLocaleFromEnv(LC_CTYPE);
	}

	AlifPreConfig saveRuntimeConfig{};
	//preConfig_copy(&saveRuntimeConfig, &alifRuntime.preconfig);

	//AlifPreCmdline cmdLine = ALIFPRECMDLINE_INIT;
	//int locale_coerced = 0;
	int loops = 0;

	while (1) {
		int utf8Mode = _config->utf8Mode;

		/* Watchdog to prevent an infinite loop */
		loops++;
		if (loops == 3) {
			status = ALIFSTATUS_ERR("تم تغيير الترميز مرتين اثناء تهيئة الملف");
			goto done;
		}

		/* bpo-34207: Py_DecodeLocale() and Py_EncodeLocale() depend
		   on the utf8_mode and legacy_windows_fs_encoding members
		   of _PyRuntime.preconfig. */
		//preConfig_copy(&alifRuntime.preConfig, _config);

		if (_args) {
			// Set command line arguments at each iteration. If they are bytes
			// strings, they are decoded from the new encoding.
			//status = alifPreCmdLine_setArgv(&cmdLine, _args);
			if (ALIFSTATUS_EXCEPTION(status)) {
				goto done;
			}
		}

		//status = preConfig_read(_config, &cmdLine);
		if (ALIFSTATUS_EXCEPTION(status)) {
			goto done;
		}

		int encoding_changed = 0;
		//if (_config->coerceCLocale && !localeCoerced) {
		//	localeCoerced = 1;
		//	alif_coerceLegacyLocale(0);
		//	encoding_changed = 1;
		//}

		if (utf8Mode == -1) {
			if (_config->utf8Mode == 1) {
				/* UTF-8 Mode enabled */
				encoding_changed = 1;
			}
		}
		else {
			if (_config->utf8Mode != utf8Mode) {
				encoding_changed = 1;
			}
		}

		if (!encoding_changed) {
			break;
		}


		//int newUtf8Mode = _config->utf8Mode;
		//int newCoerceCLocale = _config->coerceCLocale;
		//preConfig_copy(_config, &saveConfig);
		//_config->utf8Mode = newUtf8Mode;
		//_config->coerceCLocale = newCoerceCLocale;

		/* The encoding changed: read again the configuration
		   with the new encoding */
	}
	status = ALIFSTATUS_OK();

done:
	// Revert side effects
	//setlocale(LC_CTYPE, initCtypeLocale);
	//alifMem_rawFree(initCtypeLocale);
	//prConfig_copy(&alifRuntime.preconfig, &saveRuntimeConfig);
	//alifPreCmdLine_clear(&cmdLine);
	return status;
}
