#include "alif.h"

#include "AlifCore_Fileutils.h"     
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

#include "_iomodule.h"







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


#define ALIFFILEIO_CAST(_op) ALIF_CAST(FileIO*, (_op))


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
	_IO_FILEIO_READABLE_METHODDEF
	{"_isAttyOpenOnly", _ioFileIO_isAttyOpenOnly, METHOD_NOARGS},
	{nullptr, nullptr}             /* sentinel */
};



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




