#include "alif.h"

#include "AlifCore_FileUtils.h"     
#include "AlifCore_Object.h"       
#include "AlifCore_Errors.h"     

#include <stdbool.h>              // bool
#ifdef HAVE_UNISTD_H
#  include <unistd.h>             // lseek()
#endif
#ifdef HAVE_SYS_TYPES_H
#  include <sys/types.h>
#endif
#ifdef HAVE_IO_H
#  include <io.h>
#endif
#ifdef HAVE_FCNTL_H
#  include <fcntl.h>              // open()
#endif

#include "_IOModule.h"



// 46
#if BUFSIZ < (8*1024)
#  define SMALLCHUNK (8*1024)
#elif (BUFSIZ >= (2 << 25))
#  error "unreasonable BUFSIZ > 64 MiB defined"
#else
#  define SMALLCHUNK BUFSIZ
#endif

#define LARGE_BUFFER_CUTOFF_SIZE 65536


class FileIO { // 64
public:
	ALIFOBJECT_HEAD{};
	AlifIntT fd{};
	AlifUIntT created : 1;
	AlifUIntT readable : 1;
	AlifUIntT writable : 1;
	AlifUIntT appending : 1;
	signed int seekable : 2; /* -1 means unknown */
	AlifUIntT closefd : 1;
	char finalizing{};
	AlifStatStruct* statAtOpen{};
	AlifObject* weakRefList{};
	AlifObject* dict{};
};


#define ALIFFILEIO_CAST(_op) ALIF_CAST(FileIO*, (_op)) // 86

static AlifObject* portable_lseek(FileIO* _self, AlifObject* _posObj,
	AlifIntT _whence, bool _suppressPipeError); // 89


AlifIntT _alifFileIO_closed(AlifObject* _self) { // 91
	return (ALIFFILEIO_CAST(_self)->fd < 0);
}

static AlifObject* fileIO_deallocWarn(AlifObject* op, AlifObject* source) { // 100
	FileIO* self = ALIFFILEIO_CAST(op);
	if (self->fd >= 0 and self->closefd) {
		AlifObject* exc = alifErr_getRaisedException();
		//if (alifErr_resourceWarning(source, 1, "unclosed file %R", source)) {
		//	if (alifErr_exceptionMatches(_alifExcWarning_))
		//		alifErr_writeUnraisable((AlifObject*)self);
		//}
		alifErr_setRaisedException(exc);
	}
	return ALIF_NONE;
}

static AlifIntT internal_close(FileIO* self) { // 117
	AlifIntT err = 0;
	AlifIntT save_errno = 0;
	if (self->fd >= 0) {
		AlifIntT fd = self->fd;
		self->fd = -1;
		/* fd is accessible and someone else may have closed it */
		ALIF_BEGIN_ALLOW_THREADS
		ALIF_BEGIN_SUPPRESS_IPH
			err = close(fd);
		if (err < 0)
			save_errno = errno;
		ALIF_END_SUPPRESS_IPH
		ALIF_END_ALLOW_THREADS
	}
	if (err < 0) {
		errno = save_errno;
		//alifErr_setFromErrno(_alifExcOSError_);
		return -1;
	}
	return 0;
}


static AlifObject* _ioFileIO_closeImpl(FileIO* self, AlifTypeObject* cls) { // 154
	AlifObject* res{};
	AlifIntT rc{};
	AlifIOState* state = getIOState_byCls(cls);
	res = alifObject_callMethodOneArg((AlifObject*)state->alifRawIOBaseType,
		&ALIF_STR(Close), (AlifObject*)self);
	if (!self->closefd) {
		self->fd = -1;
		return res;
	}

	AlifObject* exc = nullptr;
	if (res == nullptr) {
		exc = alifErr_getRaisedException();
	}
	if (self->finalizing) {
		AlifObject* r = fileIO_deallocWarn((AlifObject*)self, (AlifObject*)self);
		if (r) {
			ALIF_DECREF(r);
		}
		else {
			alifErr_clear();
		}
	}
	rc = internal_close(self);
	if (res == nullptr) {
		_alifErr_chainExceptions1(exc);
	}
	if (rc < 0) {
		ALIF_CLEAR(res);
	}
	return res;
}


static AlifObject* fileio_new(AlifTypeObject* type,
	AlifObject* args, AlifObject* kwds) { // 191
	FileIO* self = (FileIO*)type->alloc(type, 0);
	if (self == nullptr) {
		return nullptr;
	}

	self->fd = -1;
	self->created = 0;
	self->readable = 0;
	self->writable = 0;
	self->appending = 0;
	self->seekable = -1;
	self->statAtOpen = nullptr;
	self->closefd = 1;
	self->weakRefList = nullptr;
	return (AlifObject*)self;
}



static AlifIntT _ioFileIO___init__Impl(FileIO* self, AlifObject* nameobj, const char* mode,
	AlifIntT closefd, AlifObject* opener) { // 239
#ifdef _WINDOWS
	wchar_t* widename = nullptr;
#else
	const char* name = nullptr;
#endif
	AlifObject* stringobj = nullptr;
	const char* s{};
	AlifIntT ret = 0;
	AlifIntT rwa = 0, plus = 0;
	AlifIntT flags = 0;
	AlifIntT fd = -1;
	AlifIntT fd_is_own = 0;
#ifdef O_CLOEXEC
	AlifIntT* atomic_flag_works = &_Py_open_cloexec_works;
#elif !defined(_WINDOWS)
	AlifIntT* atomic_flag_works = nullptr;
#endif
	AlifIntT fstat_result;
	AlifIntT async_err = 0;

	if (self->fd >= 0) {
		if (self->closefd) {
			/* Have to close the existing file first. */
			if (internal_close(self) < 0)
				return -1;
		}
		else
			self->fd = -1;
	}

	if (ALIFBOOL_CHECK(nameobj)) {
		//if (alifErr_warnEx(_alifExcRuntimeWarning_,
		//	"bool is used as a file descriptor", 1))
		//{
		//	return -1;
		//}
	}
	fd = alifLong_asInt(nameobj);
	if (fd < 0) {
		//if (!alifErr_occurred()) {
		//	alifErr_setString(_alifExcValueError_,
		//		"negative file descriptor");
		//	return -1;
		//}
		alifErr_clear();
	}

	if (fd < 0) {
#ifdef _WINDOWS
		if (!alifUStr_fsDecoder(nameobj, &stringobj)) {
			return -1;
		}
		widename = alifUStr_asWideCharString(stringobj, nullptr);
		if (widename == nullptr)
			return -1;
#else
		if (!alifUStr_fsConverter(nameobj, &stringobj)) {
			return -1;
		}
		name = ALIFBYTES_AS_STRING(stringobj);
#endif
	}

	s = mode;
	while (*s) {
		switch (*s++) {
		case 'x':
			if (rwa) {
			bad_mode:
				//alifErr_setString(_alifExcValueError_,
				//	"Must have exactly one of create/read/write/append "
				//	"mode and at most one plus");
				goto error;
			}
			rwa = 1;
			self->created = 1;
			self->writable = 1;
			flags |= O_EXCL | O_CREAT;
			break;
		case 'r':
			if (rwa)
				goto bad_mode;
			rwa = 1;
			self->readable = 1;
			break;
		case 'w':
			if (rwa)
				goto bad_mode;
			rwa = 1;
			self->writable = 1;
			flags |= O_CREAT | O_TRUNC;
			break;
		case 'a':
			if (rwa)
				goto bad_mode;
			rwa = 1;
			self->writable = 1;
			self->appending = 1;
			flags |= O_APPEND | O_CREAT;
			break;
		case 'b':
			break;
		case '+':
			if (plus)
				goto bad_mode;
			self->readable = self->writable = 1;
			plus = 1;
			break;
		default:
			//alifErr_format(_alifExcValueError_,
			//	"invalid mode: %.200s", mode);
			goto error;
		}
	}

	if (!rwa)
		goto bad_mode;

	if (self->readable && self->writable)
		flags |= O_RDWR;
	else if (self->readable)
		flags |= O_RDONLY;
	else
		flags |= O_WRONLY;

#ifdef O_BINARY
	flags |= O_BINARY;
#endif

#ifdef _WINDOWS
	flags |= O_NOINHERIT;
#elif defined(O_CLOEXEC)
	flags |= O_CLOEXEC;
#endif

	//if (alifSys_audit("open", "Osi", nameobj, mode, flags) < 0) {
	//	goto error;
	//}

	if (fd >= 0) {
		self->fd = fd;
		self->closefd = closefd;
	}
	else {
		self->closefd = 1;
		if (!closefd) {
			//alifErr_setString(_alifExcValueError_,
			//	"Cannot use closefd=False with file name");
			goto error;
		}

		errno = 0;
		if (opener == ALIF_NONE) {
			do {
				ALIF_BEGIN_ALLOW_THREADS
#ifdef _WINDOWS
					self->fd = _wopen(widename, flags, 0666);
#else
					self->fd = open(name, flags, 0666);
#endif
				ALIF_END_ALLOW_THREADS
			} while (self->fd < 0 and errno == EINTR /*and
				!(async_err = alifErr_checkSignals())*/);

				if (async_err)
					goto error;

				if (self->fd < 0) {
					alifErr_setFromErrnoWithFilenameObject(_alifExcOSError_, nameobj);
					goto error;
				}
		}
		else {
			AlifObject* fdobj;

#ifndef _WINDOWS
			/* the opener may clear the atomic flag */
			atomic_flag_works = nullptr;
#endif

			fdobj = alifObject_callFunction(opener, "Oi", nameobj, flags);
			if (fdobj == nullptr)
				goto error;
			if (!ALIFLONG_CHECK(fdobj)) {
				ALIF_DECREF(fdobj);
				//alifErr_setString(_alifExcTypeError_,
				//	"expected integer from opener");
				goto error;
			}

			self->fd = alifLong_asInt(fdobj);
			ALIF_DECREF(fdobj);
			if (self->fd < 0) {
				if (!alifErr_occurred()) {
					//alifErr_format(_alifExcValueError_,
					//	"opener returned %d", self->fd);
				}
				goto error;
			}
		}
		fd_is_own = 1;

#ifndef _WINDOWS
		if (_alif_setInheritable(self->fd, 0, atomic_flag_works) < 0)
			goto error;
#endif
	}

	alifMem_dataFree(self->statAtOpen);
	self->statAtOpen = (AlifStatStruct*)alifMem_dataAlloc(sizeof(AlifStatStruct));
	if (self->statAtOpen == nullptr) {
		//alifErr_noMemory();
		goto error;
	}
	ALIF_BEGIN_ALLOW_THREADS
		fstat_result = _alifFStat_noraise(self->fd, self->statAtOpen);
	ALIF_END_ALLOW_THREADS
		if (fstat_result < 0) {
#ifdef _WINDOWS
			if (GetLastError() == ERROR_INVALID_HANDLE) {
				//alifErr_setFromWindowsErr(0);
#else
			if (errno == EBADF) {
				//alifErr_setFromErrno(_alifExcOSError_);
#endif
				goto error;
			}

			alifMem_dataFree(self->statAtOpen);
			self->statAtOpen = nullptr;
			}
		else {
#if defined(S_ISDIR) and defined(EISDIR)
			if (S_ISDIR(self->statAtOpen->mode)) {
				errno = EISDIR;
				alifErr_setFromErrnoWithFilenameObject(_alifExcOSError_, nameobj);
				goto error;
			}
#endif /* defined(S_ISDIR) */
		}

#if defined(_WINDOWS) or defined(__CYGWIN__)
	/* don't translate newlines (\r\n <=> \n) */
	_setmode(self->fd, O_BINARY);
#endif

	if (alifObject_setAttr((AlifObject*)self, &ALIF_STR(Name), nameobj) < 0)
		goto error;

	if (self->appending) {
		//AlifObject* pos = portable_lseek(self, nullptr, 2, true);
		//if (pos == nullptr)
		//	goto error;
		//ALIF_DECREF(pos);
	}

	goto done;

error:
	ret = -1;
	if (!fd_is_own)
		self->fd = -1;
	if (self->fd >= 0) {
		AlifObject* exc = alifErr_getRaisedException();
		internal_close(self);
		_alifErr_chainExceptions1(exc);
	}
	if (self->statAtOpen != nullptr) {
		alifMem_dataFree(self->statAtOpen);
		self->statAtOpen = nullptr;
	}

done:
#ifdef _WINDOWS
	alifMem_dataFree(widename);
#endif
	ALIF_CLEAR(stringobj);
	return ret;
}





static AlifIntT fileIO_traverse(AlifObject* op, VisitProc visit, void* arg) { // 539
	FileIO* self = ALIFFILEIO_CAST(op);
	ALIF_VISIT(ALIF_TYPE(self));
	ALIF_VISIT(self->dict);
	return 0;
}


static AlifObject* err_closed(void) { // 580
	alifErr_setString(_alifExcValueError_, "I/O operation on closed file");
	return nullptr;
}



static AlifObject* _ioFileIO_readableImpl(FileIO* self) { // 615
	if (self->fd < 0)
		return err_closed();
	return alifBool_fromLong((long)self->readable);
}

static AlifObject* _ioFileIO_seekableImpl(FileIO* self) { // 645
	if (self->fd < 0)
		return err_closed();
	if (self->seekable < 0) {
		AlifObject* pos = portable_lseek(self, NULL, SEEK_CUR, false);
		if (pos == nullptr) {
			alifErr_clear();
		}
		else {
			ALIF_DECREF(pos);
		}
	}
	return alifBool_fromLong((long)self->seekable);
}

static AlifObject* _ioFileIO_readintoImpl(FileIO* self,
	AlifTypeObject* cls, AlifBuffer* buffer) { // 674
	AlifSizeT n{};
	AlifIntT err{};

	if (self->fd < 0)
		return err_closed();
	if (!self->readable) {
		AlifIOState* state = getIOState_byCls(cls);
		//return err_mode(state, "reading");
	}

	n = _alif_read(self->fd, buffer->buf, buffer->len);
	err = errno;

	if (n == -1) {
		if (err == EAGAIN) {
			alifErr_clear();
			return ALIF_NONE;
		}
		return nullptr;
	}

	return alifLong_fromSizeT(n);
}


static AlifUSizeT new_bufferSize(FileIO* _self, AlifUSizeT _currentSize) { // 703
	AlifUSizeT addend{};
	if (_currentSize > LARGE_BUFFER_CUTOFF_SIZE)
		addend = _currentSize >> 3;
	else
		addend = 256 + _currentSize;
	if (addend < SMALLCHUNK)
		addend = SMALLCHUNK;
	return addend + _currentSize;
}

static AlifObject* _ioFileIO_readallImpl(FileIO* _self) { // 731
	AlifOffT pos{}, end{};
	AlifObject* result;
	AlifSizeT bytesRead = 0;
	AlifSizeT n{};
	AlifUSizeT bufsize{};

	if (_self->fd < 0) {
		return err_closed();
	}

	if (_self->statAtOpen != nullptr and _self->statAtOpen->size < ALIF_READ_MAX) {
		end = (AlifOffT)_self->statAtOpen->size;
	}
	else {
		end = -1;
	}
	if (end <= 0) {
		/* Use a default size and resize as needed. */
		bufsize = SMALLCHUNK;
	}
	else {
		/* This is probably a real file. */
		if (end > ALIF_READ_MAX - 1) {
			bufsize = ALIF_READ_MAX;
		}
		else {
			bufsize = (AlifUSizeT)end + 1;
		}

		if (bufsize > LARGE_BUFFER_CUTOFF_SIZE) {
			ALIF_BEGIN_ALLOW_THREADS
			ALIF_BEGIN_SUPPRESS_IPH
#ifdef _WINDOWS
			pos = _lseeki64(_self->fd, 0L, SEEK_CUR);
#else
			pos = lseek(_self->fd, 0L, SEEK_CUR);
#endif
			ALIF_END_SUPPRESS_IPH
			ALIF_END_ALLOW_THREADS

			if (end >= pos and pos >= 0 and (end - pos) < (ALIF_READ_MAX - 1)) {
				bufsize = (AlifUSizeT)(end - pos) + 1;
			}
		}
	}


	result = alifBytes_fromStringAndSize(nullptr, bufsize);
	if (result == nullptr)
		return nullptr;

	while (1) {
		if (bytesRead >= (AlifSizeT)bufsize) {
			bufsize = new_bufferSize(_self, bytesRead);
			if (bufsize > ALIF_SIZET_MAX || bufsize <= 0) {
				alifErr_setString(_alifExcOverflowError_,
					"unbounded read returned more bytes "
					"than a ALIF bytes object can hold");
				ALIF_DECREF(result);
				return nullptr;
			}

			if (ALIFBYTES_GET_SIZE(result) < (AlifSizeT)bufsize) {
				if (alifBytes_resize(&result, bufsize) < 0)
					return nullptr;
			}
		}

		n = _alif_read(_self->fd,
			ALIFBYTES_AS_STRING(result) + bytesRead,
			bufsize - bytesRead);

		if (n == 0)
			break;
		if (n == -1) {
			if (errno == EAGAIN) {
				alifErr_clear();
				if (bytesRead > 0)
					break;
				ALIF_DECREF(result);
				return ALIF_NONE;
			}
			ALIF_DECREF(result);
			return nullptr;
		}
		bytesRead += n;
	}

	if (ALIFBYTES_GET_SIZE(result) > bytesRead) {
		if (alifBytes_resize(&result, bytesRead) < 0)
			return nullptr;
	}
	return result;
}



static AlifObject* portable_lseek(FileIO* _self, AlifObject* _posObj,
	AlifIntT _whence, bool _suppressPipeError) { // 947
	AlifOffT pos{}, res{};
	AlifIntT fd = _self->fd;

#ifdef SEEK_SET
	/* Turn 0, 1, 2 into SEEK_{SET,CUR,END} */
	switch (_whence) {
#if SEEK_SET != 0
	case 0: _whence = SEEK_SET; break;
#endif
#if SEEK_CUR != 1
	case 1: _whence = SEEK_CUR; break;
#endif
#if SEEK_END != 2
	case 2: _whence = SEEK_END; break;
#endif
	}
#endif /* SEEK_SET */

	if (_posObj == nullptr) {
		pos = 0;
	}
	else {
#if defined(HAVE_LARGEFILE_SUPPORT)
		pos = alifLong_asLongLong(_posObj);
#else
		pos = alifLong_asLong(_posObj);
#endif
		if (alifErr_occurred())
			return nullptr;
	}

	ALIF_BEGIN_ALLOW_THREADS
	ALIF_BEGIN_SUPPRESS_IPH
#ifdef _WINDOWS
		res = _lseeki64(fd, pos, _whence);
#else
		res = lseek(fd, pos, _whence);
#endif
	ALIF_END_SUPPRESS_IPH
	ALIF_END_ALLOW_THREADS

		if (_self->seekable < 0) {
			_self->seekable = (res >= 0);
		}

	if (res < 0) {
		if (_suppressPipeError and errno == ESPIPE) {
			res = 0;
		}
		else {
			//return alifErr_setFromErrno(_alifExcOSError_);
		}
	}

#if defined(HAVE_LARGEFILE_SUPPORT)
	return alifLong_fromLongLong(res);
#else
	return alifLong_fromLong(res);
#endif
}


static AlifObject* _ioFileIO_isAttyImpl(FileIO* self) { // 1202
	long res{};

	if (self->fd < 0)
		return err_closed();
	ALIF_BEGIN_ALLOW_THREADS
	ALIF_BEGIN_SUPPRESS_IPH
	res = isatty(self->fd);
	ALIF_END_SUPPRESS_IPH
	ALIF_END_ALLOW_THREADS
	return alifBool_fromLong(res);
}


static AlifObject* _ioFileIO_isAttyOpenOnly(AlifObject* op, AlifObject* ALIF_UNUSED(ignored)) { // 1226
	FileIO* self = ALIFFILEIO_CAST(op);
	if (self->statAtOpen != nullptr and !S_ISCHR(self->statAtOpen->mode)) {
		ALIF_RETURN_FALSE;
	}
	return _ioFileIO_isAttyImpl(self);
}

#include "clinic/FileIO.cpp.h" // 1236

static AlifMethodDef _fileIOMethods_[] = { // 1238
	_IO_FILEIO_READALL_METHODDEF
	_IO_FILEIO_READINTO_METHODDEF
	_IO_FILEIO_CLOSE_METHODDEF
	_IO_FILEIO_SEEKABLE_METHODDEF
	_IO_FILEIO_READABLE_METHODDEF
	{"_isAttyOpenOnly", _ioFileIO_isAttyOpenOnly, METHOD_NOARGS},
	{"_deallocWarn", fileIO_deallocWarn, METHOD_O},
	{nullptr, nullptr}             /* sentinel */
};


static AlifObject* fileIO_getClosed(AlifObject* op, void* closure) { // 1261
	FileIO* self = ALIFFILEIO_CAST(op);
	return alifBool_fromLong((long)(self->fd < 0));
}


static AlifObject* fileIO_getBlkSize(AlifObject* op, void* closure) { // 1282
	FileIO* self = ALIFFILEIO_CAST(op);
#ifdef HAVE_STRUCT_STAT_ST_BLKSIZE
	if (self->statAtOpen != nullptr and self->statAtOpen->blkSize > 1) {
		return alifLong_fromLong(self->statAtOpen->blkSize);
	}
#endif /* HAVE_STRUCT_STAT_ST_BLKSIZE */
	return alifLong_fromLong(DEFAULT_BUFFER_SIZE);
}

static AlifGetSetDef _fileIOGetSetList_[] = { // 1294
	{"Closed", fileIO_getClosed, nullptr, "True if the file is closed"},
	{"_blksize", fileIO_getBlkSize, nullptr, "Stat blksize if available"},
	{nullptr},
};

static AlifMemberDef _fileIOMembers_[] = { // 1303
	{"_finalizing", ALIF_T_BOOL, offsetof(FileIO, finalizing), 0},
	{"__weakListOffset__", ALIF_T_ALIFSIZET, offsetof(FileIO, weakRefList), ALIF_READONLY},
	{"__dictOffset__", ALIF_T_ALIFSIZET, offsetof(FileIO, dict), ALIF_READONLY},
	{nullptr}
};


static AlifTypeSlot _fileIOSlots_[] = { // 1310
	{ALIF_TP_TRAVERSE, fileIO_traverse},
	{ALIF_TP_METHODS, _fileIOMethods_},
	{ALIF_TP_MEMBERS, _fileIOMembers_},
	{ALIF_TP_GETSET, _fileIOGetSetList_},
	{ALIF_TP_INIT, _ioFileIO___init__},
	{ALIF_TP_NEW, fileio_new},
	{0, nullptr},
};



AlifTypeSpec _fileIOSpec_ = { // 1324
	.name = "تبادل.ملف",
	.basicsize = sizeof(FileIO),
	.flags = (ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_BASETYPE | ALIF_TPFLAGS_HAVE_GC |
			  ALIF_TPFLAGS_IMMUTABLETYPE),
	.slots = _fileIOSlots_,
};




