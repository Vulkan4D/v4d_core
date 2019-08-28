#ifdef _WINDOWS
	#include <direct.h>
	#include <winsock2.h>
	#include <ws2tcpip.h>
	#include <windows.h>

	#define MAXFLOAT std::numeric_limits<float>::max()

	#include "common.hh"
#endif
