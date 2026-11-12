#include "alif.h"
#include "AlifCore_FileUtils.h"
#include "AlifCore_InitConfig.h"
#include "AlifCore_LifeCycle.h"
#include "AlifCore_Runtime.h"


#ifdef HAVE_UNISTD_H
#  include <unistd.h>             // close()
#endif
#ifdef _WINDOWS
#  include <windows.h>
#  include <bcrypt.h>
#else
#  include <fcntl.h>              // O_RDONLY
#  ifdef HAVE_SYS_STAT_H
#    include <sys/stat.h>
#  endif
#  ifdef HAVE_LINUX_RANDOM_H
#    include <linux/random.h>     // GRND_NONBLOCK
#  endif
#  if defined(HAVE_SYS_RANDOM_H) && (defined(HAVE_GETRANDOM) || defined(HAVE_GETENTROPY))
#    include <sys/random.h>       // getrandom()
#  endif
#  if !defined(HAVE_GETRANDOM) && defined(HAVE_GETRANDOM_SYSCALL)
#    include <sys/syscall.h>      // SYS_getrandom
#  endif
#endif



// 49
#ifdef _WINDOWS

static AlifIntT win32_urandom(unsigned char *_buffer, AlifSizeT _size,
	AlifIntT _raise) { // 53
	while (_size > 0)
	{
		DWORD chunk = (DWORD)ALIF_MIN(_size, ALIF_DWORD_MAX);
		NTSTATUS status = BCryptGenRandom(nullptr, _buffer, chunk, BCRYPT_USE_SYSTEM_PREFERRED_RNG);
		if (!BCRYPT_SUCCESS(status)) {
			/* BCryptGenRandom() failed */
			if (_raise) {
				//alifErr_setFromWindowsErr(0);
			}
			return -1;
		}
		_buffer += chunk;
		_size -= chunk;
	}
	return 0;
}

#else /* !_WINDOWS */ // 73



#define URANDOM_CACHE (_alifRuntime_.alifHashState.urandomCache) // 271

static AlifIntT dev_urandom(char *buffer,
	AlifSizeT size, AlifIntT raise) { // 300
	AlifIntT fd{};
	AlifSizeT n{};

	if (raise) {
		class AlifStatStruct st{};
		AlifIntT fstat_result{};

		if (URANDOM_CACHE.fd >= 0) {
			ALIF_BEGIN_ALLOW_THREADS
				fstat_result = _alifFStat_noraise(URANDOM_CACHE.fd, &st);
			ALIF_END_ALLOW_THREADS

				if (fstat_result
					or st.st_dev != URANDOM_CACHE.st_dev
					or st.st_ino != URANDOM_CACHE.st_ino) {
					URANDOM_CACHE.fd = -1;
				}
		}
		if (URANDOM_CACHE.fd >= 0)
			fd = URANDOM_CACHE.fd;
		else {
			fd = _alif_open("/dev/urandom", O_RDONLY);
			if (fd < 0) {
				if (errno == ENOENT || errno == ENXIO ||
					errno == ENODEV || errno == EACCES) {
					//alifErr_setString(_alifExcNotImplementedError_,
					//	"/dev/urandom (or equivalent) not found");
				}
				return -1;
			}
			if (URANDOM_CACHE.fd >= 0) {
				close(fd);
				fd = URANDOM_CACHE.fd;
			}
			else {
				if (_alif_fStat(fd, &st)) {
					close(fd);
					return -1;
				}
				else {
					URANDOM_CACHE.fd = fd;
					URANDOM_CACHE.st_dev = st.st_dev;
					URANDOM_CACHE.st_ino = st.st_ino;
				}
			}
		}

		do {
			n = _alif_read(fd, buffer, (AlifUSizeT)size);
			if (n == -1)
				return -1;
			if (n == 0) {
				alifErr_format(_alifExcRuntimeError_,
					"فشل في قراءة %zi ثمانية من /dev/urandom",
					size);
				return -1;
			}

			buffer += n;
			size -= n;
		} while (0 < size);
	}
	else {
		fd = _alifOpen_noraise("/dev/urandom", O_RDONLY);
		if (fd < 0) {
			return -1;
		}

		while (0 < size)
		{
			do {
				n = read(fd, buffer, (AlifUSizeT)size);
			} while (n < 0 and errno == EINTR);

			if (n <= 0) {
				/* stop on error or if read(size) returned 0 */
				close(fd);
				return -1;
			}

			buffer += n;
			size -= n;
		}
		close(fd);
	}
	return 0;
}



#endif /* !_WINDOWS */ // 409



static AlifIntT alif_urandom(void *_buffer, AlifSizeT _size,
	AlifIntT _blocking, AlifIntT _raise) { // 476
#if defined(ALIF_GETRANDOM) or defined(ALIF_GETENTROPY)
	AlifIntT res{};
#endif

	if (_size < 0) {
		if (_raise) {
			alifErr_format(_alifExcValueError_,
				"negative argument not allowed");
		}
		return -1;
	}

	if (_size == 0) {
		return 0;
	}

#ifdef _WINDOWS
	return win32_urandom((unsigned char *)_buffer, _size, _raise);
#else

#if defined(ALIF_GETRANDOM) or defined(ALIF_GETENTROPY)
	if (HAVE_GETENTRYPY_GETRANDOM_RUNTIME) {
	#ifdef ALIF_GETRANDOM
		res = alif_getRandom(_buffer, _size, _blocking, _raise);
	#else
		res = alif_getEntropy(_buffer, _size, _raise);
	#endif
		if (res < 0) {
			return -1;
		}
		if (res == 1) {
			return 0;
		}
	}
#endif

	return dev_urandom((char*)_buffer, _size, _raise);
#endif
}




AlifIntT _alifOS_uRandomNonBlock(void *_buffer, AlifSizeT _size) { // 544
	return alif_urandom(_buffer, _size, 0, 1);
}
