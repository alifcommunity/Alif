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
	ALIFOBJECT_HEAD;
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












AlifTypeSpec _fileIOSpec_ = { // 1324
	.name = "تبادل.ملف",
	.basicsize = sizeof(FileIO),
	.flags = (ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_BASETYPE | ALIF_TPFLAGS_HAVE_GC |
			  ALIF_TPFLAGS_IMMUTABLETYPE),
	//.slots = _fileIOSlots_,
};
