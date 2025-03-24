#include "alif.h"

#include "AlifCore_Call.h"
#include "AlifCore_Codecs.h"
#include "AlifCore_Interpreter.h"
#include "AlifCore_Long.h"
#include "AlifCore_Object.h"
#include "AlifCore_Errors.h"
#include "AlifCore_State.h"

#include "_IOModule.h"










typedef class TextIO TextIO;


#include "clinic/TextIO.cpp.h" // 33




































/* TextIOWrapper */

typedef AlifObject* (*EncodeFuncT)(AlifObject*, AlifObject*); // 655

class TextIO { // 658
public:
	ALIFOBJECT_HEAD{};
	AlifIntT ok{}; /* initialized? */
	AlifIntT detached{};
	AlifSizeT chunkSize{};
	AlifObject* buffer{};
	AlifObject* encoding{};
	AlifObject* encoder{};
	AlifObject* decoder{};
	AlifObject* readnl{};
	AlifObject* errors{};
	const char* writenl; /* ASCII-encoded; nullptr stands for \n */
	char line_buffering;
	char write_through;
	char readuniversal;
	char readtranslate;
	char writetranslate;
	char seekable;
	char has_read1;
	char telling;
	char finalizing;
	EncodeFuncT encodeFunc{};
	char encodingStartOfStream{};
	AlifObject* decoded_chars{};
	AlifSizeT decodedCharsUsed{};
	AlifObject* pendingBytes{};
	AlifSizeT pendingBytesCount{};
	AlifObject* snapshot{};
	double b2cratIO{};

	AlifObject* raw{};

	AlifObject* weakRefList{};
	AlifObject* dict{};

	AlifIOState* state{};
};
