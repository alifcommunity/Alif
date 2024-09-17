#pragma once






class AlifBytesObject { // 5
public:
	ALIFOBJECT_VAR_HEAD;
	AlifHashT Hash{}; // Deprecated
	char val[1]{};

	/* Invariants:
	 *     val contains space for 'ob_size+1' elements.
	 *     val[size] == 0.
	 *     hash is the hash of the byte string or -1 if not computed yet.
	 */
};


// 20
#define ALIFBYTES_CAST(op) \
    (ALIF_CAST(AlifBytesObject*, op))

static inline char* alifBytes_asString(AlifObject* _op) { // 23
	return ALIFBYTES_CAST(_op)->val;
}
#define ALIFBYTES_AS_STRING(op) alifBytes_asString(ALIFOBJECT_CAST(op))
