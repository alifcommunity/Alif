#include "alif.h"

#include "AlifCore_AST.h"
#include "AlifCore_Call.h"
#include "AlifCore_FileUtils.h"
#include "AlifCore_Frame.h"
#include "AlifCore_Interpreter.h"
#include "AlifCore_Parser.h"
#include "AlifCore_State.h"
#include "AlifCore_SysModule.h"
#include "AlifCore_Traceback.h"




#include "clinic/Traceback.cpp.h"





static AlifObject* tb_createRaw(AlifTracebackObject* next, AlifFrameObject* frame,
	AlifIntT lasti, AlifIntT lineno) { // 43
	AlifTracebackObject* tb{};
	if ((next != nullptr and !ALIFTRACEBACK_CHECK(next)) or
		frame == nullptr or !ALIFFRAME_CHECK(frame)) {
		//ALIFERR_BADINTERNALCALL();
		return nullptr;
	}
	tb = ALIFOBJECT_GC_NEW(AlifTracebackObject, &_alifTraceBackType_);
	if (tb != nullptr) {
		tb->next = (AlifTracebackObject*)ALIF_XNEWREF(next);
		tb->frame = (AlifFrameObject*)ALIF_XNEWREF(frame);
		tb->lasti = lasti;
		tb->lineno = lineno;
		alifObject_gcTrack(tb);
	}
	return (AlifObject*)tb;
}



static AlifObject* tb_newImpl(AlifTypeObject* _type, AlifObject* _tbNext, AlifFrameObject* _tbFrame,
	AlifIntT _tbLasti, AlifIntT _tbLineno) { // 76
	if (_tbNext == ALIF_NONE) {
		_tbNext = nullptr;
	}
	else if (!ALIFTRACEBACK_CHECK(_tbNext)) {
		return alifErr_format(_alifExcTypeError_,
			"expected traceback object or None, got '%s'",
			ALIF_TYPE(_tbNext)->name);
	}

	return tb_createRaw((AlifTracebackObject*)_tbNext, _tbFrame, _tbLasti,
		_tbLineno);
}




static AlifIntT tb_getLineNo(AlifTracebackObject* tb) { // 110
	AlifInterpreterFrame* frame = tb->frame->frame;
	return alifCode_addr2Line(_alifFrame_getCode(frame), tb->lasti);
}




AlifTypeObject _alifTraceBackType_ = { // 209
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "تتبع",
	.basicSize = sizeof(AlifTracebackObject),
	//.dealloc = (Destructor)tb_dealloc,
	.getAttro = alifObject_genericGetAttr,
	.flags = ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_HAVE_GC,
	//.traverse = (TraverseProc)tb_traverse,
	//(Inquiry)tb_clear,
	//.methods = tb_methods,
	//.members = tb_memberlist,
	//.getSet = tb_getsetters,
	.new_ = tb_new,
};


AlifObject* _alifTraceBack_fromFrame(AlifObject* _tbNext, AlifFrameObject* _frame) { // 251
	AlifIntT addr = ALIFINTERPRETERFRAME_LASTI(_frame->frame) * sizeof(AlifCodeUnit);
	return tb_createRaw((AlifTracebackObject*)_tbNext, _frame, addr, -1);
}

AlifIntT alifTraceBack_here(AlifFrameObject* _frame) { // 261
	AlifObject* exc = alifErr_getRaisedException();
	AlifObject* tb = alifException_getTraceback(exc);
	AlifObject* newtb = _alifTraceBack_fromFrame(tb, _frame);
	ALIF_XDECREF(tb);
	if (newtb == nullptr) {
		_alifErr_chainExceptions1(exc);
		return -1;
	}
	alifException_setTraceback(exc, newtb);
	ALIF_XDECREF(newtb);
	alifErr_setRaisedException(exc);
	return 0;
}



static AlifIntT display_sourceLine(AlifObject* f, AlifObject* filename, AlifIntT lineno,
	AlifIntT indent, AlifIntT* truncation, AlifObject** line) { // 417
	AlifIntT fd{};
	AlifIntT i{};
	char* found_encoding{};
	const char* encoding{};
	AlifObject* io;
	AlifObject* binary;
	AlifObject* fob = nullptr;
	AlifObject* lineobj = nullptr;
	AlifObject* res;
	//char buf[MAXPATHLEN + 1]{};
	AlifIntT kind{};
	const void* data{};

	/* open the file */
//	if (filename == nullptr)
//		return 0;
//
//	if (ALIFUSTR_READ_CHAR(filename, 0) == '<') {
//		AlifSizeT len = ALIFUSTR_GET_LENGTH(filename);
//		if (len > 0 and ALIFUSTR_READ_CHAR(filename, len - 1) == '>') {
//			return 0;
//		}
//	}
//
//	io = alifImport_importModule("io");
//	if (io == nullptr) {
//		return -1;
//	}
//
//	binary = _alifObject_callMethod(io, &ALIF_ID(Open), "Os", filename, "rb");
//	if (binary == nullptr) {
//		alifErr_clear();
//
//		binary = _alif_findSourceFile(filename, buf, sizeof(buf), io);
//		if (binary == nullptr) {
//			ALIF_DECREF(io);
//			return -1;
//		}
//	}
//
//	/* use the right encoding to decode the file as unicode */
//	fd = alifObject_asFileDescriptor(binary);
//	if (fd < 0) {
//		ALIF_DECREF(io);
//		ALIF_DECREF(binary);
//		return 0;
//	}
//	found_encoding = _alifTokenizer_findEncodingFilename(fd, filename);
//	if (found_encoding == nullptr)
//		alifErr_clear();
//	encoding = (found_encoding != nullptr) ? found_encoding : "utf-8";
//	/* Reset position */
//	if (lseek(fd, 0, SEEK_SET) == (off_t)-1) {
//		ALIF_DECREF(io);
//		ALIF_DECREF(binary);
//		alifMem_dataFree(found_encoding);
//		return 0;
//	}
//	fob = _alifObject_callMethod(io, &ALIF_ID(TextIOWrapper),
//		"Os", binary, encoding);
//	ALIF_DECREF(io);
//	alifMem_dataFree(found_encoding);
//
//	if (fob == nullptr) {
//		alifErr_clear();
//
//		res = alifObject_callMethodNoArgs(binary, &ALIF_ID(Close));
//		ALIF_DECREF(binary);
//		if (res)
//			ALIF_DECREF(res);
//		else
//			alifErr_clear();
//		return 0;
//	}
//	ALIF_DECREF(binary);
//
//	/* get the line number lineno */
//	for (i = 0; i < lineno; i++) {
//		ALIF_XDECREF(lineobj);
//		lineobj = alifFile_getLine(fob, -1);
//		if (!lineobj) {
//			alifErr_clear();
//			break;
//		}
//	}
//	res = alifObject_callMethodNoArgs(fob, &ALIF_ID(Close));
//	if (res) {
//		ALIF_DECREF(res);
//	}
//	else {
//		alifErr_clear();
//	}
//	ALIF_DECREF(fob);
//	if (!lineobj or !ALIFUSTR_CHECK(lineobj)) {
//		ALIF_XDECREF(lineobj);
//		return -1;
//	}
//
//	if (line) {
//		*line = ALIF_NEWREF(lineobj);
//	}
//
//	/* remove the indentation of the line */
//	kind = ALIFUSTR_KIND(lineobj);
//	data = ALIFUSTR_DATA(lineobj);
//	for (i = 0; i < ALIFUSTR_GET_LENGTH(lineobj); i++) {
//		AlifUCS4 ch = ALIFUSTR_READ(kind, data, i);
//		if (ch != ' ' && ch != '\t' && ch != '\014')
//			break;
//	}
//	if (i) {
//		AlifObject* truncated;
//		truncated = alifUStr_subString(lineobj, i, ALIFUSTR_GET_LENGTH(lineobj));
//		if (truncated) {
//			ALIF_SETREF(lineobj, truncated);
//		}
//		else {
//			alifErr_clear();
//		}
//	}
//
//	if (truncation != nullptr) {
//		*truncation = i - indent;
//	}
//
//	/* Write some spaces before the line */
//	if (_alif_writeIndent(indent, f) < 0) {
//		goto error;
//	}
//
//	/* finally display the line */
//	if (alifFile_writeObject(lineobj, f, ALIF_PRINT_RAW) < 0) {
//		goto error;
//	}
//
//	if (alifFile_writeString("\n", f) < 0) {
//		goto error;
//	}
//
//	ALIF_DECREF(lineobj);
	return 0;
//error:
//	ALIF_DECREF(lineobj);
//	return -1;
}





// 576
#define IS_WHITESPACE(c) (((c) == ' ') || ((c) == '\t') || ((c) == '\f'))
#define TRACEBACK_SOURCE_LINE_INDENT 4

static inline AlifIntT ignore_sourceErrors(void) { // 579
	if (alifErr_occurred()) {
		//if (alifErr_exceptionMatches(_alifExcKeyboardInterrupt_)) {
		//	return -1;
		//}
		alifErr_clear();
	}
	return 0;
}

static AlifIntT tb_displayLine(AlifTracebackObject* tb,
	AlifObject* f, AlifObject* filename, AlifIntT lineno,
	AlifFrameObject* frame, AlifObject* name) { // 590
	if (filename == nullptr or name == nullptr) {
		return -1;
	}

	AlifObject* line = alifUStr_fromFormat("  الملف \"%U\", السطر %d, في %U\n",
		filename, lineno, name);
	if (line == nullptr) {
		return -1;
	}

	AlifIntT res = alifFile_writeObject(line, f, ALIF_PRINT_RAW);
	ALIF_DECREF(line);
	if (res < 0) {
		return -1;
	}

	AlifIntT err = 0;

	AlifIntT truncation = TRACEBACK_SOURCE_LINE_INDENT;
	AlifObject* source_line = nullptr;
	AlifIntT rc = display_sourceLine(
		f, filename, lineno, TRACEBACK_SOURCE_LINE_INDENT,
		&truncation, &source_line);
	if (rc != 0 or !source_line) {
		/* ignore errors since we can't report them, can we? */
		err = ignore_sourceErrors();
	}
	ALIF_XDECREF(source_line);
	return err;
}

static const AlifIntT _tbRecursiveCutoff_ = 3; // 625

static AlifIntT tb_printLineRepeated(AlifObject* f, long cnt) { // 627
	cnt -= _tbRecursiveCutoff_;
	AlifObject* line = alifUStr_fromFormat(
		(cnt > 1)
		? "  [Previous line repeated %ld more times]\n"
		: "  [Previous line repeated %ld more time]\n",
		cnt);
	if (line == nullptr) {
		return -1;
	}
	AlifIntT err = alifFile_writeObject(line, f, ALIF_PRINT_RAW);
	ALIF_DECREF(line);
	return err;
}




static AlifIntT tb_printInternal(AlifTracebackObject* tb, AlifObject* f, long limit) { // 644
	AlifCodeObject* code = nullptr;
	AlifSizeT depth = 0;
	AlifObject* last_file = nullptr;
	AlifIntT last_line = -1;
	AlifObject* last_name = nullptr;
	long cnt = 0;
	AlifTracebackObject* tb1 = tb;
	while (tb1 != nullptr) {
		depth++;
		tb1 = tb1->next;
	}
	while (tb != nullptr and depth > limit) {
		depth--;
		tb = tb->next;
	}
	while (tb != nullptr) {
		code = alifFrame_getCode(tb->frame);
		int tb_lineno = tb->lineno;
		if (tb_lineno == -1) {
			tb_lineno = tb_getLineNo(tb);
		}
		if (last_file == nullptr or
			code->filename != last_file or
			last_line == -1 or tb_lineno != last_line or
			last_name == nullptr or code->name != last_name) {
			if (cnt > _tbRecursiveCutoff_) {
				if (tb_printLineRepeated(f, cnt) < 0) {
					goto error;
				}
			}
			last_file = code->filename;
			last_line = tb_lineno;
			last_name = code->name;
			cnt = 0;
		}
		cnt++;
		if (cnt <= _tbRecursiveCutoff_) {
			if (tb_displayLine(tb, f, code->filename, tb_lineno,
				tb->frame, code->name) < 0) {
				goto error;
			}

			//if (alifErr_checkSignals() < 0) {
			//	goto error;
			//}
		}
		ALIF_CLEAR(code);
		tb = tb->next;
	}
	if (cnt > _tbRecursiveCutoff_) {
		if (tb_printLineRepeated(f, cnt) < 0) {
			goto error;
		}
	}
	return 0;
error:
	ALIF_XDECREF(code);
	return -1;
}


#define ALIFTRACEBACK_LIMIT 1000 // 707


AlifIntT _alifTraceBack_print(AlifObject* _v, const char* header, AlifObject* _f) { // 709
	AlifObject* limitv{};
	long limit = ALIFTRACEBACK_LIMIT;

	if (_v == nullptr) {
		return 0;
	}
	if (!ALIFTRACEBACK_CHECK(_v)) {
		//ALIFERR_BADINTERNALCALL();
		return -1;
	}
	limitv = alifSys_getObject("tracebacklimit");
	if (limitv and ALIFLONG_CHECK(limitv)) {
		AlifIntT overflow{};
		limit = alifLong_asLongAndOverflow(limitv, &overflow);
		if (overflow > 0) {
			limit = LONG_MAX;
		}
		else if (limit <= 0) {
			return 0;
		}
	}

	if (alifFile_writeString(header, _f) < 0) {
		return -1;
	}

	if (tb_printInternal((AlifTracebackObject*)_v, _f, limit) < 0) {
		return -1;
	}

	return 0;
}
