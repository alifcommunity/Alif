#pragma once


#include "AlifCore_FileUtils.h"
#include "AlifCore_GlobalObjects.h"



/* --- Characters Type APIs ----------------------------------------------- */
extern AlifIntT _alifUStr_isXIDStart(AlifUCS4); // 18
extern AlifIntT _alifUStr_isXIDContinue(AlifUCS4); // 19
extern AlifIntT _alifUStr_toLowerFull(AlifUCS4, AlifUCS4*); // 20

extern AlifIntT _alifUStr_toUpperFull(AlifUCS4, AlifUCS4*); // 22

extern AlifIntT _alifUStr_isCaseIgnorable(AlifUCS4); // 24
extern AlifIntT _alifUStr_isCased(AlifUCS4); // 25

/* --- Unicode API -------------------------------------------------------- */

void _alifUStr_exactDealloc(AlifObject*); // 34

AlifObject* _alifUStr_copy(AlifObject*); // 40

extern void _alifUStr_fastFill(AlifObject*, AlifSizeT, AlifSizeT, AlifUCS4); // 45

extern void _alifUStr_fastCopyCharacters(AlifObject*, AlifSizeT,
	AlifObject*, AlifSizeT, AlifSizeT); // 55



extern AlifObject* _alifUStr_fromASCII(const char*, AlifSizeT); // 65

extern AlifUCS4 _alifUStr_findMaxChar(AlifObject*, AlifSizeT, AlifSizeT); // 71

extern AlifIntT _alifUStr_formatAdvancedWriter(AlifUStrWriter*, AlifObject*,
	AlifObject*, AlifSizeT, AlifSizeT); // 80

ALIFAPI_FUNC(AlifObject*) _alifUStr_asUTF8String(AlifObject*, const char*); // 98

ALIFAPI_FUNC(AlifObject*) _alifUStr_encodeUTF32(AlifObject*, const char*, AlifIntT); // 105

ALIFAPI_FUNC(AlifObject*) _alifUStr_encodeUTF16(AlifObject*, const char*, AlifIntT); // 127

ALIFAPI_FUNC(AlifObject*) alifUStr_decodeUStrEscapeInternal(const char*, AlifSizeT,
	const char*, AlifSizeT*, const char**); // 144



extern AlifObject* _alifUStr_asLatin1String(AlifObject*, const char*); // 164


extern AlifObject* _alifUStr_asASCIIString(AlifObject*, const char*); // 170


ALIFAPI_FUNC(AlifObject*) _alifUStr_transformDecimalAndSpaceToASCII(AlifObject*); // 201

ALIFAPI_FUNC(AlifObject*) alifUStr_joinArray(AlifObject*, AlifObject* const*, AlifSizeT); // 206


extern AlifSizeT _alifUStr_insertThousandsGrouping(AlifUStrWriter*, AlifSizeT, AlifObject*,
	AlifSizeT, AlifSizeT, AlifSizeT, const char*, AlifObject*, AlifUCS4*); // 240



ALIFAPI_FUNC(AlifIntT) _alifUStr_equalToASCIIString(AlifObject*, const char*); // 224

extern AlifObject* _alifUStr_xStrip(AlifObject*, AlifIntT, AlifObject*); // 230

ALIFAPI_FUNC(AlifIntT) _alifUStr_equal(AlifObject*, AlifObject*); // 257

ALIFAPI_FUNC(AlifSizeT) _alifUStr_scanIdentifier(AlifObject*); // 263

extern AlifStatus alifUStr_initGlobalObjects(AlifInterpreter*); // 268

ALIFAPI_FUNC(void) _alifUStr_internMortal(AlifInterpreter*, AlifObject**); // 280
ALIFAPI_FUNC(void) _alifUStr_internImmortal(AlifInterpreter*, AlifObject**); // 281


extern void _alifUStr_internStatic(AlifInterpreter*, AlifObject**); // 285


class AlifUnicodeRuntimeIds { // 289
public:
	AlifMutex mutex{};
	AlifSizeT nextIndex{};
};

class AlifUnicodeRuntimeState { // 296
public:
	AlifUnicodeRuntimeIds ids{};
};


class AlifUnicodeFSCodec { // 302
public:
	char* encoding{};   // Filesystem encoding (encoded to UTF-8)
	AlifIntT utf8{};         // encoding=="utf-8"?
	char* errors{};     // Filesystem errors (encoded to UTF-8)
	AlifErrorHandler_ errorHandler{};
};

class AlifUnicodeIDs { // 309
public:
	AlifSizeT size{};
	AlifObject** array{};
};

class AlifUnicodeState { // 314
public:
	class AlifUnicodeFSCodec fsCodec {};

	//AlifUnicodeNameCAPI* ucnhashCapi{};

	class AlifUnicodeIDs ids {};
};

ALIFAPI_FUNC(const char*) _alifUStr_asUTF8NoNUL(AlifObject*); // 327
