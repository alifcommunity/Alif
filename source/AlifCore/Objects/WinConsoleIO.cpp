#include "alif.h"

#include "AlifCore_FileUtils.h"
#include "AlifCore_Object.h"




#ifdef HAVE_WINDOWS_CONSOLE_IO // 14






char _get_consoleType(HANDLE handle) { // 51
	DWORD mode{}, peek_count{};

	if (handle == INVALID_HANDLE_VALUE)
		return '\0';

	if (!GetConsoleMode(handle, &mode))
		return '\0';

	/* Peek at the handle to see whether it is an input or output handle */
	if (GetNumberOfConsoleInputEvents(handle, &peek_count))
		return 'r';
	return 'w';
}








#endif /* HAVE_WINDOWS_CONSOLE_IO */
