#include <alif.h>
#include <marshal.h>

#include "AlifCore_FileUtils.h"
#include <AlifCore_Import.h>

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifndef _WINDOWS
#  include <unistd.h>
#endif

static const struct Frozen _noModules_[] = {
	{0, 0, 0} /* sentinel */
};
static const struct ModuleAlias _aliases_[] = {
	{0, 0} /* sentinel */
};

const struct Frozen* _alifImportFrozenBootstrap_;
const struct Frozen* _alifImportFrozenStdlib_;
const struct Frozen* _alifImportFrozenTest_;
const struct Frozen* _alifImportFrozenModules_;
const struct ModuleAlias* _alifImportFrozenAliases_;

static const char _header_[] =
"/* هذا الملف تم إنشائه بشكل تلقائي عن طريق البرنامج source/FreezeModule/FreezeModule.cpp */";


static void runtime_init(void) {
	AlifConfig config;
	alifConfig_initIsolatedConfig(&config);

	config.siteImport = 0;

	AlifStatus status;
	status = alifConfig_setString(&config, &config.programName,
		L"./FreezeModule");
	if (alifStatus_exception(status)) {
		alifConfig_clear(&config);
		alif_exitStatusException(status);
	}

	/* Don't install importlib, since it could execute outdated bytecode. */
	config.installImportLib = 0;
	config.initMain = 0;

	status = alif_initFromConfig(&config);
	alifConfig_clear(&config);
	if (alifStatus_exception(status)) {
		alif_exitStatusException(status);
	}
}

static const char* read_text(const char* inpath) {
	FILE* infile = fopen(inpath, "rb");
	if (infile == nullptr) {
		fprintf(stderr, "لا يمكن فتح '%s' للقراءة\n", inpath);
		return nullptr;
	}

	class AlifStatStruct stat;
	if (_alifFStat_noraise(fileno(infile), &stat)) {
		fprintf(stderr, "cannot fstat '%s'\n", inpath);
		fclose(infile);
		return nullptr;
	}
	size_t text_size = (size_t)stat.st_size;

	char* text = (char*)malloc(text_size + 1);
	if (text == nullptr) {
		fprintf(stderr, "could not allocate %ld bytes\n", (long)text_size);
		fclose(infile);
		return nullptr;
	}
	size_t n = fread(text, 1, text_size, infile);
	fclose(infile);

	if (n < text_size) {
		fprintf(stderr, "read too short: got %ld instead of %ld bytes\n",
			(long)n, (long)text_size);
		free(text);
		return nullptr;
	}

	text[text_size] = '\0';
	return (const char*)text;
}

static AlifObject* compile_andMarshal(const char* name, const char* text) {
	char* filename = (char*)malloc(strlen(name) + 10);
	if (filename == nullptr) {
		//return alifErr_noMemory();
		return nullptr;
	}
	sprintf(filename, "<frozen %s>", name);
	AlifObject* code = alif_compileStringExFlags(text, filename,
		ALIF_FILE_INPUT, nullptr, 0);
	free(filename);
	if (code == nullptr) {
		return nullptr;
	}

	AlifObject* marshalled = alifMarshal_writeObjectToString(code, ALIF_MARSHAL_VERSION);
	ALIF_CLEAR(code);
	if (marshalled == nullptr) {
		return nullptr;
	}

	return marshalled;
}

static char* get_varname(const char* name, const char* prefix) {
	size_t n = strlen(prefix);
	char* varname = (char*)malloc(strlen(name) + n + 1);
	if (varname == nullptr) {
		return nullptr;
	}
	(void)strcpy(varname, prefix);
	for (size_t i = 0; name[i] != '\0'; i++) {
		if (name[i] == '.') {
			varname[n++] = '_';
		}
		else {
			varname[n++] = name[i];
		}
	}
	varname[n] = '\0';
	return varname;
}

static void write_code(FILE* outfile, AlifObject* marshalled, const char* varname) {
	unsigned char* data = (unsigned char*)ALIFBYTES_AS_STRING(marshalled);
	size_t data_size = ALIFBYTES_GET_SIZE(marshalled);

	fprintf(outfile, "const unsigned char %s[] = {\n", varname);
	for (size_t n = 0; n < data_size; n += 16) {
		size_t i, end = ALIF_MIN(n + 16, data_size);
		fprintf(outfile, "    ");
		for (i = n; i < end; i++) {
			fprintf(outfile, "%u,", (unsigned int)data[i]);
		}
		fprintf(outfile, "\n");
	}
	fprintf(outfile, "};\n");
}

static int write_frozen(const char* outpath, const char* inpath,
	const char* name, AlifObject* marshalled) {
	FILE* outfile = fopen(outpath, "w");
	if (outfile == nullptr) {
		fprintf(stderr, "cannot open '%s' for writing\n", outpath);
		return -1;
	}

	fprintf(outfile, "%s\n", _header_);
	char* arrayname = get_varname(name, "_ALIF_M__");
	if (arrayname == nullptr) {
		fprintf(stderr, "memory error: could not allocate varname\n");
		fclose(outfile);
		return -1;
	}
	write_code(outfile, marshalled, arrayname);
	free(arrayname);

	if (ferror(outfile)) {
		fprintf(stderr, "error when writing to '%s'\n", outpath);
		fclose(outfile);
		return -1;
	}
	fclose(outfile);
	return 0;
}

int main(int argc, char* argv[]) {
	const char* name, * inpath, * outpath;

	_alifImportFrozenBootstrap_ = _noModules_;
	_alifImportFrozenStdlib_ = _noModules_;
	_alifImportFrozenTest_ = _noModules_;
	_alifImportFrozenModules_ = nullptr;
	_alifImportFrozenAliases_ = _aliases_;

	if (argc != 4) {
		fprintf(stderr, "need to specify the name, input and output paths\n");
		return 2;
	}
	name = argv[1];
	inpath = argv[2];
	outpath = argv[3];

	runtime_init();

	const char* text = read_text(inpath);
	if (text == nullptr) {
		goto error;
	}

	AlifObject* marshalled; marshalled = compile_andMarshal(name, text);
	free((char*)text);
	if (marshalled == nullptr) {
		goto error;
	}

	int res; res = write_frozen(outpath, inpath, name, marshalled);
	ALIF_DECREF(marshalled);
	if (res != 0) {
		goto error;
	}

	alif_finalize();
	return 0;

error:
	alifErr_print();
	alif_finalize();
	return 1;
}
