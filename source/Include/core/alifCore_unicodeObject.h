#pragma once


/* ----------- Other API ----------- */

struct AlifUnicodeRuntimeIDs {
	AlifThreadTypeLock lock;
	AlifSizeT nextIndex;
};

struct AlifUnicodeRuntimeState {
	struct AlifUnicodeRuntimeIDs ids;
};
